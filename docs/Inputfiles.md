## Input Files

This software requires a set of input files in order to function. Mover, these must be formatted and named using the following convention.

## Initial Structure Files (Prior to any run)

All starting structure files must live in the same directory, share a delimiter, and use integer names.

**GROMACS**

```text
structures/
├── 1.gro
├── 2.gro
└── 3.gro
```
**LAMMPS**

```text
structures/
├── 1.in
├── 2.in
└── 3.in
```

**Variable vs Constant Topology/Index (GROMACS ONLY)**

In GROMACS simulations there are input structure files as well as topology `.top` and index `.ndx` files. In cases where all input systems share a common number of atoms these must be placed in the software's working directory under `topol.top` and `index.ndx`. Otherwise, they must be present in a directory (it need not be the same directory but I have done so in this example) and named correspondingly to the structure files. 


```text
structures/
├── 1.gro
├── 1.top
├── 1.ndx
├── 2.gro
├── 2.top
└── 2.ndx
```

## ffs.mdp


All run parameters must be inserted into a file named `ffs.mdp` within the software's working directory. 
### Required fields — GROMACS

```
gen-seed
gen-vel
gen-temp
dt
nstxout
nstvout
nstfout
```

### Required fields — LAMMPS

```
Temperature line (NVT or NPT)
Timestep line
```

> **Do not include a velocity line in the LAMMPS ffs.mdp.** Velocity initialization is handled internally by AdSamp and an explicit velocity line will cause issues.

Sample `ffs.mdp` files may be found in [Sample ffs.mdp](assets/samplefiles/ffs.mdp) and [Sample LAMMPS ffs.mdp](assets/samplefiles/lammpsffs.mdp)


## Order Parameter File

All simulations run with AdSamp require an order parameter file. The two supported approaches are a PLUMED script (supported for GROMACS and LAMMPS) or a custom C++ function (LAMMPS only)

### PLUMED (recommended)

Your `plumed.dat` must contain the following sections. AdSamp checks for all three at login and will fail fast if any are missing or malformed.

| Section | Purpose |
|---|---|
| Mathematical definition | Defines the scalar order parameter |
| `COMMITTOR` | Allows the order parameter to trigger a simulation stop |
| `PRINT` | Outputs the time-dependent parameter to a file |
| `FLUSH` | Forces PLUMED to write output at a defined interval |

AdSamp modifies the targets and output paths in these sections on a per-trajectory basis, so the structure must be uniform across runs. See `templates/plumed.dat` for a working example.

![Example PLUMED script](assets/samplefiles/modelplumed.png)


