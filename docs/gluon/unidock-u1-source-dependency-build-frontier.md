# Uni-Dock U1 source, dependency, build/test frontier

## Receipt status

This is the U1 inventory receipt for the first Uni-Dock MUSA 5.2.0 task. It
freezes the upstream denominator and a smallest coherent implementation
frontier; it does not adapt production source.

**Result: INCONCLUSIVE for native-MUSA build and GPU correctness.** The assigned
runner has no GPU and the unmodified project requires NVCC at CMake's
`project(... LANGUAGES CXX CUDA)` statement. The inventory and the MUSA compiler
discovery probe completed, but neither an upstream CUDA build nor GPU execution
could be established here. This is not a demonstrated product failure and is
therefore not `NO_GO`. No required harness or verification commands were
supplied (`{}` for both contracts).

## Frozen source identity

| Field | Frozen value / evidence |
| --- | --- |
| Canonical source | `git@github.com:dptech-corp/Uni-Dock.git` (`origin` fetch and push URL) |
| Reviewed commit | `ebde3b4a22c8b2c7b26e523cc15109aab815be21` |
| Required parent for the next receipt commit | the commit above; the documentation commit must be its direct child |
| Checkout state before inventory | detached `HEAD`, clean |
| Commit subject/date | `build(deps): bump actions/checkout from 6 to 7 (#203)`, committed 2026-06-30 +0800 |
| Described revision | `1.2.0-4-gebde3b4` |
| Engine version | CMake project `1.2.0`; compiled `--version` text is `Uni-Dock v1.2.0` |
| Nearest release | tag `1.2.0` is commit `95e4091`; the reviewed source is four commits later |
| Tools version | `unidock_tools` package `0.0.1` |
| License | repository-wide Apache License 2.0; root README records relicensing on 2025-03-10 |
| Submodules / gitlinks | none (`git submodule status` and mode-160000 inventory were empty) |

The 1.2.0 changelog is material to the denominator: it adds CUDA 12.8/Blackwell
support and fixes grid-buffer overflow, large-box handling, macrocycle affinity,
out-of-bounds poses, a `cudaMemset` pointer error, bad-ligand/atom-type crashes,
quaternion instability, and flexible-receptor GPU handling. These corrections
must not be compared against an older release oracle.

## Components and scope

### Supported upstream surface

- `unidock`: Linux C++/CUDA docking engine, `unidock` executable, helper `split`
  executable, object libraries, Catch2 parser test, and Python GPU integration
  tests. It accepts PDBQT and SDF GPU batches and PDBQT CPU ligands.
- Scoring functions: Vina, Vinardo, and AD4. AD4 consumes pre-generated affinity
  maps; Vina/Vinardo can calculate maps from a receptor.
- GPU modes: `--gpu_batch`, text `--ligand_index`, normal/local/score-only flows,
  fixed-seed Monte Carlo, and paired 1:1 receptor/ligand batching using CUDA
  streams. The paired-batch README limits that mode to Vina.
- CPU path: single `--ligand` docking, Boost-thread/OpenMP host work, parsing,
  scoring/refinement, and output handling remain ordinary C++.
- Search presets: fast = exhaustiveness 128/max-step 20, balance = 384/40,
  detail = 512/40. Output can contain multiple poses with energy and RMSD data.
- Upstream advertises NVIDIA compute capability >= 7.0. Default architectures
  are `70` (CUDA <13 only), `75`, `80`, `86`, `89`, `90`, `100`, and `120`.
  Release builds default to `--use_fast_math`.
- Input limits are encoded in `src/cuda/kernel.h`: specialized ligand shapes up
  through 300 atoms, 48 ligand torsions, one flexible-receptor torsion, fixed
  grid storage of 531,441 points, and several fixed pair/rigid-node bounds.
- `unidock_tools`: separate Python CLI with receptor preparation, ligand
  preparation, end-to-end docking, AD4 grid generation, and multi-conformation
  docking. It shells out to the engine and optional chemistry programs.

### Explicit exclusions for the first MUSA slice

- No ROCm preservation or revival. The README says v1.0 supported ROCm, while
  v1.1+ is CUDA-optimized and future ROCm support was dropped.
- No GPU support beyond the current engine semantics: more than one flexible
  receptor torsion, unsupported SDF atom types, larger fixed model/grid bounds,
  or changes to macrocycle sampling are out of scope.
- No receptor-processor resurrection; 1.2.0 removed it.
- No `unidock_tools`, CDPKit/OpenBabel/MGLTools, bundled `autogrid4`, benchmark
  repository, Docker image, packaging, or workflow migration in the first
  native engine slice. Those are immediate consumers, not prerequisites for
  proving the engine build.
- No algorithm rewrite, CPU/synthetic GPU fallback, broad automated conversion,
  performance tuning, multi-GPU work, or new correctness gates.

## Build and immediate caller map

The complete tracked build entry points are `unidock/CMakeLists.txt`,
`unidock/Dockerfile`, `unidock/test/Makefile`, `unidock/test/run_test.sh`,
`unidock_tools/pyproject.toml`, `unidock_tools/Dockerfile`, and the four GitHub
workflows. There are no other tracked CMake modules or build shell scripts.

The CMake graph is unconditional:

1. CMake >=3.16 enables `CXX` and `CUDA`; C++14 is selected for CUDA 12.x and
   C++17 for CUDA >=13. The source does not set `CMAKE_CUDA_STANDARD` in the
   pre-13 branch, although the host standard is C++14.
2. OpenMP is required. Boost >=1.72 components are `system`, `thread`,
   `serialization`, `filesystem`, `program_options`, and `timer`. With
   `FETCH_BOOST=ON`, exact Boost 1.84.0 plus header-only `Boost::math` is fetched.
3. Catch2 is always fetched from its official repository at tag `v3.4.0`; there
   is no `BUILD_TESTING` guard around the fetch or test target.
4. Object target `lib` contains 16 host `.cpp` files. Object target `cuda`
   contains exactly `src/cuda/monte_carlo.cu` and `src/cuda/precalculate.cu` and
   includes all five `.cuh`/`.h` GPU headers. `unidock` links both objects,
   Boost, and OpenMP. `split` is host-only. `unit_tests` links host and GPU
   objects, OpenMP, Boost, and Catch2.
5. `main.cpp` selects CPU versus batch/GPU paths, queries device count and free
   memory, and calls `Vina::enable_gpu` / `global_search_gpu`. `vina.cpp` creates
   GPU precalculation data and calls `precalculate_parallel`, then invokes the
   GPU Monte Carlo operators. `vina_cuda_worker.h` and
   `simulation_container.h` are the paired-batch callers and own per-thread
   device selection/stream-based launches.
6. `precalculate.cu` owns the pairwise scoring-table kernel and constant-memory
   upload. `monte_carlo.cu` owns host/device marshalling, four variants of the
   docking path, Philox state, the Monte Carlo/BFGS kernel, output transfer, and
   stream/event lifetime. `kernel.h`, `common.cuh`, `warp_ops.cuh`,
   `precalculate_gpu.cuh`, and `atom_constants_gpu.cuh` define all shared GPU
   layouts and device functions.

Container/CI denominator: engine images use
`nvidia/cuda:12.9.2-devel-ubuntu24.04`; CI installs build-essential, CMake,
`libboost-all-dev`, Ninja, and Git, configures Release/Ninja, builds/installs,
then runs the Python engine tests on an NVIDIA runner. The engine Dockerfile
uses the same CUDA version but the default CMake generator. The benchmark
workflow uses external `dptech-corp/Uni-Dock-Benchmarks-bak`, fixed seed 123,
and molecular-docking plus virtual-screening result CSVs.

The legacy `unidock/test/Makefile` is not the current CMake graph: it references
old `build/linux/release` objects, `src/rocm`, Boost 1.67, GoogleTest, and three
missing test source files. Its cleanup target was not invoked. Treat it as stale
historical evidence, not a build denominator.

## CUDA surface and MUSA disposition

The GPU surface is concentrated but not limited to lexical runtime calls.

| CUDA surface | Evidence in reviewed source | MUSA 5.2.0 disposition |
| --- | --- | --- |
| Language/build | CMake `CUDA`, two `.cu` sources, CUDA architecture list, CUDA flags/toolkit include variable | **Direct MUSA analogue.** Installed custom modules expose `enable_language(MUSA)`, `FindMUSAToolkit`, `MUSA::musart`, and `mcc`; they require Ninja and recognize `.mu` as the language suffix. Replace build metadata deliberately. |
| Runtime/error | `cudaGetErrorName`; device count/select; memory info; device sync | **Direct MUSA analogue.** Exact `musaGetErrorName`, `musaGetDeviceCount`, `musaSetDevice`, `musaMemGetInfo`, and `musaDeviceSynchronize` declarations are installed. |
| Memory | device malloc/free, pinned host malloc/free, sync/async copies, memset, and two constant-symbol copies | **Direct MUSA analogue.** Exact `musaMalloc`, `musaFree`, `musaMallocHost`, `musaFreeHost`, copy/memset, async, and `musaMemcpyToSymbol` APIs are installed. |
| Streams/events | create/destroy/synchronize stream; record/synchronize/destroy events; elapsed time | **Direct MUSA analogue.** Matching MUSA types/APIs are installed. Preserve stream ordering and default-stream behavior explicitly. |
| Device random | `<curand_kernel.h>`, `curandStatePhilox4_32_10_t`, `curand_init`, `curand_uniform` | **Direct MUSA analogue.** Installed `<murand_kernel.h>` provides `murandStatePhilox4_32_10_t`, `murand_init`, and `murand_uniform`. Sequence equivalence must be measured, not assumed. |
| Kernels/device C++ | `__global__`, `__device__`, `__host__`, launch syntax, built-in indices, shared/constant memory, launch bounds | **Compiler analogue available.** `mcc 5.2.0` compiles MUSA language. Compile compatibility remains unproven for these translation units. |
| Cooperative groups | tiled groups of 32 and `cg::reduce` from identically named headers | **Header analogue available.** MUSA ships both headers and a 32-thread tile implementation. Runtime reduction semantics still need the later GPU oracle. |
| Atomics/warp assumptions | float `atomicAdd`, 32-thread launch, tile and shared-memory formulas, `MAX_THREADS_PER_BLOCK=32`, `MIN_BLOCKS_PER_MP=32` | **Primitive exists; correctness risk.** MUSA provides float atomic add and 32-thread groups, but no GPU was assigned to validate scheduling, occupancy, or reduction order. Do not blindly rewrite warp code. |
| Math/precision | float-heavy device math and CUDA `--use_fast_math` | **Compiler analogue available.** `mcc` accepts `--use_fast_math` (and documents its FTZ/div/sqrt effects). Numeric equivalence requires tolerant comparison. |
| Architecture | CUDA SM list including Volta through Blackwell | **Must be replaced, not translated.** CUDA SM numbers are invalid as a MUSA architecture policy. Use the assigned MUSA target/toolchain setting when hardware is known; do not invent an SM mapping. |

Raw inventory counts across production source found 56 `cudaMalloc`, 56
`cudaFree`, 24 each of pinned host allocation/free, 38 synchronous and 9
asynchronous copies, 13 sync and 13 async memsets, four device synchronizations
in Monte Carlo plus one in precalculation, and stream/event operations. These
counts include repeated specialized docking implementations and show why a
global blind replacement would be unsafe.

## Dependency classification

Disposition names below are the requested categories. “Available” means found
on this reviewed runner, not merely mentioned by upstream.

| Dependency / constraint | Upstream requirement | Runner evidence | Disposition |
| --- | --- | --- | --- |
| MUSA toolkit/runtime | adaptation target | `/usr/local/musa`; `mcc` and toolkit 5.2.0; runtime 5.2.0; `libmusart.so.5.2.0` | **already available in the reviewed MUSA runner** |
| CUDA toolkit/NVCC | engine README >=11.8; CI/image 12.9.2 | `nvcc` absent | **replaceable through a direct MUSA analogue**; unmodified configure cannot pass |
| CMake | >=3.16 | 3.22.1 | **already available in the reviewed MUSA runner** |
| C++ compiler | NVCC-compatible; C++14 or C++17 branch | GCC/G++ 11.4.0 | **already available in the reviewed MUSA runner** |
| Ninja | CI and MUSA custom-language requirement | 1.13.2; MUSA configure succeeds with Ninja and fails with Unix Makefiles | **already available in the reviewed MUSA runner** |
| GNU Make | Docker/default and stale legacy tests | 4.3 | **already available in the reviewed MUSA runner**, but not usable for the installed custom MUSA language |
| OpenMP | required CMake package; only direct pragmas in `vina.cpp` | GCC/libgomp toolchain present; MUSA probe found Threads | **already available in the reviewed MUSA runner** |
| Boost | >=1.72, six libraries; math headers in GPU compilation | headers/libraries absent | **fetchable/installable from an official source**; CMake freezes official Boost 1.84.0 when `FETCH_BOOST=ON` |
| Catch2 | CMake unit-test framework | no cached dependency | **fetchable/installable from an official source** at upstream tag `v3.4.0`; tag is versioned but not a recorded commit hash |
| Python | tools metadata >=3.10; tools README incorrectly says >=3.8; examples say >=3.6 | Python 3.10.12 | **already available in the reviewed MUSA runner** for metadata minimum and examples |
| setuptools | tools build backend, unconstrained | absent | **fetchable/installable from an official source**; packaging is excluded from slice 1 |
| rdkit, networkx, numpy, pandas, pathos | unconstrained `pyproject.toml` runtime dependencies | all absent; `multiprocess` import arrives via pathos | **fetchable/installable from an official source**; versions are not frozen upstream |
| pytest / pytest-cov / Ruff | CI-only test/lint dependencies | absent | **fetchable/installable from an official source**; CI requests pytest >=8 only for engine, otherwise versions float |
| CDPKit `confgen` | optional MCDock conformer generator | absent; Docker pins official release 1.1.0 installer | **fetchable/installable from an official source** |
| OpenBabel `obabel` | alternative MCDock generator and legacy conversion | absent; Docker uses unpinned conda-forge package | **fetchable/installable from an official source** |
| MGLTools | README requirement for AD4 preparation | absent and undeclared in package metadata | **fetchable/installable from an official source**, outside engine slice |
| AutoGrid4/data | AD4 affinity-map generation | tracked x86-64 `autogrid4` binary and `AD4.1_bound.dat` | **already available in the reviewed source tree**; provenance/version is not declared, so do not use it as a new MUSA build dependency |
| External benchmark suite/data | benchmark workflow | not in this repository; workflow checks out a separate repository and downloads data | **fetchable/installable from an official source**, but not frozen by this commit |
| NVIDIA container runtime/images | Docker and CI only | Docker CLI exists; no GPU assigned and daemon/image state not probed | **concrete blocker** for reproducing upstream GPU CI on this runner; not a blocker to native MUSA compilation |
| Miniforge/conda | tools Dockerfile | absent; Dockerfile downloads `latest` | **fetchable/installable from an official source**, but upstream does not freeze the installer |
| Reusable Gluon dependency | potential cross-project source | no other project or declared reusable package exists in this checkout | **reusable from another Gluon project: none evidenced**; do not assume one |

Additional packaging observation: `unidock_tools/pyproject.toml` declares
`license = { file = "LICENSE" }`, but there is no `unidock_tools/LICENSE`.
The repository root license exists. This may block or warn during isolated tools
packaging, but it is outside the engine adaptation frontier.

No vendored source libraries or submodules exist. The notable tracked artifacts
are the AutoGrid4 executable and two compressed DUD-E screening archives
(`def` and `mmp13`; despite `.tar.bz2`, `file` identifies gzip streams).

## Tests, examples, and comparison hooks

### Inventories

- CMake/Catch2: one parser case loads `unittest/test_data/def.pdbqt` and requires
  exactly 1,613 receptor atoms. It links the GPU objects but executes no GPU API.
- Engine Python integration: four test methods: PDBQT Vina/Vinardo on 1IEP and
  SDF Vina/Vinardo on 1A30. All use GPU batch, fast mode, one pose, seed 181129,
  require an output energy record, and only assert a broad `[-20, -1]` range.
- Tools unit tests: six definitions covering CDPKit/OpenBabel conformers,
  Vina/AD4 engine invocation, ligand topology, and PDB-to-PDBQT conversion.
- Tools application tests: nine definitions / eleven collected parameter cases
  covering ligand/protein preparation, 8AJX MCDock stages and CLI, 1BCU index,
  AD4 and multi-pose flows, plus `bigsdf`, `1bcu`, and one-bad-ligand batches.
  Several assert only existence/count/score sign; the MCDock RMSD assertion is
  commented out.
- Historical scripts cover CASF-2016 redocking/RMSD and LIT-PCBA PPARG virtual
  screening/enrichment, but embed unavailable `/root/dockdata`, `hmtdock`,
  OpenBabel, Biopandas, pandas, and NumPy dependencies. They are not CI tests.
- Representative repository examples are DUD-E `def` and `mmp13` virtual
  screening (seed 5, Vina, 128/20, nine poses), and four `def` receptor/active
  ligand pairs in paired-batch mode (seed 5, 1024/60).
- The external benchmark workflow fixes seed 123 and uploads docking result and
  metrics CSVs plus screening results, but the benchmark data and expected
  metric values are not frozen here.

There are no tracked golden docking output files or exact expected affinities.
Useful deterministic hooks are explicit seeds, Philox state keyed by pose ID,
three-decimal energy serialization, sorted poses, input structures/configs,
`rmsdfn.py` coordinate RMSD readers, tools' RDKit RMSD helper, and the existing
score/pose-count assertions. Fast math and parallel floating-point reductions
mean fixed seed alone does not justify byte-identical cross-device output.

### Later representative GPU correctness oracle

Use the existing 1IEP PDBQT Vina case as the first representative oracle. It is
the smallest real end-to-end path through receptor parsing, Vina grid creation,
GPU precalculation, Philox Monte Carlo, cooperative-group BFGS, result transfer,
refinement, ranking, and PDBQT writing:

```text
unidock --receptor unidock/test/receptor/1iep_receptor.pdbqt \
  --gpu_batch unidock/test/ligands/1iep_ligand.pdbqt --dir <output> \
  --center_x 15.19 --center_y 53.903 --center_z 16.917 \
  --size_x 20 --size_y 20 --size_z 20 \
  --scoring vina --search_mode fast --num_modes 1 --seed 181129
```

Frozen input SHA-256 values are:

- receptor: `761469710e3915b89b274483076dfe49754be7956661bc42f4cc36a182399d59`
- ligand: `37a20e58e77072c6b3ff07f285a345e647dd4e564af316a06e49518b15b7aa61`

Before implementing MUSA, run that exact command once with this exact source on
the designated upstream NVIDIA reference GPU/container and freeze: hardware,
driver/toolkit, complete command/environment, stdout, output PDBQT, output hash,
reported affinity, and heavy-atom coordinates. That artifact is pre-freezable
now because source, inputs, and command are fixed; it was not fabricated on this
GPU-less runner. Compare the MUSA result by output existence/parseability, one
pose, finite negative affinity, affinity delta, and heavy-atom coordinate RMSD.
Set numeric tolerances from repeat runs on the fixed NVIDIA reference and fixed
MUSA target, rather than inventing them in U1. Preserve the upstream broad range
as a smoke assertion, not as the correctness oracle. The 1A30 SDF case should
follow after this first oracle to cover SDF serialization, not enlarge slice 1.

## Smallest coherent native-MUSA implementation frontier

The minimum useful slice is a **complete native-MUSA engine build with the
existing parser test**, not one isolated kernel and not the Python tools:

1. Add a narrow MUSA build branch in `unidock/CMakeLists.txt` using the installed
   official custom modules, Ninja, MUSA language, and `MUSA::musart`. Keep the
   16 host sources, `unidock`, `split`, and `unit_tests` graph intact. Mark or
   provide only the two GPU translation units as MUSA sources; do not convert
   unrelated C++ or architecture numbers mechanically.
2. Adapt the shared error/runtime boundary in `kernel.h`, the two GPU translation
   units, and the direct host callers `main.cpp` and `vina_cuda_worker.h` from
   CUDA runtime types/names to their exact MUSA counterparts. Replace CURAND
   Philox identifiers with MURAND. Keep layouts, kernels, launch dimensions,
   streams, events, cooperative-group code, atomics, and scoring math unchanged
   unless `mcc` produces a concrete diagnostic.
3. Build all four current targets in Release with `FETCH_BOOST=ON`, two build
   jobs, `OMP_NUM_THREADS=2`, and the existing fast-math setting. Catch2 remains
   the upstream test denominator; no new gate is required.
4. Run `ctest` for the single parser case. This is a real link/load check of the
   MUSA objects and runtime without GPU execution. Record `unidock --version`
   only if it does not initialize a device. A successful compile/parser test is
   the completion boundary of the first implementation slice.
5. On a later assigned MUSA GPU, execute the frozen 1IEP oracle above. Only then
   decide whether the fixed 32-thread tile/reduction, launch bounds, MURAND
   sequence, or numeric tolerances need targeted changes.

This frontier necessarily includes both `.cu` implementations because the
current CMake graph and `Vina::global_search_gpu` link them as one engine. A
precalculation-only object would not produce a callable docking binary; porting
all Python tools/containers would add consumers without proving GPU semantics.

## Bounded probes and evidence

All probes used `OMP_NUM_THREADS=2`; no GPU command was run. No dependencies were
downloaded because discovery was conclusive without them. Fresh build roots are
retained as requested.

| Probe | Exact outcome |
| --- | --- |
| Source state | clean detached `ebde3b4a22c8b2c7b26e523cc15109aab815be21`; canonical SSH origin; no submodules |
| Runner | Ubuntu 22.04/glibc 2.35 host userland, x86-64; 128 online CPUs visible but commands were bounded to CPU2/OMP2 |
| Toolchain | CMake 3.22.1, GCC/G++ 11.4.0, Ninja 1.13.2, Make 4.3, Python 3.10.12, Git 2.34.1 |
| Upstream configure | `/tmp/unidock-u1-cuda-config.xyNdUW`; failed immediately because NVCC/CUDA toolkit was absent |
| MUSA/Make configure | `/tmp/unidock-u1-musa-config.llUy6W`; failed because the installed MUSA language module does not support Unix Makefiles |
| MUSA/Ninja configure | `/tmp/unidock-u1-musa-ninja-config.FMSOql`; succeeded; compiler identified as MooreThreads and `MUSA::musart` toolkit paths were found |
| MUSA identity | mcc 5.2.0 (Clang 14 base); toolkit commit `8c44cbed02a79be2066f1f62bdca039177f71d51`; runtime commit `c92b4e82ce65993b9b996b20eb33b1ffd3185dbd` |
| Dependency presence | Boost, pytest, setuptools, RDKit, NetworkX, NumPy, pandas, pathos, OpenBabel, CDPKit CLI, conda, mamba, and pip absent |
| Static syntax | AST parsing succeeded for all 43 tracked Python files under `unidock` and `unidock_tools`; TOML parser was unavailable on runner Python and no package install was attempted |
| Tests/GPU | not run: current binary cannot configure/build without CUDA, dependencies are absent, and no GPU was assigned |

Accordingly, the evidence supports beginning the narrow implementation slice,
but it does not support a build or correctness `GO` claim.
