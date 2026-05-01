# Outputs

AdSamp uses an identical output structure regardless of whether you are running LAMMPS or GROMACS. All outputs are organized under a user-named output directory within the working directory. A live summary file, `Runinfo.txt`, is written directly to the working directory and updated continuously by the controller script.

---

## Directory Structure

Each numbered subdirectory corresponds to the batch array task ID of the job working in that folder.

```plaintext
workingdir/
├── Runinfo.txt
└── outputdir/
    ├── 1/
    │   ├── seedvalues
    │   ├── outputfiles
    │   ├── iterationtracker
    │   ├── possibleoutputs
    │   ├── path.txt
    │   ├── basininfo.txt (basin runs only)
    │   ├── basincrossingrecorded.txt (basin runs only)
    │   ├── Cfolder_X_subfolder_1_initialconfiguration_Y_iteration_Z (Time Dependent Order Parameter)
    │   ├── folder_X_subfolder_1_initialconfiguration_Y_iteration_Z.dat (Modified Plumed File)
    │   ├── folder_X_subfolder_1_initialconfiguration_Y_iteration_Z.mdp (Modified mdp File)
    │   └── folder_X_subfolder_1_initialconfiguration_Y_iteration_Z.cpt
    └── 2/
        └── ...
```

![Outputfile Organization](/assets/outputfileorganization.png)

---

## Bookkeeping Files

These four files exist in each numbered subdirectory and are written to support reproducibility.

| File | Contents |
|---|---|
| `seedvalues` | Random velocity initialization seed used per trajectory |
| `outputfiles` | Structure file used for each trajectory |
| `iterationtracker` | Iteration count per structure file (e.g. 2 if this file has been used before) |
| `possibleoutputs` | All structure files that were eligible to be selected for this run |

---

## path.txt

`path.txt` is the primary bookkeeping file read by the controller to update `Runinfo.txt`. Each line corresponds to one trajectory and contains the following fields:

```plaintext
structure_file,   iteration,   propername,   status
```

The value of `status` depends on the simulation type:

| Simulation type | Status values |
|---|---|
| Exploration | `complete` or `incomplete` |
| Basin | Number of crossings recorded |
| Probabilistic | `success`, `failure`, or `incomplete` |

---

## Propername

Every trajectory is assigned a unique tag called its propername, used to name all of its associated files. The structure is:

```plaintext
folder_X_subfolder_Y_initialconfiguration_Z_iteration_N
```

This naming scheme guarantees no collisions within a run. The files carrying this tag are:

| File | Description |
|---|---|
| `propername.dat` | Per-trajectory copy of `plumed.dat`, modified with trajectory-specific targets. In basin runs, only one order parameter boundary is active. |
| `propername.mdp` | Per-trajectory copy of `ffs.mdp`, modified with a unique velocity initialization seed |
| `Cpropername` | Order parameter value over time (colvar file) |
| `RCpropername` | Colvar restarter file, present only if the run was continued after interruption |
| `propername.cpt` | Most recent checkpoint, used to resume the trajectory |

---

## Basin-Only Files

Two additional files appear in each subdirectory when running a basin simulation:

| File | Contents |
|---|---|
| `basininfo.txt` | Current run mode per trajectory — whether the trajectory (1) is advancing toward the goal milestone or resetting toward $\lambda_A_$  (2)|
| `basincrossingrecorded.txt` | Timestamps of each recorded crossing |

---

## Postprocessing

Postprocessing must be run before proceeding to the next simulation step. Running `PostProcessing.sh` does the following:

1. Extracts colvar files, checkpoints, `path.txt`, and bookkeeping files from each numbered subdirectory into a single flat postprocessed directory
2. Prompts for the previous run directory and generates `gronames.txt`, which maps each output structure to its initial configuration
3. Appends 'folder_x' to bookkeeping files and moves them to the new folder
4. Creates a global path file.

`gronames.txt` is required for GROMACS continuation runs, since GROMACS needs the original input structure to initialize the next simulation. It is also useful for analysis when tracing a trajectory back to its starting point.

> Skipping postprocessing and proceeding directly to the next step will cause the run to fail.
