# Uni-Dock U4 1IEP correctness-divergence repair

## Result

**Technical result: GO.** One corrected native-MUSA execution on the assigned
MTT S5000 exited 0, showed sustained device utilization, and passed the
unchanged frozen 1IEP oracle. The candidate affinity is `-13.224` kcal/mol,
its absolute delta from the `-13.277` kcal/mol CPU reference is `0.053`
kcal/mol, and its direct 37-heavy-atom pose RMSD is `0.112` A. The pre-existing
limits remain exactly `<= 1.0` kcal/mol and `<= 2.0` A. Technical completion is
not formal acceptance.

The U3 result at starting commit
`0159c1324e4ca85ab103a3a2577e26029d53c360` remains **NO_GO**. Its committed
output, `docs/gluon/evidence/unidock-u3-1iep-s5000-output.pdbqt` (SHA-256
`6b6a3e0965a5fcca53928a5e3d011f44c75105123f08e4568c23a7414190f5b5`),
has affinity delta `2.891` kcal/mol and pose RMSD `12.418` A. It was neither
relabelled nor replayed, and no tolerance was relaxed.

## Earliest deterministic divergence

The bounded pre-GPU investigation read the U1, U1R1, U2, and U3 evidence, the
U2 source adaptation, the complete frozen comparator, CPU reference, U3 output,
and relevant host/device search source. It checked the following boundaries in
execution order:

1. command/search-state construction;
2. host initial conformer construction and host-to-device copies;
3. Philox seed, subsequence, offset, constants, and scalar/vector ordering;
4. mutation and coordinate transforms;
5. scoring and warp-reduction ordering; and
6. device-to-host result copy, refinement, ranking, and PDBQT writing.

The earliest reproducible difference occurs at item 1, before any Philox draw
or GPU kernel iteration. `--search_mode fast` deterministically sets
`exhaustiveness=128` and `max_step=20` in `src/main/main.cpp`. For 1IEP the
heuristic is:

```text
41 movable atoms + 10 * (7 ligand torsions + 6 rigid-body DOF) = 171
70 * 3 * (50 + 171) / 2 = 23,205 global Monte Carlo steps
```

The CPU oracle path in `Vina::global_search` uses all 23,205 steps. At U3, the
GPU path computed the same depth and immediately truncated it to 20, a
`1160.25`-fold reduction. Thus the very first search-state contract supplied to
the two algorithms already differed. The U3 affinity/pose failure did not need
to be attributed speculatively to downstream random, reduction, transform, or
handoff behavior.

The MUSA 5.2 MURAND Philox header uses the same Random123 constants and the same
seed/subsequence/offset construction as the CUDA algorithm. A deterministic
seed-`181129`, pose-0 source calculation gave initial words
`2cbb098f 6cc669fc d68ac952 b0723ed7` and first selector `% 9 == 2`; no earlier
MUSA-specific random-state divergence was found. The MUSA compatibility layer
keeps the same `monte_carlo.cu`, warp operations, coordinate transforms, and
copy sequence as the upstream CUDA source.

The exact upstream denominator
`ebde3b4a22c8b2c7b26e523cc15109aab815be21` contains the same GPU 20-step cap.
There is no executable CUDA baseline in this environment, so no CUDA result is
claimed and no MUSA self-consistency result is substituted for the frozen CPU
oracle.

## Causally bounded correction and intent test

`src/lib/gpu_search_depth.h` makes the depth decision explicit. An
`ENABLE_MUSA` correctness build retains the complete CPU-oracle heuristic
depth; the non-MUSA/CUDA route retains its existing requested cap. Both GPU
entry points use that helper, so templated batch groups and the non-template
worker cannot disagree. No Philox, scoring, reduction, transform, memory-copy,
or output logic changed.

`unittest/SearchDepthTests/gpu_search_depth_test.cpp` protects the diagnosed
boundary. It requires 1IEP's MUSA path to select 23,205 rather than 20 steps and
separately requires the explicit non-MUSA/CUDA throughput path to remain capped
at 20. This test would fail if the diagnosed MUSA truncation returned.

The corrected Release build used Ninja, `ENABLE_MUSA=ON`,
`BUILD_FAST_MATH=ON`, `MUSA_ARCHITECTURE=mp_31`, and two build jobs with
`OMP_NUM_THREADS=2`. Configuration and build exited 0. CTest passed all three
tests:

```text
parse pdbqt rigid ............................................. Passed
native MUSA correctness keeps CPU-oracle Monte Carlo depth ... Passed
CUDA throughput preset remains bounded by its requested cap .. Passed
100% tests passed, 0 tests failed out of 3
```

## Pre-execution freeze

Before any GPU query or corrected execution,
`.gluon/u4-pre-gpu-freeze.txt` froze the diagnosis, corrected source and build,
input/oracle hashes, exact command and seed, expected exit and output, local
device rule, monitor command, and unchanged limits. Its SHA-256 is
`d49e7cbd502aae3d8ce1d995e5f1b4a97e40daab0f897ae70e4115470e71ea64`.
It was not changed after execution.

The frozen corrected executable is Uni-Dock 1.2.0, SHA-256
`834cb5d6d64891fa623e60a9dbc6255d2413d4fb1cac5333122febeae7b91d08`.
`readelf` records `NEEDED libmusart.so.5` and no `libcuda` or `libcudart`.
Both native objects contain `musa-mtgpu-mt-musa--mp_31`; their hashes are:

- Monte Carlo: `1067fc67e7b5876d8ceb32aca418a71bb375646c0f01f128eb2ddc07010cde36`.
- Precalculation: `48425413415b1cc9727fe28cff5999493e9375d296d1235b718d9f616121d1d5`.

The resource contract assigned GPU index 0 and the scheduler exposed it through
`MTHREADS_VISIBLE_DEVICES=0`. The frozen command therefore selected local
`--device_id 0`; it did not infer a physical index or change visibility.

## Exactly one corrected S5000 run

Exactly one corrected native-MUSA 1IEP case ran. It exited 0 and produced
`.gluon/u4-evidence/gpu-run/1iep_ligand_out.pdbqt`, SHA-256
`852cafe3011dbb9adf8556a1a081ce9a3bcc618f4098aba7c593ca7f9c60572d`.
No corrected GPU command was replayed. Stdout (SHA-256
`96e2bd251c661620e2408724d878eec8313268b0ea433acf5f103eaf454ffbc8`)
records device 0, 81,829 MiB total memory, the 41-atom/7-torsion ligand in the
native Medium group, `266408.812500 ms` GPU time, `266461 ms` kernel time,
affinity `-13.22`, and `276804 ms` batch time. Stderr is empty.

Independent 100 ms monitoring retained 2,873 rows from
`2026/08/04 00:41:19.868` through `00:46:15.764`. Every row identifies
`MTT S5000`, UUID `e1fd70a2-fcb6-60e3-3cc4-bea59f512937`; 2,687 rows have
nonzero utilization, peak utilization is 100%, and peak allocated memory is
209 MiB. The CSV SHA-256 is
`cba98daddb81c95ad7c29931b61f99e0a1643897eebf6a7102dc6629c00ad560`;
monitor stderr is empty. These receipts plus MUSA-only linkage and native-object
evidence exclude a CPU or CUDA fallback.

## Unchanged oracle result

The frozen comparator is unchanged at SHA-256
`0e27eb9903f8826bafc37696f276ac5253a195a0e0dc35495017bda574a65d94`.
It hard-codes `SCORE_TOLERANCE_KCAL_MOL = 1.0` and
`POSE_TOLERANCE_ANGSTROM = 2.0`.

The pre-execution receipt mistakenly rendered the two comparator paths as
`--reference` and `--candidate` options and redundantly appended tolerance
options. The unchanged script accepts two positional paths and no tolerance
options. That first post-run invocation exited 2 at argument parsing and did
not inspect the result; its stderr is preserved at
`.gluon/u4-evidence/comparator.stderr`, SHA-256
`2a9067c46e501979591097139cafd423f1afb89493c8ce7b19243afcd0191537`.
No GPU work was repeated. The unchanged script was then invoked correctly with
the same already-frozen reference and candidate paths, exited 0, and emitted:

```text
reference_score_kcal_mol=-13.277
candidate_score_kcal_mol=-13.224
absolute_score_delta_kcal_mol=0.053
mapped_heavy_atom_count=37
heavy_atom_pose_rmsd_angstrom=0.112
PASS: frozen 1IEP score and pose tolerances satisfied
```

That receipt is `.gluon/u4-evidence/comparator-positional.stdout`, SHA-256
`26e8379d10b471ecdb6eeafaed5570c0f847b842c5a51c40effa5359269c0b27`;
stderr is empty. Both unchanged conditions pass, so the bounded correction has
technical GO evidence without changing the pre-handover oracle.
