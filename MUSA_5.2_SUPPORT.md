# Uni-Dock native MUSA build note

This note describes the public build integration carried by the Moore Threads
MUSA branch. It is an architecture and configuration reference, not a
performance or runtime qualification statement.

## Source scope

The branch is based on Uni-Dock **v1.2.0** and retains the upstream source
layout. The upstream project is <https://github.com/dptech-corp/Uni-Dock>.
The native MUSA changes are confined to the engine build graph, compatibility
headers, MUSA translation units, and the related search-policy code and tests.

## Build integration

`unidock/CMakeLists.txt` provides an explicit `ENABLE_MUSA` option:

```sh
cmake -S unidock -B build-musa -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_MUSA=ON \
  -DFETCH_BOOST=ON \
  -DBUILD_FAST_MATH=ON \
  -DMUSA_ARCHITECTURE=mp_31
cmake --build build-musa
```

The option uses the MUSA toolkit CMake modules, enables the MUSA language,
and links the MUSA runtime. `MUSA_ARCHITECTURE` is a native offload target;
it is not a CUDA `sm_` value. The default source configuration names `mp_31`,
which can be changed for another supported target by the build user.

The MUSA engine compiles `src/cuda/monte_carlo.mu` and
`src/cuda/precalculate.mu`. These units include the shared algorithm bodies,
while `src/cuda/musa_compat.h` supplies the MUSA equivalents for the runtime,
memory, stream/event, error, and random-number calls used by the engine. Host
code and the CUDA `.cu` route remain in place, so the same source tree can be
configured for either backend.

## Dependencies

Use CMake 3.16 or newer, Ninja, OpenMP, and Boost 1.72 or newer. Passing
`FETCH_BOOST=ON` uses the Boost release selected by the CMake configuration.
Catch2 v3.4.0 is fetched for the unit-test target. A MUSA 5.2 toolkit with
its CMake package and compiler must be installed and discoverable for
`ENABLE_MUSA=ON`; a supported CUDA toolkit is required for the CUDA route.

The companion `unidock_tools` package is independent of the engine's CMake
language selection. Install it with Python 3.10 or newer from its directory:

```sh
python -m pip install ./unidock_tools
```

## Compatibility boundaries

The repository preserves the upstream engine interfaces, docking input/output
formats, CUDA configuration, and Python tooling. Backend-specific options and
device targets should be selected according to the installed toolkit and the
target hardware. Changes to shared kernels or search policy should be reviewed
for both CUDA and MUSA configurations.

For command-line usage, examples, contribution guidance, and attribution, see
the root [README.md](README.md) and the inherited engine documentation under
`unidock/`.
