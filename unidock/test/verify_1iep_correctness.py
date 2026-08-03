#!/usr/bin/env python3
"""Compare the frozen 1IEP CPU oracle with one native-GPU docking output."""

import argparse
import math
from pathlib import Path


SCORE_TOLERANCE_KCAL_MOL = 1.0
POSE_TOLERANCE_ANGSTROM = 2.0
SMOKE_SCORE_RANGE = (-20.0, -1.0)


def read_pose(path: Path):
    models = 0
    scores = []
    heavy_atoms = []
    for line in path.read_text(encoding="utf-8").splitlines():
        if line.startswith("MODEL"):
            models += 1
        elif line.startswith("REMARK VINA RESULT:"):
            scores.append(float(line.split()[3]))
        elif line.startswith(("ATOM", "HETATM")):
            atom_type = line.split()[-1]
            if not atom_type.upper().startswith("H"):
                key = (int(line[6:11]), line[12:16].strip(), atom_type)
                xyz = tuple(float(line[start : start + 8]) for start in (30, 38, 46))
                heavy_atoms.append((key, xyz))
    if models != 1:
        raise ValueError(f"{path}: expected exactly one MODEL, found {models}")
    if len(scores) != 1 or not math.isfinite(scores[0]):
        raise ValueError(f"{path}: expected exactly one finite Vina score")
    if not heavy_atoms:
        raise ValueError(f"{path}: no heavy atoms found")
    return scores[0], heavy_atoms


def rmsd(reference, candidate):
    reference_keys = [atom[0] for atom in reference]
    candidate_keys = [atom[0] for atom in candidate]
    if candidate_keys != reference_keys:
        raise ValueError("heavy-atom mapping mismatch (serial, name, AutoDock type)")
    squared = sum(
        sum((left - right) ** 2 for left, right in zip(ref_xyz, candidate_xyz))
        for (_, ref_xyz), (_, candidate_xyz) in zip(reference, candidate)
    )
    return math.sqrt(squared / len(reference))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("reference", type=Path)
    parser.add_argument("candidate", type=Path)
    args = parser.parse_args()

    reference_score, reference_atoms = read_pose(args.reference)
    candidate_score, candidate_atoms = read_pose(args.candidate)
    score_delta = abs(candidate_score - reference_score)
    pose_rmsd = rmsd(reference_atoms, candidate_atoms)

    failures = []
    if not SMOKE_SCORE_RANGE[0] <= candidate_score <= SMOKE_SCORE_RANGE[1]:
        failures.append(f"candidate score {candidate_score:.3f} outside {SMOKE_SCORE_RANGE}")
    if score_delta > SCORE_TOLERANCE_KCAL_MOL:
        failures.append(
            f"score delta {score_delta:.3f} > {SCORE_TOLERANCE_KCAL_MOL:.3f} kcal/mol"
        )
    if pose_rmsd > POSE_TOLERANCE_ANGSTROM:
        failures.append(f"pose RMSD {pose_rmsd:.3f} > {POSE_TOLERANCE_ANGSTROM:.3f} A")

    print(f"reference_score_kcal_mol={reference_score:.3f}")
    print(f"candidate_score_kcal_mol={candidate_score:.3f}")
    print(f"absolute_score_delta_kcal_mol={score_delta:.3f}")
    print(f"mapped_heavy_atom_count={len(reference_atoms)}")
    print(f"heavy_atom_pose_rmsd_angstrom={pose_rmsd:.3f}")
    if failures:
        for failure in failures:
            print(f"FAIL: {failure}")
        raise SystemExit(1)
    print("PASS: frozen 1IEP score and pose tolerances satisfied")


if __name__ == "__main__":
    main()
