# Uni-Dock U3 1IEP pre-GPU correctness freeze

This receipt was committed before any GPU query or execution in U3. It freezes
the complete representative case and its acceptance standard; later results
must not weaken or reinterpret it.

## Identities and routes

- Accepted implementation parent: `67bfdc95b5a1fc2aac38c327a16bac732be7312a`.
- Upstream denominator: `ebde3b4a22c8b2c7b26e523cc15109aab815be21`.
- Receptor: `unidock/test/receptor/1iep_receptor.pdbqt`, SHA-256
  `761469710e3915b89b274483076dfe49754be7956661bc42f4cc36a182399d59`.
- Ligand: `unidock/test/ligands/1iep_ligand.pdbqt`, SHA-256
  `37a20e58e77072c6b3ff07f285a345e647dd4e564af316a06e49518b15b7aa61`.
- CPU oracle: `unidock/test/reference/1iep_cpu_reference.pdbqt`, SHA-256
  `048067fa044e5ea5fa99cafc333d611057e43f83ca0ecf12b2c0e8cefd7793dc`.
- CPU oracle generation exited 0, reported affinity `-13.277` kcal/mol, wrote
  one model with 37 mapped heavy atoms, and has a direct heavy-atom Cartesian
  RMSD of `0.896095` A from the crystallographic input coordinates.
- Native MUSA output is `1iep_ligand_out.pdbqt` in the declared GPU output
  directory. There is no allowed CUDA or CPU fallback for that command.

The CPU oracle was generated from the same Release MUSA-linked binary with the
GPU batch argument replaced by the CPU `--ligand` route, an explicit output,
and `--cpu 2`:

```text
OMP_NUM_THREADS=2 unidock \
  --receptor unidock/test/receptor/1iep_receptor.pdbqt \
  --ligand unidock/test/ligands/1iep_ligand.pdbqt \
  --out <cpu-output>/1iep_ligand_out.pdbqt \
  --center_x 15.19 --center_y 53.903 --center_z 16.917 \
  --size_x 20 --size_y 20 --size_z 20 \
  --scoring vina --search_mode fast --num_modes 1 --seed 181129 --cpu 2
```

The frozen native MUSA command is:

```text
MUSA_VISIBLE_DEVICES=1 OMP_NUM_THREADS=2 unidock \
  --receptor unidock/test/receptor/1iep_receptor.pdbqt \
  --gpu_batch unidock/test/ligands/1iep_ligand.pdbqt \
  --dir <gpu-output> --device_id 0 \
  --center_x 15.19 --center_y 53.903 --center_z 16.917 \
  --size_x 20 --size_y 20 --size_z 20 \
  --scoring vina --search_mode fast --num_modes 1 --seed 181129
```

`MUSA_VISIBLE_DEVICES=1` restricts the process to assigned physical GPU 1; it
therefore addresses that visible device as `--device_id 0`.

## Frozen comparisons and exit standard

The GPU command must exit 0 and emit exactly one parseable model and one finite
Vina affinity. The affinity must remain in the upstream smoke interval
`[-20, -1]` kcal/mol and differ from the CPU oracle by at most `1.0` kcal/mol.
The pose must have the same 37-heavy-atom sequence, mapped exactly by PDBQT atom
serial, atom name, and AutoDock atom type. The direct Cartesian RMSD over those
mapped coordinates (no fitting or symmetry remapping) must be at most `2.0` A.

`unidock/test/verify_1iep_correctness.py` implements that frozen mapping and
both numeric tolerances. Output byte identity is not required because the CPU
and MUSA searches use distinct parallel random-number and reduction paths.

## Pre-execution engine requirements

The build is the accepted U2 configuration: Release, Ninja, `ENABLE_MUSA=ON`,
`FETCH_BOOST=ON`, `BUILD_FAST_MATH=ON`, MUSA 5.2.0, and `mp_31`. Before GPU
execution, `ctest` must pass, the executable must require `libmusart.so.5`, both
GPU objects must contain `musa-mtgpu-mt-musa--mp_31`, and neither `libcuda` nor
`libcudart` may appear in the dynamic dependencies. During execution, assigned
S5000 utilization must be observed through `mthreads-gmi` while the process is
alive. Any failure is reported without changing this freeze.
