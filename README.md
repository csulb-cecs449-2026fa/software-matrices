# Software Matrices

This is the focused assignment project derived from the `Matrices` stage of
`software-3d-basics`. It keeps the reusable raster code, the matrix-based
rendering pipeline, the graphics application's Assimp mesh loading, and the
Bunny model without the unrelated lecture-progression executables.

The student TODOs from the Matrices stage are preserved. The tests under
`tests/MatricesTests/` validate the model, view, perspective, and orthographic
matrix builders, along with MVP composition and perspective division in
`drawMesh`. The Common regression tests remain alongside them.

## Project structure

- `Common/` builds `common_drawing`, the supplied framebuffer, line, and
  wireframe-triangle library.
- `Matrices/` builds `matrices_core`, which contains the testable matrix
  construction functions, coordinate conversion, and mesh drawing code.
- `Matrices/` also builds the optional `Matrices` SFML application.
- `models/bunny.obj` is copied beside the application after it builds.
- `tests/CommonTests/` contains regression tests for the supplied Common code.
- `tests/MatricesTests/` contains the assignment-specific matrix tests.
- `tests/CMakeLists.txt` is also a standalone instructor-test project.

All tests are compiled into the `instructor_tests` executable and registered
as the single CTest test `instructor_tests`. This is the interface expected by
the Classroom 50 setup script.

## Prerequisites

- CMake 3.21 or newer
- Ninja
- vcpkg, with `VCPKG_ROOT` set
- A C++20 compiler

GLM is the only core dependency. Assimp and SFML belong to the `graphics`
feature, while GoogleTest belongs to the `tests` feature.

## Build and test on Windows x64

From this directory:

```powershell
cmake --preset x64-debug
cmake --build --preset x64-debug
ctest --preset x64-debug
```

The platform presets are filtered by host architecture. Windows ARM64 presets
are shown on Windows ARM64 machines; Windows x64 presets are shown on Windows
x64 machines. macOS includes separate ARM64 and x64 presets.

## Classroom 50 / Linux autograder

The `linux-autograder` preset disables the graphics application and installs
only GLM and the `tests` feature. It does not install Assimp or SFML.

Configure once:

```sh
cmake --preset linux-autograder
```

Build and run the instructor tests:

```sh
cmake --build --preset linux-autograder --target instructor_tests
ctest --preset linux-autograder -R '^instructor_tests$'
```

Classroom 50 configures `tests/` directly rather than configuring the root
project. The standalone test project accepts `REPO_ROOT`, compiles the Common
and Matrices implementation sources, and does not build or link Assimp or SFML.

After the tests and `vcpkg.json` have been committed, configure the Classroom
50 checks from the repository root with:

```sh
bash add-tests.sh ORGANIZATION/CLASSROOM/assignments/ASSIGNMENT tests
```

The script protects the committed tests and manifest with a Git-tree digest,
builds the expected `instructor_tests` target outside the student repository,
and runs the matching CTest entry.
