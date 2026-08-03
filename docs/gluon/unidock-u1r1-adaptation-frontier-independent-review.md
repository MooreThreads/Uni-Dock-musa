# Uni-Dock U1R1 adaptation-frontier independent review

## Review disposition

**Technical result: the retained U1 frontier is independently verified and is
fit to be the implementation input. Native-MUSA build and GPU correctness remain
INCONCLUSIVE, not failed.** This documentation-only review neither implements the
frontier nor grants formal acceptance.

The review began from a clean detached worktree at exact retained U1 result
commit `6c88a7126d96b4a01849262a9f82fc09cf462e35`. That commit is the direct child
of reviewed Uni-Dock source commit
`ebde3b4a22c8b2c7b26e523cc15109aab815be21` and adds only
`docs/gluon/unidock-u1-source-dependency-build-frontier.md`; `git diff --quiet`
across the engine, tools, root metadata, and workflows returned zero. Thus the
source independently inspected here is byte-identical to the source inventoried
by U1. The worktree was clean before all probes. No failed worktree was located,
opened, modified, or cleaned, and no nested worktree was created.

The required outcome is therefore established at the scope-review level: begin
the narrow implementation slice described below without adding another gate.
Compilation and correctness are deliberately not promoted to `GO`: the retained
source cannot configure without NVCC, no production adaptation exists yet, and
the harness forbids GPU access. Those facts are expected boundaries rather than
a demonstrated product failure, so `NO_GO` would be inaccurate.

## Source and version identity

Independent Git and source checks established:

| Item | Evidence |
| --- | --- |
| Review base | clean detached `6c88a7126d96b4a01849262a9f82fc09cf462e35` |
| Reviewed source | parent `ebde3b4a22c8b2c7b26e523cc15109aab815be21` |
| Source commit parent | `244f24a5197b758a06e7f6cdb1b523afdaeb4251` |
| Source subject/date | `build(deps): bump actions/checkout from 6 to 7 (#203)`; `2026-06-30T17:52:08+08:00` |
| Describe/tag | `1.2.0-4-gebde3b4`; tag `1.2.0` points to `95e4091` |
| Canonical remote | fetch/push `git@github.com:dptech-corp/Uni-Dock.git` |
| Engine identity | CMake project `1.2.0`; `VERSION="v${PROJECT_VERSION}"`; CLI text `Uni-Dock v1.2.0` |
| Tools identity | `unidock_tools/pyproject.toml` version `0.0.1` |
| Repository structure | no gitlinks or submodules; Apache-2.0 root license |

This confirms that the retained receipt did not accidentally review an older
release denominator. Its 1.2.0 changelog warning remains material, including
the CUDA 12.8/Blackwell and docking-correctness fixes listed there.

## Independent build and caller review

`unidock/CMakeLists.txt` is the only tracked CMake entry point. Its graph is
unconditional at `project(... LANGUAGES CXX CUDA)` and independently confirms:

1. OpenMP is required. Boost >=1.72 requires system, thread, serialization,
   filesystem, program-options, and timer; `FETCH_BOOST=ON` freezes Boost 1.84.0
   and its header-only math component. Catch2 is always fetched at tag `v3.4.0`.
2. Object target `lib` lists 16 host `.cpp` files. There are 17 tracked
   `unidock/src/lib/*.cpp` files because `scoring_function.cpp` is not listed in
   that target; this does not contradict the U1 target-graph count.
3. Object target `cuda` consists of both and only
   `src/cuda/monte_carlo.cu` and `src/cuda/precalculate.cu`. Its shared GPU
   surface is the five headers `kernel.h`, `common.cuh`, `warp_ops.cuh`,
   `precalculate_gpu.cuh`, and `atom_constants_gpu.cuh`.
4. `unidock` links both object targets, Boost, and OpenMP; `split` is host-only;
   `unit_tests` links both object targets plus Boost, OpenMP, and Catch2. Hence a
   single-kernel or precalculation-only port cannot yield the current engine or
   its real link test.
5. `main.cpp` selects GPU batch work, queries/selects devices, checks memory, and
   enters `Vina::enable_gpu` / `global_search_gpu`. `vina.cpp` calls
   `precalculate_parallel`, constructs `monte_carlo`, and dispatches its stream
   or default-stream operators. `vina_cuda_worker.h` selects a device and runs
   the paired 1:1 path; `simulation_container.h` supplies threaded immediate
   callers. This matches the retained caller map.

The CI and container denominator also matches U1: the engine CI uses
`nvidia/cuda:12.9.2-devel-ubuntu24.04`, installs CMake/Boost/Ninja/Git, builds
and installs the complete engine, and runs four Python GPU integration methods.
The Dockerfile uses the same CUDA image family with CMake's default generator.
The old `unidock/test/Makefile` refers to missing sources, ROCm-era paths,
Boost 1.67, and GoogleTest, so it remains historical rather than a new gate.

## CUDA, MURAND, and cooperative-groups surface

The two GPU translation units contain the runtime allocation, copy, memset,
stream/event, synchronization, constant-symbol, launch, and device-code surface.
`main.cpp` is the only non-`.cu` file with a CUDA runtime include, while
`common.cuh` owns the CURAND include. `kernel.h` owns the shared error wrapper
and all fixed GPU layouts. `warp_ops.cuh` directly uses
`cooperative_groups::thread_block_tile` and `cg::reduce`.

Installed MUSA evidence provides the necessary direct surface:

- `/usr/local/musa/version.json` identifies toolkit 5.2.0 at commit
  `8c44cbed02a79be2066f1f62bdca039177f71d51` and runtime 5.2.0 at commit
  `c92b4e82ce65993b9b996b20eb33b1ffd3185dbd`; `libmusart.so.5.2.0` is present.
- `/usr/local/musa/bin/mcc --version` reports mcc 5.2.0 on a Clang 14 base. The
  compiler is installed but is not on this fresh shell's default `PATH`, so a
  native build must prepend `/usr/local/musa/bin` or set `MUSACXX` explicitly.
- Installed CMake modules expose `EnableMUSALanguage`, `FindMUSAToolkit`, and
  `MUSA::musart`; the language suffix is `.mu` and the compiler module explicitly
  supports only Ninja.
- Installed runtime headers declare the matching device, allocation, pinned
  allocation, copy/memset, constant-symbol, stream, event, and error APIs.
- `murand_kernel.h` provides `murandStatePhilox4_32_10_t`, the matching
  three-argument `murand_init`, and `murand_uniform`. Sequence/numeric identity
  is a later measurement, not an assumption.
- Installed cooperative-groups headers provide static thread-block tiles and
  reduction overloads, including the 32-thread shape used by Uni-Dock. Float
  atomics and `--use_fast_math` also exist, but scheduling, reduction order,
  occupancy, and numeric behavior still require the later GPU oracle.

An independent exact-token scan found 62 `cudaMalloc`, 61 `cudaFree`, 25
`cudaMallocHost`, 24 `cudaFreeHost`, 44 synchronous `cudaMemcpy`, 9
`cudaMemcpyAsync`, 13 each of synchronous/asynchronous memset, 5 device
synchronizations, and 2 constant-symbol copies. These replace the lower raw
counts in U1's prose. The correction strengthens the warning against global
blind replacement but does not change the concentrated file frontier or any
dependency/build conclusion.

CUDA SM architecture numbers have no justified MUSA translation. The
implementation must use an assigned MUSA target/toolchain policy rather than
inventing an SM mapping. The MUSA compiler/header presence proves availability,
not successful compilation of these unmodified CUDA units.

## Dependency and test inventory

The fresh runner independently provides Ubuntu 22.04 x86-64, CMake 3.22.1,
GCC/G++ 11.4.0, Ninja 1.13.2, Make 4.3, Python 3.10.12, Git 2.34.1, libgomp,
and MUSA 5.2.0. NVCC and system Boost libraries are absent. The executable or
Python-module probes also found pytest, setuptools, RDKit, NetworkX, NumPy,
pandas, pathos, pip, OpenBabel, CDPKit `confgen`, conda, and mamba absent.
Consequently `FETCH_BOOST=ON` is the practical official engine path; the tools
and chemistry stack remain downstream consumers, not first-slice prerequisites.

The test inventory was verified from definitions and data rather than inferred
from names:

- CMake has one Catch2 parser case. It parses the tracked `def.pdbqt` receptor
  and requires exactly 1,613 atoms. It executes no GPU API, but the current
  target links both GPU object files and therefore provides a real native-MUSA
  compile/link/load check.
- Engine integration contains exactly four methods: PDBQT 1IEP Vina/Vinardo and
  SDF 1A30 Vina/Vinardo. All are real GPU-batch docking with seed 181129, one
  pose, output/energy parsing, and only a broad `[-20, -1]` score assertion.
- Tools unit tests contain six definitions. Tools application tests contain nine
  definitions and eleven parameterized cases: two 8AJX cases, six unparameterized
  preparation/pipeline cases, and the three-case `bigsdf`/`1bcu`/bad-ligand
  default pipeline. Their external tools and weak assertions keep them outside
  the first engine slice.
- All 43 tracked Python files under `unidock` and `unidock_tools` parsed through
  Python's AST without writing bytecode. No unavailable dependency was fetched.

No golden pose or exact affinity is tracked, and fixed seed does not make
cross-device fast-math/reduction output byte-identical. U1 correctly avoids
turning the broad integration score range into a correctness oracle.

## Pre-freezable representative 1IEP oracle

The existing PDBQT/Vina 1IEP case is independently confirmed as the smallest
representative end-to-end correctness plan, not a synthetic kernel probe. Its
call path covers receptor and ligand parsing, Vina map generation, GPU pairwise
precalculation, Philox Monte Carlo, cooperative-group BFGS/reductions, result
transfer, host refinement/ranking, and PDBQT serialization.

Freeze and run this exact command first on a designated upstream NVIDIA
reference GPU/container, then on the assigned MUSA target:

```text
unidock --receptor unidock/test/receptor/1iep_receptor.pdbqt \
  --gpu_batch unidock/test/ligands/1iep_ligand.pdbqt --dir <output> \
  --center_x 15.19 --center_y 53.903 --center_z 16.917 \
  --size_x 20 --size_y 20 --size_z 20 \
  --scoring vina --search_mode fast --num_modes 1 --seed 181129
```

Independent SHA-256 checks reproduce:

- receptor: `761469710e3915b89b274483076dfe49754be7956661bc42f4cc36a182399d59`
- ligand: `37a20e58e77072c6b3ff07f285a345e647dd4e564af316a06e49518b15b7aa61`

The reference freeze must retain hardware, driver/toolkit, exact source,
command/environment, stdout, output PDBQT and hash, affinity, and heavy-atom
coordinates. Repeat runs on the fixed NVIDIA and fixed MUSA devices determine
affinity-delta and heavy-atom RMSD tolerances. The comparison requires a
parseable one-pose output and finite negative affinity in addition to those
measured tolerances; the existing broad range remains only a smoke assertion.
The 1A30 SDF case follows to cover SDF output, rather than enlarging the first
oracle. This plan is pre-freezable now and was not executed without a GPU.

## Smallest coherent implementation slice

The smallest practical native-MUSA slice is the complete engine graph, with no
artificial gate:

1. Add a narrow MUSA branch to `unidock/CMakeLists.txt` using the installed
   official modules, Ninja, explicit MUSA compiler discovery, and
   `MUSA::musart`. Preserve the 16-source host object, both GPU translation
   units, `unidock`, `split`, and `unit_tests`. Give only the two GPU units the
   MUSA-recognized source treatment; replace CUDA architecture policy rather
   than mechanically translating SM numbers.
2. Adapt the runtime/error boundary in `kernel.h`, CURAND in `common.cuh`, both
   GPU translation units, and direct host runtime callers `main.cpp` and
   `vina_cuda_worker.h` to exact MUSA/MURAND counterparts. Preserve layouts,
   algorithms, stream/event order, launch dimensions, 32-thread tiles,
   cooperative-group reductions, atomics, and scoring math unless mcc emits a
   concrete incompatibility.
3. Configure Release with `FETCH_BOOST=ON`, the existing fast-math behavior,
   Ninja, two build jobs, and `OMP_NUM_THREADS=2`; build the default complete
   graph (`lib`, `cuda`, `unidock`, `split`, and `unit_tests`). Catch2 remains
   the upstream denominator. Do not add a synthetic kernel or another
   acceptance gate.
4. Run CTest for the existing parser case. It is the correct non-GPU boundary
   for a full native-MUSA compile/link/load. Record `unidock --version`; source
   inspection confirms its early return occurs before any device operation.
5. In the separately authorized GPU stage, run the frozen 1IEP oracle before
   changing tile/reduction, MURAND sequence handling, or numeric tolerances.

Both GPU units are necessary because `Vina::global_search_gpu` uses their
precalculation and Monte Carlo symbols as a single engine. Porting Python tools,
containers, packaging, benchmarks, multi-GPU behavior, or broad source
conversion would add consumers or risk without proving this boundary.

## Bounded probe receipt

Every command that could schedule work used `OMP_NUM_THREADS=2` and
`taskset -c 0,1`. No GPU command, device query, build, download, service,
container, credential, database, cleanup, deletion, move, rename, push, deploy,
or production edit was performed. Probe roots are under `/tmp`, not the
worktree, and were left in place.

| Probe | Exact result |
| --- | --- |
| Initial source state | detached exact `6c88a7126d96b4a01849262a9f82fc09cf462e35`; clean porcelain |
| Upstream configure | `/tmp/gluon-u1r1-upstream-cuda-ninja`; Release/Ninja; exit 1 at CUDA compiler discovery because NVCC was not found |
| MUSA/Ninja configure | installed `demo_project_enable_musa_language` into `/tmp/gluon-u1r1-musa-ninja`; exit 0; compiler identified as MooreThreads; toolkit include/library paths and `MUSA::musart` found |
| MUSA/Make configure | same installed demo into `/tmp/gluon-u1r1-musa-make`; exit 1 with explicit unsupported-generator diagnostic |
| Static source checks | complete CMake/caller inspection, exact API scan, test-definition inventory, input SHA-256, and AST parsing of 43 Python files succeeded |
| GPU/build/tests | not run: prohibited GPU access; unadapted source cannot configure without NVCC; dependencies were intentionally not downloaded |

This evidence independently validates U1's scope and dependencies and supports
starting its complete native-MUSA engine slice. It does not establish native
build success, GPU correctness, performance, deployment readiness, or formal
acceptance.
