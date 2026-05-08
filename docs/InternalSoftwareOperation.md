## Different Functionalities of the Software

The AdSamp software has five modes that are accessible depending on the total number of inputs given by the user. This allows all of the software's internal operations to be handled within one unified script. These are:

- Login mode: no inputs, just the run command on the software
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

- OpenMPI package name — the module name as it appears on the cluster
- FFTW package name — same
- GROMACS installation location — validated by checking that a `bin/` subdirectory exists at the given path
- PLUMED installation location — same check applied
- Postprocessing mode — if yes, the login flow branches into either postprocessing or binary export mode and returns early (see Part 3 below)

### Part 2 — Run Configuration

If the user is not postprocessing, the flow continues to gather simulation parameters:

**Scheduler settings:**
- Controller partition
- Controller max runtime, validated with `D-HH:MM:SS` or `HH:MM:SS` formatting
- Email address
- Runner partition
- Runner max runtime, validated with `D-HH:MM:SS` or `HH:MM:SS` formatting
- Runner core count, validated to be a positive integer

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

The user provides a file delimiter (including the `.`, e.g. `.gro`) and a structure file directory. The software checks that files of the given type exist in that directory and are named with integers. For GROMACS runs, the delimiter must be either `.gro` or `.cpt` — any other value is rejected immediately.

If using GROMACS with variable atom counts, the user is asked whether topology is constant. If not, the software checks that matching `.top` and `.ndx` files exist alongside each structure file. If constant, it checks for `topol.top` and `index.ndx` in the working directory.

If the delimiter is `.cpt` and the engine is GROMACS, the user is additionally prompted for a root directory containing the original `.gro` files. This directory is validated and cross-referenced against the `.cpt` files using `gronamechecker` to confirm that every checkpoint has a matching source structure.

This loop repeats until a valid combination of delimiter and directory is provided.

### Run Type Parameters

The user then specifies:

- **Output folder name** — validated against a regex to ensure it contains only safe characters and does not already exist
- **Run type** — Basin (0), Probabilistic (1), or Exploration (2)
- **Order parameter stride** — how often the OP is evaluated, must be greater than 0

**Basin and Exploration additionally prompt for:**
- Number of MD steps, validated to be a positive integer
- Batch size (entering 0 sets batch size to the number of input files automatically), validated to be a non-negative integer

**Probabilistic additionally prompts for:**
- Batch size, validated to be a positive integer
- Max crossings before interruption, validated to be a positive integer
- Trajectories per folder, validated to be a positive integer

**Basin and Probabilistic additionally prompt for:**
- λ_A (return boundary)
- Goal lambda (target milestone)
- These are re-prompted if equal, as equal bounds are invalid

### Output

Once all inputs are validated the login calls `writebash`, which generates `AdSampControl.sh` and `AdSamp.sh` populated with all collected parameters. The user then submits `AdSampControl.sh` to the scheduler to begin the run.

For more information regarding validity of inputs check [Input Files](/docs/Inputfiles.md)

### Part 3 — Postprocessing Login Path

If the user selects postprocessing mode during login, the normal simulation setup is skipped. The software instead prompts for information needed to generate postprocessing or binary export scripts.

The user is first asked:

- Postprocessing type — whether to export from binary or produce a postprocessed folder
- Partition
- Max runtime, validated with `D-HH:MM:SS` or `HH:MM:SS` formatting
- Email address

### Standard Postprocessing

If binary export is not selected, the user provides:

- Most recent run type
- Completed run directory, validated to exist
- Postprocessing folder name, validated to not already exist

For probabilistic runs, the user is also prompted for:

- **Groname file** — validated to exist, maps probabilistic outputs back to their original structure names

Once all inputs are validated, the software generates either a `PostProcessing.sh` or `CPTExport.sh` bash script that the user may then submit.

---

## Postprocessing

This run type is activated with a `PostProcessing.sh` bash script, created after interaction with the login mode. This script should not be run on an interruption-prone scavenge partition. For more information regarding postprocessing check [Algorithms](/docs/algorithms.md).

---

## Binary Export

This mode is activated with a `CPTExport.sh` bash script, created after interaction with the login mode. This script should not be run on an interruption-prone scavenge partition, as an interruption mid-export will leave files in a partially converted state with no recovery mechanism.

This script iterates through all `.cpt` files in a postprocessed directory and converts them to readable structure files using the following commands:

### LAMMPS

LAMMPS restart files are converted by initializing a LAMMPS object, reading the checkpoint, and performing a zero-step run with a dump command to write the structure to a readable `.in` file:

1. `read_restart cptfile`
2. `reset_timestep 0`
3. `dump exportdump all atom 1 outputfolder/nom.in`
4. `run 0`
5. `undump exportdump`

### GROMACS

In GROMACS, an `i.mdp` file is first created with the necessary inputs for exporting the current step to a readable format. The `gronames.txt` file is then used to find the name of the original structure file, topology file, and index file. Following this, the conversion proceeds in three steps:

1. `gmx_mpi grompp -f i.mdp -c initialstructurefile -r initialstructurefile -p topologyfile -n indexfile -o tprfile -t cptfile -maxwarn 2`
2. `gmx_mpi mdrun -s tprfile -cpi cptfile -rerun cptfile -noappend -o trrfile`
3. `gmx_mpi trjconv -s tprfile -f trrfile -o outputname -center -pbc mol`

---

## Controller

Background information regarding the controller and worker scripts can be found in [MultiStep](/docs/MultiStepWorkflow.md). This script works by reading all `path.txt` files within the run directories (see [OutputFiles](/docs/outputfiles.md)) and pooling them to produce a global summary in `Runinfo.txt`. The controller also monitors for duplicate instances of itself — if a new controller starts while one is already running, the existing instance is cancelled and the new one takes over. When all worker jobs are detected as complete the controller shuts itself down.

For probabilistic steps, the `Cpropername` colvar files (see [OutputFiles](/docs/outputfiles.md) for naming conventions) are read when successes and failures are recorded in order to determine average path lengths. These files are only read for completed runs, avoiding possible read/write conflicts with active trajectories.

---

## Simulation Run Mode

This run type is the heart of the AdSamp package. The AdSamp controller activates an array of these jobs, each corresponding to a subdirectory and operating on a unique set of trajectories. Each instance receives 15 inputs specifying the MD engine, run type, structure file locations, order parameter bounds, batch size, core count, and other parameters needed to fully define the simulation. These rely on molecular dynamics runs with varying termination criteria as per [Algorithms](/docs/algorithms.md). Interruptions are handled using checkpointing, requeues, and restart commands. Information regarding the embedded LAMMPS and GROMACS commands is given in [Engines](/docs/MDengines.md).

