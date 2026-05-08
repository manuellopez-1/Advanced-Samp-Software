### Ad Samp LAMMPS with PLUMED Tutorial

This tutorial will instruct a user for how to run a full FFS calculation on a simple system, two atoms in a lennard-jones system. For information on the overall flow of simulations, see [multistep](/docs/multistep.md).  

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

You will notice safety features throughout, for example if you misformat the run times you will be prompted to retype this. 

### Launching simulation
Following these inputs the following two files will have been created: `AdSamp.sh` and `AdSampController.sh`. To launch the simulations simply `sbatch AdSampController.sh` note that the controller will launch `AdSamp.sh` do not launch this one. There are some safety features built in here. For example if you launch `AdSampController.sh` twice the newer one will cancel the former and continue monitoring the simulations. These scripts will run until the simulation is complete, at which point an email will be sent if the account was provided. 


### Outputs

Once the simulation has run all outputs described in [outputs](/docs/outputfiles.md) will be generated under the folder `Exploration/`. Moreover, though this simulation is quick, a live file called `RunInfo.txt` was generated in the working directory to give the completed number of simulations over time. Now, in order for this run to be used as an input for the next simulation, the run must be postprocessed.


### Input interaction
To begin postprocessing, run `./AdSamp` to begin login interaction. Input the following:

1. OpenMPI package name: | Your OpenMPI module (Ex: OpenMPI/4.0.5-GCC-10.2.0-CUDA-11.3.1)
2. FFTW package name: | Your FFTW module (Ex: FFTW.MPI/3.3.10-gompi-2022b)
3. Gromacs Installation Location (ex: /home/mal258/ManuelsGromacs): | Your custom Gromacs directory
4. Plumed Installation Location (ex: /home/mal258/ManuelsPlumed): | Your linked plumed directory
5. Postprocessing (Yes: 1, No: 2): | 1
6. Exporting from binary files? (after postprocessing) (Yes: 1, No: 2): | 2
7. Partition: | pi_balou
8. Max Run Time (D-HH:MM:SS): | 24:00:00
9. email (write 0 for no email): | manuel.lopez@yale.edu
10. Most Recent Run type (0: Basin, 1: Probabilistic, 2: Exploration): | 2
11. Completed Run Directory: | Exploration
12. Postprocessing folder name: | ProcessedExploration

Once these inptus have been given, a file called `PostProcessing.sh` will be created, submit it using `sbatch PostProcessing.sh` to develop a postprocessed folder, for more information on this process read [Algorithms](/docs/algorithms.md).

## Basin Simulation

### Input interaction

The next step is a basin simulation using the outputs of the exploration as an input run `./AdSamp` to begin login interaction. Input the following:

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
15. Output folder: | Basin
16. Run type (0: Basin, 1: Probabilistic, 2: Exploration): | 0
17. OP Stride: | 100
18. Structure File Delimeter (Including .): | .cpt
19. Structure File Directory: | ProcessedExploration
20. Desired Number of Steps: | 10000000
21. Batch Size (for 1 per file return 0): | 0
22. lambda_A: | 7
23. Goal Lambda: | 4

Now launch `sbatch AdSampControl.sh` to begin the basin simulation.
