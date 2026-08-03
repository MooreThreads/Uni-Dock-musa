# Uni-Dock MUSA 5.2 support and maintenance handoff

## Accepted baseline

This handoff describes the bounded native-MUSA consumer supported by the
accepted U2 implementation and U4 correctness correction. It is not a general
GPU-support claim and does not grant formal acceptance.

| Identity | Exact value |
| --- | --- |
| Canonical source | `git@github.com:dptech-corp/Uni-Dock.git` |
| Upstream source denominator | `ebde3b4a22c8b2c7b26e523cc15109aab815be21` (`1.2.0-4-gebde3b4`) |
| Engine / CLI version | CMake `1.2.0`; `Uni-Dock v1.2.0` |
| Accepted native-MUSA implementation (U2) | `67bfdc95b5a1fc2aac38c327a16bac732be7312a` |
| Rejected correctness result (U3) | `0159c1324e4ca85ab103a3a2577e26029d53c360` (`NO_GO`) |
| Accepted correctness correction (U4) | `07e3a079de9850d8941d5a3552b4c5feb044c99c` (technical `GO`) |
| Toolkit | MUSA toolkit `5.2.0`, commit `8c44cbed02a79be2066f1f62bdca039177f71d51` |
| Runtime | MUSA runtime `5.2.0`, commit `c92b4e82ce65993b9b996b20eb33b1ffd3185dbd` |
| Compiler / target | mcc `5.2.0` (Clang 14 base); `mp_31` |
| Build dependencies | Boost `1.84.0` through `FETCH_BOOST`; Catch2 `v3.4.0`; OpenMP |

The formal scope/dependency review is
`docs/gluon/unidock-u1r1-adaptation-frontier-independent-review.md`. The U2
build receipt is `docs/gluon/unidock-u2-native-musa-engine.md`. The immutable
U3 freeze and rejected result are
`docs/gluon/unidock-u3-1iep-pre-gpu-freeze.md` and
`docs/gluon/unidock-u3-1iep-musa-correctness.md`. The accepted correction and
final evidence are in
`Docs/Notes/evidence/unidock-u4-correctness-divergence-repair.md`. Those records
remain authoritative when this summary is insufficient.

## Native engine surface

`ENABLE_MUSA=ON` is a separate CMake route. It loads the official modules from
`/usr/local/musa/cmake`, requires `MUSAToolkit 5.2.0` exactly, uses the Ninja
generator, defines `ENABLE_MUSA`, links `MUSA::musart`, and emits native
`mp_31` objects. It does not translate a CUDA SM number into a MUSA target.

The complete engine graph is retained: the 16-source host object library, both
GPU implementations, `unidock`, `split`, and `unit_tests`. The thin
`src/cuda/monte_carlo.mu` and `src/cuda/precalculate.mu` wrappers include the
shared `.cu` algorithm bodies. `src/cuda/musa_compat.h` binds their runtime,
stream/event, memory/copy, error, and Philox calls to MUSA/MURAND. Direct device
callers in `src/main/main.cpp` and `src/main/vina_cuda_worker.h` use MUSA APIs.
The existing launch geometry, layouts, stream/event order, 32-thread
cooperative-group reductions, scoring, transforms, and output path remain
shared with the upstream engine.

The U2 compiler-compatibility changes are limited to the member-template
definition, device-safe atom-type comparison, terminal impossible-path
returns, the MUSA/Boost `__noinline__` collision, and non-PIE linking. They are
not an alternate docking algorithm.

## Reproducible MUSA 5.2 `mp_31` build

Run from the repository root. Use a clean build directory, keep the fetched
dependency versions declared by CMake, and do not substitute Make for Ninja.
The following is the accepted configuration, bounded to two CPUs:

```sh
PATH=/usr/local/musa/bin:$PATH OMP_NUM_THREADS=2 taskset -c 0,1 \
  cmake -S unidock -B .gluon/musa-5.2-build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DENABLE_MUSA=ON -DFETCH_BOOST=ON \
  -DBUILD_FAST_MATH=ON -DMUSA_ARCHITECTURE=mp_31
PATH=/usr/local/musa/bin:$PATH OMP_NUM_THREADS=2 taskset -c 0,1 \
  cmake --build .gluon/musa-5.2-build --parallel 2
PATH=/usr/local/musa/bin:$PATH OMP_NUM_THREADS=2 taskset -c 0,1 \
  ctest --test-dir .gluon/musa-5.2-build --output-on-failure
.gluon/musa-5.2-build/unidock --version
```

The accepted U4 build passed three CTest cases: the existing rigid-PDBQT parser,
the native-MUSA CPU-oracle search-depth case, and the CUDA requested-cap case.
Before any separately authorized device run, verify that `readelf -d` on the
executable requires `libmusart.so.5` and contains neither `libcuda` nor
`libcudart`, and that both native object files contain
`musa-mtgpu-mt-musa--mp_31`. U4 recorded executable SHA-256
`834cb5d6d64891fa623e60a9dbc6255d2413d4fb1cac5333122febeae7b91d08`,
Monte Carlo object SHA-256
`1067fc67e7b5876d8ceb32aca418a71bb375646c0f01f128eb2ddc07010cde36`,
and precalculation object SHA-256
`48425413415b1cc9727fe28cff5999493e9375d296d1235b718d9f616121d1d5`.
Those hashes authenticate the accepted U4 build, not arbitrary rebuilds.

## Accepted 1IEP command and oracle

The only accepted correctness denominator is one PDBQT/Vina ligand on one
assigned MUSA GPU. Its tracked inputs are:

- `unidock/test/receptor/1iep_receptor.pdbqt`, SHA-256
  `761469710e3915b89b274483076dfe49754be7956661bc42f4cc36a182399d59`;
- `unidock/test/ligands/1iep_ligand.pdbqt`, SHA-256
  `37a20e58e77072c6b3ff07f285a345e647dd4e564af316a06e49518b15b7aa61`;
- `unidock/test/reference/1iep_cpu_reference.pdbqt`, SHA-256
  `048067fa044e5ea5fa99cafc333d611057e43f83ca0ecf12b2c0e8cefd7793dc`,
  affinity `-13.277` kcal/mol and 37 mapped heavy atoms.

After a maintenance run has separate device authorization, an output directory,
and a scheduler-exposed single device at local index 0, the frozen command is:

```sh
OMP_NUM_THREADS=2 .gluon/musa-5.2-build/unidock \
  --receptor unidock/test/receptor/1iep_receptor.pdbqt \
  --gpu_batch unidock/test/ligands/1iep_ligand.pdbqt \
  --dir <gpu-output> --device_id 0 \
  --center_x 15.19 --center_y 53.903 --center_z 16.917 \
  --size_x 20 --size_y 20 --size_z 20 \
  --scoring vina --search_mode fast --num_modes 1 --seed 181129
```

Do not override the scheduler's visibility or infer a physical device index.
The command must exit 0 and produce exactly one parseable model with one finite
affinity and the same 37-heavy-atom sequence, mapped by PDBQT serial, atom name,
and AutoDock atom type. Compare the already-produced output with the unchanged
positional interface (there are no tolerance command-line options):

```sh
OMP_NUM_THREADS=2 python3 unidock/test/verify_1iep_correctness.py \
  unidock/test/reference/1iep_cpu_reference.pdbqt \
  <gpu-output>/1iep_ligand_out.pdbqt
```

The candidate affinity must be in `[-20, -1]` kcal/mol, its absolute difference
from the reference must be at most `1.0` kcal/mol, and its direct Cartesian
heavy-atom RMSD must be at most `2.0` A. No fitting or symmetry remapping is
allowed. The comparator SHA-256 at U4 is
`0e27eb9903f8826bafc37696f276ac5253a195a0e0dc35495017bda574a65d94`.

U4 executed the corrected command exactly once on an assigned MTT S5000. The
output SHA-256 was
`852cafe3011dbb9adf8556a1a081ce9a3bcc618f4098aba7c593ca7f9c60572d`;
affinity was `-13.224` kcal/mol, absolute affinity delta was `0.053` kcal/mol,
and direct 37-heavy-atom RMSD was `0.112` A. The process exited 0, MUSA device
utilization was independently observed, and MUSA-only linkage/native objects
excluded CPU or CUDA fallback. This is the accepted evidence; it is not a
request to replay GPU work.

## U3 failure and U4 correction

U3 remains `NO_GO`. Its native-MUSA execution was genuine and exited 0, but its
committed output (SHA-256
`6b6a3e0965a5fcca53928a5e3d011f44c75105123f08e4568c23a7414190f5b5`)
had affinity `-10.386` kcal/mol, a `2.891` kcal/mol delta, and `12.418` A pose
RMSD. Both frozen tolerances failed. Never relabel this result, discard it, or
weaken the oracle.

U4 found the earliest deterministic divergence before a GPU kernel iteration.
Fast mode sets `exhaustiveness=128` and `max_step=20`. For 1IEP, the heuristic
is `41 + 10 * (7 + 6) = 171`, and the CPU-oracle Monte Carlo depth is
`70 * 3 * (50 + 171) / 2 = 23,205`. U3 truncated the GPU search to 20 steps, a
`1160.25`-fold reduction, so CPU and GPU did not receive the same search-depth
contract.

`src/lib/gpu_search_depth.h` now makes the policy explicit: an `ENABLE_MUSA`
correctness build keeps the full heuristic depth, while the non-MUSA/CUDA
throughput route keeps its existing requested cap. Both GPU entry points call
the helper. `unidock/unittest/SearchDepthTests/gpu_search_depth_test.cpp`
requires MUSA/1IEP to select 23,205 instead of 20 and independently requires
the CUDA route to remain capped at 20. A regression that restores the U3
truncation therefore fails the intent test. No Philox, scoring, reduction,
transform, copy, or serialization logic was changed by U4.

## Supported and excluded matrix

| Dimension | Supported denominator | Explicitly excluded / unestablished |
| --- | --- | --- |
| Source | Exact upstream `ebde3b4…` plus accepted U2/U4 commits | Other upstream revisions until the refresh checklist passes |
| Toolkit / target | Exact MUSA `5.2.0`, mcc `5.2.0`, runtime `5.2.0`, `mp_31`, Ninja Release build | Other MUSA versions, compilers, runtimes, architectures, generators, or build modes |
| Hardware / topology | One scheduler-assigned MTT S5000 exposed as local device 0 | Multi-GPU, concurrent jobs, other MUSA devices, physical-index assumptions |
| Correctness case | Exact 1IEP PDBQT/Vina command, fast mode, one pose, seed 181129, fixed box and oracle | Other receptors/ligands, SDF, Vinardo/AD4, other modes/seeds/boxes/pose counts |
| Engine surface | Complete native engine graph and native MUSA runtime route | Python tools, chemistry preparation, containers, packages, benchmarks, workflows |
| Claim | The recorded single-run affinity/pose correctness result | Performance, determinism across repeated runs, CUDA numerical equivalence, production/deployment readiness, general device support |

## Periodic toolkit and upstream refresh checklist

Treat every upstream or toolkit change as a new candidate, not as inherited
support. Perform the following in a controlled maintenance workspace and record
command output, exit status, identities, and SHA-256 receipts. Network access,
GPU access, installation, and formal acceptance require their own authorization.

1. Record the current accepted commits above, the candidate upstream commit,
   `git describe --always --tags`, repository status, engine version, and
   canonical remote. Keep the accepted U3 `NO_GO` and U4 result in the history.
2. Inventory the candidate toolkit without changing the host: record
   `/usr/local/musa/version.json`, `mcc --version`, toolkit/runtime commits,
   `libmusart` identity, official CMake modules, supported generator, and the
   assigned architecture. Stop if these do not match the support row unless the
   task explicitly authorizes qualifying a new matrix entry.
3. Diff the old and candidate upstream revisions across `unidock/CMakeLists.txt`,
   the 16 host sources, both GPU units and shared headers, `main.cpp`,
   `vina_cuda_worker.h`, tests, and the three tracked 1IEP artifacts. Surface
   conflicts; preserve upstream correctness fixes rather than mechanically
   replaying an old patch.
4. Reapply or update only the native surface described above. Confirm that both
   `.mu` wrappers still compile the shared algorithm bodies, the compatibility
   layer still maps every consumed API, both GPU entry points use the depth
   helper, and CUDA architecture/cap policy remains separate.
5. Before building, run source-only checks: `git diff --check`; verify the three
   1IEP hashes and comparator hash with `sha256sum`; inspect the CMake graph and
   API call sites; and run the comparator against the CPU reference itself. The
   self-comparison must report score delta and RMSD `0.000` with 37 mapped heavy
   atoms. It tests the oracle plumbing but is not GPU evidence.
6. Configure and build with the exact two-CPU commands above. Require all three
   intent-bearing CTest cases to pass and `unidock --version` to identify 1.2.0,
   or explicitly review and record an intentional version change.
7. Authenticate the candidate binary before device access: MUSA-only dynamic
   linkage, both `mp_31` native objects, no CUDA dependency, and recorded hashes
   for the binary and objects. A compile/link/load pass alone is not correctness
   acceptance.
8. Before any authorized GPU query or execution, freeze the source/build/toolkit
   identities, input/oracle/comparator hashes, exact command, scheduler-local
   device rule, monitor procedure, expected output, and unchanged tolerances.
9. Under separate single-GPU authorization, execute only the frozen 1IEP case,
   observe the assigned device independently, retain stdout/stderr/output and
   hashes, and run the positional comparator without replaying docking.
10. Report a demonstrated tolerance, linkage, device, or test failure as
    `NO_GO`; report unavailable or prohibited evidence as `INCONCLUSIVE`. Do not
    relax tolerances, replace the CPU oracle with MUSA self-consistency, broaden
    the matrix, or report `GO` when a required check is skipped.
