# Uni-Dock U3 representative 1IEP native-MUSA correctness receipt

## Result: NO_GO

The canonical 1IEP PDBQT/Vina case executed through the complete native MUSA
engine on the assigned S5000 and exited 0 with observed device utilization.
The output failed both numeric comparisons frozen before GPU execution:
absolute affinity delta was `2.891` kcal/mol (limit `1.000`) and mapped
heavy-atom pose RMSD was `12.418` A (limit `2.000`). The correctness goal is
therefore **NO_GO**, without weakening the standard.

The final receipt descends directly through accepted U2 implementation
`67bfdc95b5a1fc2aac38c327a16bac732be7312a`. U3 committed the immutable oracle,
mapping, tolerances, and command at `8fd0861fbc68201216bc8f262e72f6d8ed34ac9e`,
then recorded the scheduler's physical-1-to-local-0 mapping at
`c53d58a880cbdfa1815533eb710de1dfd8ab4eef`, still before GPU execution. See
`docs/gluon/unidock-u3-1iep-pre-gpu-freeze.md` for the complete freeze.

## Build, linkage, and non-fallback evidence

The exact accepted U2 configuration was rebuilt in `.gluon/u3-build` with
`OMP_NUM_THREADS=2`, CPU affinity `0,1`, and two build jobs:

```text
cmake -S unidock -B .gluon/u3-build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DENABLE_MUSA=ON -DFETCH_BOOST=ON \
  -DBUILD_FAST_MATH=ON -DMUSA_ARCHITECTURE=mp_31
cmake --build .gluon/u3-build --parallel 2
```

Configuration and the 288-step default build exited 0. The rebuilt executable
SHA-256 is
`041dbc262ee6f1f8ee6ad3eed0b84756de51ea4ac1821df59d6dc3770a1ae029`.
`readelf -d` records `NEEDED libmusart.so.5` and runpath
`/usr/local/musa/lib:`; it records neither `libcuda` nor `libcudart`. Both MUSA
objects contain `musa-mtgpu-mt-musa--mp_31`. Their hashes are:

- Monte Carlo: `b8d61dcf6161bf6eee482276c0b69c136e2d240a73ca0aeac627cb336dce9794`.
- Precalculation: `48425413415b1cc9727fe28cff5999493e9375d296d1235b718d9f616121d1d5`.

`ctest --test-dir .gluon/u3-build --output-on-failure` passed the parser test
1/1. `unidock --version` returned `Uni-Dock v1.2.0`. The comparator parsed and
passed against the CPU oracle itself before the GPU run.

## Assigned S5000 execution evidence

The resource layer exposed assigned physical GPU 1 as the only local device,
index 0: `MTT S5000`, UUID
`79960515-2565-20f5-7e7e-0bebfe422a33`. The exact command was the one committed
in the pre-GPU freeze, with binary path `.gluon/u3-build/unidock` and output
directory `.gluon/u3-evidence/gpu-run`.

The command exited 0. Stdout identifies device 0, 81,829 MiB total memory,
the 41-atom/7-torsion ligand in the Medium native-GPU group, 526.367 ms GPU
time, and 578 ms kernel time. Its SHA-256 is
`9272028f7e1fdee08c92cda643057270f209f7ec361577aaebfe664047eddea7`.

Independent 100 ms `mthreads-gmi` sampling retained 637 rows from
`2026/08/03 23:42:58.064` through `23:44:03.616`. There were 106 nonzero GPU
samples, peak utilization was 100%, and peak allocated memory was 209 MiB.
Every row identifies the same S5000 UUID. The complete retained CSV SHA-256 is
`c98f0f87da93fedfb96e916fecf40b82e311704d6a2bc176881701069d18742f`;
monitor stderr is empty. This, native kernel timing, the MUSA-only dynamic
dependency, and both native GPU objects jointly exclude a CPU/CUDA fallback.

## Declared output comparison

The exact S5000 output is committed at
`docs/gluon/evidence/unidock-u3-1iep-s5000-output.pdbqt`, SHA-256
`6b6a3e0965a5fcca53928a5e3d011f44c75105123f08e4568c23a7414190f5b5`.
It contains one parseable model, one finite affinity (`-10.386` kcal/mol), all
37 expected mapped heavy atoms, and passes the upstream `[-20, -1]` smoke
range. The frozen CPU oracle is `-13.277` kcal/mol.

```text
reference_score_kcal_mol=-13.277
candidate_score_kcal_mol=-10.386
absolute_score_delta_kcal_mol=2.891
mapped_heavy_atom_count=37
heavy_atom_pose_rmsd_angstrom=12.418
FAIL: score delta 2.891 > 1.000 kcal/mol
FAIL: pose RMSD 12.418 > 2.000 A
```

The S5000 pose is also `12.327` A from the crystallographic input, while the
CPU oracle is `0.896` A from that input. No output is missing and no mapping
ambiguity or symmetry fitting is involved.

## Bounded maintenance handoff

The first exact blocker is search-result divergence under the frozen fast-mode
configuration, not execution, linkage, parsing, device selection, or output
serialization. Static review found no MUSA-only algorithm/layout/launch change:
the accepted port shares the CUDA kernel bodies and replaces only runtime and
Philox APIs with their direct MUSA counterparts. The failed result still meets
upstream's broad smoke assertion, so this receipt does not claim a proven
port-specific runtime defect and does not alter production code speculatively.

The next bounded discriminator is to run this already-frozen command and oracle
on the canonical CUDA implementation at the same source revision. If CUDA
passes, compare CUDA versus MUSA initial randomized conformations and Philox
draws first, then cooperative-group reductions; if CUDA fails identically, the
frozen CPU-to-fast-GPU equivalence is not met by the upstream GPU algorithm.
Neither outcome permits changing U3's recorded tolerances. Do not expand into
tools, additional ligands, performance work, or packaging until that single
discriminator identifies the owner of the divergence.

No credentials, services, containers, deployments, pushes, new docking data,
or cleanup were used. The ignored build and raw evidence roots remain retained.
