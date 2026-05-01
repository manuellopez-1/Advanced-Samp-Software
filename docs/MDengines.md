# MD Engines

AdSamp supports two molecular dynamics engines: LAMMPS and GROMACS. The workflow and output structure are identical between them, but the way AdSamp interacts with each engine differs significantly under the hood. This page documents those differences and the commands used for each engine.

---

## LAMMPS

LAMMPS can be embedded directly in a C++ program and used as an object. AdSamp initializes a LAMMPS object with the run information and structure file, then runs it a set number of steps at a time. This direct interaction makes order parameter calculation flexible — both PLUMED and custom C++ functions are supported.

Parallelism is handled by initializing with OpenMPI thread splitting and passing each thread to the LAMMPS initialization and run calls, as well as to any custom order parameter calculations.

## GROMACS

GROMACS cannot be interacted with directly in C++. AdSamp instead uses system calls to drive GROMACS, with PLUMED handling order parameter computation, output, and trajectory termination. After each run terminates, AdSamp reads the PLUMED output to determine the reason for exit and decides how to proceed.

Parallelism for GROMACS is handled differently. Passing all threads to the system calls would produce that many concurrent single-core processes rather than one multi-core run. Instead, AdSamp idles the non-principal threads and issues a single call with unbound overlapping cores, allowing the idle threads to join the process.

### Modified GROMACS

GROMACS was internally modified for use with AdSamp to fix checkpoint saving behavior. The standard approach — saving every few steps — was prohibitively slow when operating across many cores, as all threads would divide the MD work only to recombine frequently for writes. The modified version appends to GROMACS's internal interruption mechanism so that the current state is saved immediately upon any interruption signal, whether from an HPC cancellation, a timeout, or an order parameter being reached. This allows saves to occur far less frequently during normal operation without sacrificing accuracy.
![Speedup](/assets/GROMACSMOD.png)

See the [setup guide](../README.md) for instructions on installing the modified GROMACS build.

---

## Command Reference

The table below documents the initialization and run commands AdSamp uses internally for each engine across all simulation contexts.

| | GROMACS | LAMMPS |
|---|---|---|
| **Initialization** | | |
| Standard | `gmx_mpi grompp -f ffs.mdp -c n.gro -r n.gro -p n.top -n n.ndx -o propername -maxwarn 2` | `include n.in` `include ffs.mdp` |
| With PLUMED | — | `... fix cv all plumed plumedfile plumed.dat outfile propername.log` |
| Continuation (Probabilistic) | `... -cpi propername.cpt` | `read_restart propername.cpt` `reset_timestep 0` `include propername.mdp` |
| With PLUMED | — | `... fix cv all plumed plumedfile propername.dat outfile propername.log` |
| Continuation (Limited Steps) | No command, reuse existing `.tpr` file | Same as probabilistic continuation |
| **Run** | | |
| Standard | `gmx_mpi mdrun -v -deffnm -plumed plumed.dat -c propername.gro -append -cpt 5 -ntomp 1` | `run strides pre no post no` |
| With PLUMED | — | `run strides pre yes post no` |
| Continuation (Probabilistic) | Same as standard run | `run strides pre no post no` |
| With PLUMED | — | `run strides pre yes post no` |
| Continuation (Limited Steps) | `... -cpi propername.cpt` | `run strides pre no post no` |
| With PLUMED | — | `run strides pre yes post no` |
