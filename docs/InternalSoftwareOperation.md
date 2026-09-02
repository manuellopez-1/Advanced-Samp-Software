## Different Functionalities of the Software

The AdSamp software has five modes that are accessible depending on the total number of inputs given by the user. This allows all of the software's internal operations to be handled within one unified script. These are:

- Login mode: no inputs, just the run command on the software
- Mathematics mode: Begins with Mathematics keyword
- Postprocessing mode: 4 inputs
- Binary Export mode: 6 inputs
- Controller mode: 5 inputs
- Simulation run mode: 15 inputs, simulation type specified as one of these inputs
  - Exploration run
  - Basin run
  - Probabilistic run

## Login Mode

The login mode is triggered when the software is run with no arguments. It runs on the login node and is responsible for gathering all run parameters, validating them before any cluster resources are committed, and generating the bash script(s) that will perform the actual work.

Everything in login mode branches off a single first question:

> **Simulation (1), PostProcessing (2), or Mathematics (3)?**

### Mathematics Branch

Selecting Mathematics collects a partition, max runtime (`D-HH:MM:SS`/`HH:MM:SS`), and email, then asks for a calculation type:

- **Free Energy (0)** — prompts for the number of simulation steps, a postprocessed directory per step (validated to contain a colvar file starting with `C`), a milestone value at each boundary (λ_B, λ_1, λ_2, …), and a `delT` that must evenly divide every interval between milestones.
- **Landscape Variance (1)** — prompts for the number of steps and a postprocessed directory per step (validated to contain `successes.txt`). No milestones or `delT` needed.

Either choice writes a self-contained bash script (`FreeEnergy.sh` or `LandscapeVariance.sh`) that re-invokes the program with a `Mathematics` keyword and returns immediately — this branch never touches the environment or scheduler prompts below.

### Environment Setup (Simulation and PostProcessing only)

Both remaining branches need to know the cluster environment before they can generate anything. In a properly configured build, none of this is interactive — it's baked in at compile time via CMake (`BUILD_OPENMPI_MODULE`, `BUILD_FFTW_MODULE`, `BUILD_GROMACS_DIR`, `BUILD_PLUMED_DIR`, `BUILD_MD_ENGINE`). The login only falls back to asking when a value comes in empty:

- **OpenMPI package name**
- **FFTW package name**
- **MD engine** — LAMMPS or GROMACS. This is fixed at build time and is never asked interactively.
- **GROMACS installation location** — validated by checking for a `bin/` subdirectory; only relevant when the engine is GROMACS.
- **PLUMED installation location** — same `bin/` check; only requested here when the engine is GROMACS. (LAMMPS runs that opt into PLUMED are asked again for this later, in the Order Parameter step below.)

### PostProcessing Branch

Selecting PostProcessing asks for a partition, max runtime, and email, then:

> **Exporting from binary files? (after postprocessing)** (Yes: 1, No: 2)

**No → Standard Postprocessing:** prompts for the most recent run type, the completed run directory, and a new (non-existent) folder name. Probabilistic runs additionally require a **groname file**, which maps probabilistic outputs back to their original structure names. This generates `PostProcessing.sh`.

**Yes → Binary Export:** prompts for a constant-topology flag (GROMACS only, cross-checked against `topol.top`/`index.ndx` in the working directory), a postprocessed directory (validated to contain `.cpt` files), and an output folder name. If the engine is GROMACS, it additionally asks for a root directory of the original `.gro` files and cross-references it against the `.cpt` files with `gronamechecker` to confirm every checkpoint has a matching source structure. This generates `CPTExport.sh`.

Both scripts should be submitted on a partition that isn't scavenge/interruption-prone — see [Binary Export](#binary-export) below for why.

### Simulation Branch

This is the only branch that reaches full run configuration.

**Scheduler settings:**
- Controller partition
- Controller max runtime (`D-HH:MM:SS`/`HH:MM:SS`)
- Email
- Runner partition
- Runner max runtime (same format)
- Runner core count (positive integer)

**Simulation settings:**
- MD step size in picoseconds (positive float). The MD engine itself is not asked here — it's already fixed by the environment setup step above.

**`ffs.mdp` validation** — checked immediately after step size is collected, using the already-fixed engine:
- **GROMACS** — requires `gen-vel`, `gen-temp`, `gen-seed`, `nsteps`, `dt`.
- **LAMMPS** — requires a temperature line (`nvt`/`npt` containing `temp`) and a `timestep` line. A `velocity` line causes an immediate error exit, since velocity initialization is handled internally and an explicit line would conflict with it.

Missing fields are reported individually before the login exits.

**Order parameter validation:**
- If the engine is LAMMPS, the user is asked whether they're using PLUMED or a custom C++ order parameter; if custom, the filename is recorded. GROMACS runs always assume PLUMED.
- If PLUMED is in use, `plumed.dat` is checked for a `COMMITTOR` block, a `PRINT` line, and a `FLUSH` line, with the `COMMITTOR` block's internal line ordering verified via a tick counter. Each missing or malformed piece is reported individually.

**Run type and structure files:**
- **Output folder name** (regex-validated to safe characters, must not already exist), **run type** (Basin 0 / Probabilistic 1 / Exploration 2), and **OP stride** (must be > 0) are collected first.
- The user then supplies a **structure file delimiter** (including the `.`, e.g. `.gro`) and a **structure file directory**. For GROMACS, the delimiter must be `.gro` or `.cpt`. The software checks that matching files exist and are integer-named, repeating the prompt until a valid combination is given.
- For GROMACS with a `.cpt` delimiter, a **root directory** of the original `.gro` files is additionally required and cross-checked with `gronamechecker`.
- For GROMACS with variable topology, matching `.top`/`.ndx` files are required alongside each structure file; with constant topology, `topol.top`/`index.ndx` are required in the working directory instead.

**Run-type-specific parameters:**
- **Basin / Exploration** — number of MD steps (positive int), batch size (0 auto-fills to the number of input files).
- **Probabilistic** — batch size (positive int), max crossings before interruption, trajectories per folder.
- **Basin / Probabilistic** — λ_A and goal lambda, re-prompted if they're equal.

Once every input validates, the login calls `writebash`, generating `AdSampControl.sh` and `AdSamp.sh`. The user submits `AdSampControl.sh` to the scheduler to start the run.

For more information regarding validity of inputs check [Input Files](/docs/Inputfiles.md).

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

