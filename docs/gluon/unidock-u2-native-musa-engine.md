# Uni-Dock U2 native-MUSA engine receipt

## Result

**Technical build result: GO. Overall representative GPU correctness:
INCONCLUSIVE.** The complete native-MUSA engine graph configured, compiled,
linked, and passed the permitted CPU-only parser and version checks. No GPU was
queried or executed, so this receipt is not formal acceptance and does not claim
representative docking correctness.

The implementation commit is a direct child of accepted U1R1 result
`5d2f2f341e575e564bf5475ab6ea323abc542670`.

## Implemented boundary

- `ENABLE_MUSA=ON` selects the installed official custom language modules,
  requires exact `MUSAToolkit 5.2.0`, enables MUSA only with Ninja, links
  `MUSA::musart`, and targets the explicitly assigned `mp_31`. The CUDA branch
  and its architecture policy remain separate and unchanged.
- The full existing graph remains present: all 16 host engine sources, the two
  GPU implementations, `unidock`, `split`, and `unit_tests`. Thin `.mu`
  translation-unit wrappers include the original `.cu` implementations because
  mcc selects CUDA mode from a `.cu` suffix even when CMake labels it MUSA.
- `musa_compat.h` binds the existing runtime surface to the exact MUSA runtime
  APIs and the Philox surface to MURAND. Direct device selection/count/memory
  callers and the runtime error boundary use explicit MUSA counterparts.
  Installed cooperative-groups headers keep the existing 32-thread tiles and
  reductions without source changes.
- No kernel, layout, launch size, stream/event order, reduction, scoring math,
  CPU fallback, synthetic gate, SM mapping, or Python tooling was added or
  changed. Concrete mcc fixes were limited to a standards-correct member
  template definition, a device-safe string comparison, terminal returns on
  impossible change-index paths, the MUSA/Boost `__noinline__` collision, and
  non-PIE host linking for mcc objects.
- `FETCH_BOOST=ON` now includes the actually consumed Boost 1.84 modules and
  propagates their headers to the object targets. Catch2 remains upstream
  v3.4.0. Both dependencies remain in `.gluon/u2-build/_deps`.

## Exact CPU-only build evidence

The required final configure, build, test, and version commands used
`OMP_NUM_THREADS=2` and `taskset -c 0,1`. The retained build root is
`/workspace/.gluon/u2-build`.

```text
PATH=/usr/local/musa/bin:$PATH OMP_NUM_THREADS=2 taskset -c 0,1 \
  cmake -S unidock -B .gluon/u2-build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DENABLE_MUSA=ON -DFETCH_BOOST=ON \
  -DBUILD_FAST_MATH=ON -DMUSA_ARCHITECTURE=mp_31
PATH=/usr/local/musa/bin:$PATH OMP_NUM_THREADS=2 taskset -c 0,1 \
  cmake --build .gluon/u2-build --parallel 2
```

The cache records `Release`, `ENABLE_MUSA=ON`, `FETCH_BOOST=ON`,
`MUSA_ARCHITECTURE=mp_31`, and generator `Ninja`. mcc reports version 5.2.0;
the installed toolkit/runtime are 5.2.0. The final default build completed all
targets. The final binary SHA-256 is
`7d3f50693e2af1e7c98acea1aead2e76baea8fa659ec074733b6e988bb929549`.

Native identity was authenticated without loading a device:

- `readelf -d unidock` records `NEEDED libmusart.so.5` and runpath
  `/usr/local/musa/lib:`.
- Both GPU objects contain `musa-mtgpu-mt-musa--mp_31`. Their SHA-256 values are
  `733a14dbfb8f7274222d3ae43a88f2f15dcc5472f3b068a36a9bea9ba033d1f3`
  for Monte Carlo and
  `48425413415b1cc9727fe28cff5999493e9375d296d1235b718d9f616121d1d5`
  for precalculation.

## Required checks

```text
Internal ctest changing into directory: /workspace/.gluon/u2-build
Test project /workspace/.gluon/u2-build
    Start 1: parse pdbqt rigid
1/1 Test #1: parse pdbqt rigid ................   Passed    0.02 sec
100% tests passed, 0 tests failed out of 1

Uni-Dock v1.2.0
```

These paths load the complete native-linked graph but do not query a device.

## Frozen GPU successor

The first separately authorized GPU successor remains exactly the U1R1 1IEP
PDBQT/Vina command and inputs. No GPU command was run here. Input identities
remain:

- receptor: `761469710e3915b89b274483076dfe49754be7956661bc42f4cc36a182399d59`
- ligand: `37a20e58e77072c6b3ff07f285a345e647dd4e564af316a06e49518b15b7aa61`

That later stage must freeze the NVIDIA reference and compare the native-MUSA
pose/affinity under measured tolerances before representative correctness can
be accepted.

## Contract accounting

No GPU access, device query, host install, credentials, service, database,
container, nested worktree, push, deployment, cleanup, deletion, move, rename,
or formal review occurred. Official dependency fetches and all build products
were retained in the workspace-local ignored root. Technical success in this
receipt is not formal acceptance.
