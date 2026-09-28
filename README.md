# Uni-Dock MUSA

Uni-Dock MUSA is the Moore Threads maintained MUSA port of [Uni-Dock](https://github.com/dptech-corp/Uni-Dock), a GPU-accelerated molecular-docking engine and its companion Python tools. The repository keeps the upstream project structure and CUDA route while adding a native MUSA build path for Moore Threads GPUs.

## Source and version

- Upstream project: <https://github.com/dptech-corp/Uni-Dock>
- Inherited source version: **v1.2.0**
- Upstream baseline: `ebde3b4a22c8b2c7b26e523cc15109aab815be21`
- MUSA maintenance branch: `uni-dock-musa`

The upstream engine and tools remain available under `unidock/` and
`unidock_tools/`. Upstream history, changelog entries, and attribution are
retained in the repository.

## MUSA adaptation

The native MUSA route is selected with `ENABLE_MUSA=ON` in
`unidock/CMakeLists.txt`. It uses the MUSA toolkit's CMake integration and
`mcc`, accepts a native `MUSA_ARCHITECTURE` (the default is `mp_31`), and links
the MUSA runtime. MUSA translation units in `unidock/src/cuda/*.mu` reuse the
shared docking implementation; `musa_compat.h` provides the runtime, memory,
stream, event, error, and random-number interfaces needed by those units.

The CUDA path remains available when `ENABLE_MUSA=OFF`. Shared host code,
scoring functions, input/output formats, tests, and the Python tooling are
kept in their upstream locations. The MUSA-specific build and search-policy
notes are summarized in [MUSA_5.2_SUPPORT.md](MUSA_5.2_SUPPORT.md).

## Dependencies

The engine requires:

- CMake 3.16 or newer and Ninja (recommended);
- a C++ compiler and OpenMP;
- Boost 1.72 or newer, or `FETCH_BOOST=ON` to fetch the version selected by
  the project; and
- Catch2 v3.4.0, fetched by CMake for the unit-test target.

For the native MUSA route, install a MUSA 5.2 toolkit with its CMake modules
available to CMake. For the CUDA route, use a supported CUDA toolkit and set
the CUDA architecture list as needed.

The Python tools require Python 3.10 or newer and the dependencies declared in
`unidock_tools/pyproject.toml` (including RDKit, NetworkX, NumPy, pandas, and
pathos). Some preparation workflows have additional tool-specific
requirements; see [unidock_tools/README.md](unidock_tools/README.md).

## Build the engine

From the repository root, configure a release MUSA build. The toolkit's CMake
package must be discoverable by the installation used on your system.

```sh
cmake -S unidock -B build-musa -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_MUSA=ON \
  -DFETCH_BOOST=ON \
  -DBUILD_FAST_MATH=ON \
  -DMUSA_ARCHITECTURE=mp_31
cmake --build build-musa
```

To build with CUDA instead, leave `ENABLE_MUSA` off and select the desired
CUDA architecture through `CMAKE_CUDA_ARCHITECTURES`:

```sh
cmake -S unidock -B build-cuda -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DFETCH_BOOST=ON \
  -DCMAKE_CUDA_ARCHITECTURES=80
cmake --build build-cuda
```

The main executable is `unidock`; the input and option reference is available
with `unidock --help`. The `split` utility and the CMake `unit_tests` target
are built alongside the main executable.

## Python tools

Install the companion package from its directory after installing the engine:

```sh
python -m pip install ./unidock_tools
```

The `unidocktools` command provides receptor and ligand preparation and an
end-to-end docking workflow. For example:

```sh
unidocktools proteinprep -r receptor.pdb -o receptor.pdbqt
unidocktools ligandprep -l ligands.sdf -sd prepared_ligands
unidocktools unidock_pipeline \
  -r receptor.pdbqt -l prepared_ligands \
  -sd results -cx 0 -cy 0 -cz 0
```

Consult the tool documentation for format-specific options and the
multi-conformation workflow.

## Repository layout

```text
unidock/          C++ docking engine, CMake build, tests, and examples
unidock_tools/    Python preparation and workflow tools
docs/             Inherited and MUSA porting documentation
MUSA_5.2_SUPPORT.md  Public MUSA build-integration note
CHANGELOG.md      Inherited release history
```

## Contributing

Please retain the CUDA path when changing MUSA code, keep MUSA-specific
changes isolated and documented, and update the relevant build or usage notes.
Bug reports and pull requests should include the affected backend, toolkit
version, target architecture, and a minimal reproduction where applicable.
See [.github/CONTRIBUTING.md](.github/CONTRIBUTING.md) for the inherited
contribution workflow.

## Attribution and license

Uni-Dock was developed by DP Technology and its contributors. Please cite the
Uni-Dock publication when using the project in research:

> Y. Yu, C. Cai, J. Wang, Z. Bo, Z. Zhu, and H. Zheng, “Uni-Dock:
> GPU-Accelerated Docking Enables Ultralarge Virtual Screening,” *Journal of
> Chemical Theory and Computation* (2023),
> <https://doi.org/10.1021/acs.jctc.2c01145>.

The Moore Threads MUSA distribution is licensed under the Apache 2.0 terms in
the root [LICENSE](LICENSE). Third-party notices and licenses retained in
subdirectories continue to apply to their respective components.
