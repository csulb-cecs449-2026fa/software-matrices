#!/usr/bin/env bash

set -euo pipefail

if (( $# != 2 )); then
    echo "Usage: $0 ORGANIZATION/CLASSROOM/assignments/ASSIGNMENT TEST_DIRECTORY" >&2
    exit 2
fi

assignmentPath=${1#/}
assignmentPath=${assignmentPath%/}

testDirInput=${2%/}
repoRoot=$(pwd -P)
manifest=vcpkg.json

# Parse:
#
#   organization/classroom/assignments/assignment
#
IFS='/' read -r org classroom assignmentsSegment assignment extra <<< "$assignmentPath"

if [[ -n "${extra:-}" ||
      -z "${org:-}" ||
      -z "${classroom:-}" ||
      "$assignmentsSegment" != "assignments" ||
      -z "${assignment:-}" ]]; then
    echo "Invalid assignment path:" >&2
    echo "  $assignmentPath" >&2
    echo >&2
    echo "Expected:" >&2
    echo "  ORGANIZATION/CLASSROOM/assignments/ASSIGNMENT" >&2
    exit 2
fi

if [[ ! -d "$testDirInput" ]]; then
    echo "Instructor-test directory not found: $testDirInput" >&2
    exit 1
fi

if [[ ! -f "$testDirInput/CMakeLists.txt" ]]; then
    echo "Instructor-test CMake project not found: $testDirInput/CMakeLists.txt" >&2
    exit 1
fi

if [[ ! -f "$manifest" ]]; then
    echo "vcpkg manifest not found: $manifest" >&2
    exit 1
fi

if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    echo "This script must be run inside the assignment template Git repository." >&2
    exit 1
fi

# Commands stored by Classroom 50 run in student-repository roots, so convert
# the local test path to a repository-relative path before embedding it.
testDirAbsolute=$(cd "$testDirInput" && pwd -P)
case "$testDirAbsolute/" in
    "$repoRoot"/*) testDir=${testDirAbsolute#"$repoRoot"/} ;;
    *)
        echo "Instructor-test directory must be inside the repository: $testDirInput" >&2
        exit 1
        ;;
esac

# Make sure every protected input exists in the committed template. The
# integrity digest is based on HEAD, not on line-ending-dependent working-tree
# bytes or on uncommitted changes.
trackedTestFiles=$(git ls-tree -r --full-tree --name-only HEAD -- "$testDir")

if [[ -z "$trackedTestFiles" ]]; then
    echo "No committed files found under: $testDir" >&2
    echo "Commit the instructor tests before configuring the autograder." >&2
    exit 1
fi

trackedManifest=$(git ls-tree --full-tree --name-only HEAD -- "$manifest")

if [[ "$trackedManifest" != "$manifest" ]]; then
    echo "Protected manifest is not committed at HEAD: $manifest" >&2
    echo "Commit the manifest before configuring the autograder." >&2
    exit 1
fi

# Hash committed Git tree entries for the entire test directory and manifest.
# Each entry records its mode, object type, object ID, and pathname, so the
# digest detects content and structural changes without being affected by
# CRLF/LF conversion in a checkout.
gitTreeDigest()
{
    git ls-tree -rz --full-tree HEAD -- "$@" |
        sha256sum |
        awk '{print $1}'
}

expectedDigest=$(gitTreeDigest "$testDir" "$manifest")

echo "Instructor tests: $testDir"
echo "Protected manifest: $manifest"
echo "Expected digest: $expectedDigest"
echo

# Classroom 50 launches commands through /bin/sh. Bash's printf %q can emit
# $'...' quoting, which /bin/sh does not understand, so quote embedded command
# arguments using POSIX single-quote syntax instead.
shellQuote()
{
    printf "'"
    printf '%s' "$1" | sed "s/'/'\"'\"'/g"
    printf "'"
}

quotedTestDir=$(shellQuote "$testDir")
quotedManifest=$(shellQuote "$manifest")

# Compute the same digest from each student's committed HEAD. Reject a root
# vcpkg-configuration.json because vcpkg would interpret it as build config.
integrityCheck="\
test ! -e vcpkg-configuration.json && \
test ! -L vcpkg-configuration.json && \
actual=\$(git ls-tree -rz --full-tree HEAD -- $quotedTestDir $quotedManifest | \
sha256sum | \
awk '{print \$1}') && \
test \"\$actual\" = '$expectedDigest'"

quotedIntegrityCheck=$(shellQuote "$integrityCheck")
integrityCommand="bash -o pipefail -c $quotedIntegrityCheck"

# The build directory is outside the student repository, cannot be supplied by
# a submission, and is shared by the declarative tests within one grading job.
configureCommand="repoRoot=\$(pwd -P); \
buildDir=\"\${RUNNER_TEMP:?}/software-matrices-instructor\"; \
cmake \
-S $quotedTestDir \
-B \"\$buildDir\" \
-DCMAKE_TOOLCHAIN_FILE=\"\$VCPKG_INSTALLATION_ROOT/scripts/buildsystems/vcpkg.cmake\" \
-DVCPKG_MANIFEST_DIR=\"\$repoRoot\" \
-DVCPKG_MANIFEST_MODE=ON \
-DVCPKG_MANIFEST_INSTALL=ON \
-DVCPKG_MANIFEST_FEATURES=tests \
-DREPO_ROOT=\"\$repoRoot\" \
-DCMAKE_BUILD_TYPE=Release"

buildCommand="$configureCommand && \
cmake --build \"\$buildDir\" --target instructor_tests --parallel"

ctestCommand="buildDir=\"\${RUNNER_TEMP:?}/software-matrices-instructor\"; \
ctest \
--test-dir \"\$buildDir\" \
--output-on-failure \
--no-tests=error \
-R '^instructor_tests$'"

echo "Adding autograder tests to '$assignment'..."

gh teacher assignment test add \
    "$org" \
    "$classroom" \
    "$assignment" \
    --name "Test target builds" \
    --type run \
    --setup "$integrityCommand" \
    --run "$buildCommand" \
    --timeout 600 \
    --points 1

gh teacher assignment test add \
    "$org" \
    "$classroom" \
    "$assignment" \
    --name "Tests pass" \
    --type run \
    --setup "$integrityCommand && { $buildCommand; }" \
    --run "$ctestCommand" \
    --timeout 600 \
    --points 1

gh teacher assignment test add \
    "$org" \
    "$classroom" \
    "$assignment" \
    --name "Tests were not modified" \
    --type run \
    --run "$integrityCommand" \
    --timeout 30 \
    --points 8

echo
echo "Autograder tests configured:"
echo "  1 point  - Test target builds"
echo "  1 point  - Tests pass"
echo "  8 points - Tests and vcpkg.json were not modified"


