### Ad Samp LAMMPS with PLUMED Tutorial

This tutorial will instruct a user for how to run a full FFS calculation on a simple system, two atoms in a lennard-jones system. 

### System Description

The system used here is not representative of a rare event. Rather it is a simple toy system that allows easy demonstration of the properties of this software. This involves two atoms in harmonic oscilation. This demonstration will show all algorithms usable in this software (for more information see [algorithms](/docs/algorithms.md). 

### Installation

First, all dependencies must be installed and the AdSamp software must be compiled. Instructions may be found at [Installation](/README.md). Ensure that OpenMPI, FFTW, and modified gromacs are loaded (gromacs must be installed and loaded even in LAMMPS simulations).


### Input File Setup

Next, copy the following files into the AdSamp working directory [ffs.mdp](/tutorials/LAMMPSwithPLUMED/ffs.mdp), [plumed.dat](/tutorials/LAMMPSwithPLUMED/plumed.dat). Finally, create an input structure working directory, for this example, the folder should be named `Inputs` , and should include this file, [samplesystem.in](/tutorials/LAMMPSwithPLUMED/samplesystem.in). Rename this file `1.in`. For information regarding the naming convention of input files see [Inputs](/docs/Inputfiles). 

## Exploration 


### Input interaction

With the system set up and the script compiled, run `./AdSamp` to begin login interaction. Input the following:
1. OpenMPI package name: | Your OpenMPI module (Ex: OpenMPI/4.0.5-GCC-10.2.0-CUDA-11.3.1)
2. FFTW package name: | Your FFTW module (Ex: FFTW.MPI/3.3.10-gompi-2022b)
3. Gromacs Installation Location (ex: /home/mal258/ManuelsGromacs): | Your custom Gromacs directory
4. Plumed Installation Location (ex: /home/mal258/ManuelsPlumed): | Your linked plumed directory
5. Postprocessing (Yes: 1, No: 2): | 2
6. Controller Partition (maximize runtime): | pi_balou
7. Controller Max Run Time (D-HH:MM:SS): | 24:00:00
8. email (write 0 for no email): | manuel.lopez@yale.edu
9. Runner Partition (maximize cores): | pi_balou
10. Runner Max Run Time (D-HH:MM:SS): | 24:00:00
11. Runner Cores: | 4
12. MD Step Size (ps): | .001
13. MD Calculator (0: LAMMPS, 1: GROMACS): | 0
14. Using PLUMED (1: yes, 0: no): | 1
15. Output folder: | Exploration
16. Run type (0: Basin, 1: Probabilistic, 2: Exploration): | 2
17. OP Stride: | 100
18. Structure File Delimeter (Including .): | .in
19. Structure File Directory: | Inputs
20. Desired Number of Steps: | 100000
21. Batch Size (for 1 per file return 0): | 0
