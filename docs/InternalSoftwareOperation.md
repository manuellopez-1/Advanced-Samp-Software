## Different Functionalities of the Software

The AdSamp software has five modes that are accessible depending on the total number of inputs given by the user. This allows all of software's internal operations to be handled internally within one unfiied script.  These are,
- Login mode: no inputs, just the run command on the software.
- Postprocessing mode: 4 inputs
- Binary Export mode: 6 inputs
- Controller mode: 5 inputs
- Simulation run mode: 15 inputs, simulation type specified as one of these inputs
  - Exploration run
  - Basin run
  - Probabilistic run

## Login Mode

The login mode is triggered when the software is run with no arguments. It runs on the login node and is responsible for gathering all run parameters, validating them before any cluster resources are committed, and generating the bash scripts that will perform the actual simulation.

The login flow is divided into two parts.

### Part 1 — Environment and Mode Selection

The user is first prompted for environment information that applies to all run types:

- **OpenMPI package name** — the module name as it appears on the cluster
- **FFTW package name** — same
- **GROMACS installation location** — validated by checking that a `bin/` subdirectory exists at the given path
- **PLUMED installation location** — same check applied
- **Postprocessing mode** — if yes, the login flow branches into either postprocessing or binary export mode and returns early (see those sections below)

### Part 2 — Run Configuration

If the user is not postprocessing, the flow continues to gather simulation parameters:

**Scheduler settings:**
- Controller partition
- max runtime, formatting validated
- email address
- Runner partition
- max runtime, formatting validated
- core count, validated to be an integer

**Simulation settings:**
- MD step size in picoseconds, validated to be a positive float
- MD engine (LAMMPS or GROMACS)

### ffs.mdp Validation

The `ffs.mdp` file in the working directory is checked immediately after the MD engine is selected. The software reads the file and verifies that all required fields are present. Missing fields are reported individually before the login exits.

**GROMACS** — checks for: `gen-vel`, `gen-temp`, `gen-seed`, `nsteps`, `dt`

**LAMMPS** — checks for: a temperature line (`nvt` or `npt` containing `temp`), a `timestep` line. If a `velocity` line is found the login exits immediately with an error, as velocity initialization is handled internally and an explicit line will cause a conflict.

### Order Parameter Validation

If using LAMMPS, the user is asked whether they are using PLUMED or a custom C++ order parameter. If using a custom function, the filename is recorded. For all other cases PLUMED is assumed.

If using PLUMED, `plumed.dat` in the working directory is checked for the following, each reported individually if missing:

- A `COMMITTOR` block
- A `PRINT` line
- A `FLUSH` line
- Correct internal ordering of the `COMMITTOR` block — the software steps through the block line by line using a tick counter and exits with an error if the sequence is malformed

### Structure File Validation

The user provides a file delimiter (including the `.`, e.g. `.gro`) and a structure file directory. The software checks that files of the given type exist in that directory and are named with integers. For GROMACS runs the delimiter must be either `.gro` or `.cpt` — any other value is rejected immediately.

If using GROMACS with variable atom counts, the user is asked whether topology is constant. If not, the software checks that matching `.top` and `.ndx` files exist alongside each structure file. If constant, it checks for `topol.top` and `index.ndx` in the working directory.

If the delimiter is `.cpt` and the engine is GROMACS, the user is additionally prompted for a root directory containing the original `.gro` files. This directory is validated and cross-referenced against the `.cpt` files using `gronamechecker` to confirm that every checkpoint has a matching source structure.

This loop repeats until a valid combination of delimiter and directory is provided.

### Run Type Parameters

The user then specifies:

- **Output folder name** — validated against a regex to ensure it contains only safe characters and does not already exist
- **Run type** — Basin (0), Probabilistic (1), or Exploration (2)
- **Order parameter stride** — how often the OP is evaluated, must be greater than 0

**Basin and Exploration additionally prompt for:**
- Number of MD steps
- Batch size (entering 0 sets batch size to the number of input files automatically)

**Probabilistic additionally prompts for:**
- Batch size
- Max crossings before interruption

**Basin and Probabilistic additionally prompt for:**
- λ_A (return boundary)
- Goal lambda (target milestone)
- These are re-prompted if equal, as equal bounds are invalid

**Probabilistic additionally prompts for:**
- Trajectories per folder

### Output

Once all inputs are validated the login calls `writebash`, which generates `AdSampControl.sh` and `AdSamp.sh` populated with all collected parameters. The user then submits `AdSampControl.sh` to the scheduler to begin the run. 

## Postprocessing

Run with non interruptable partition

## Binary Export

Also run with non interruptable partition

## Controller

## Simulation Run Mode

