### Ad Samp LAMMPS with PLUMED Tutorial

This tutorial will instruct a user for how to run a full FFS calculation on a simple system, two atoms in a lennard-jones system. 

### System Description

The system used here is not representative of a rare event. Rather it is a simple toy system that allows easy demonstration of the properties of this software. This involves two atoms in harmonic oscilation. This demonstration will show all algorithms usable in this software (for more information see [algorithms](/docs/algorithms.md). 

### Installation

First, all dependencies must be installed and the AdSamp software must be compiled. Instructions may be found at [Installation](/README.md).


### Input File Setup

Next, copy the following files into the AdSamp working directory [ffs.mdp](/tutorials/LAMMPSwithPLUMED/ffs.mdp), [plumed.dat](/tutorials/LAMMPSwithPLUMED/plumed.dat). Finally, create an input structure working directory, for this example, the folder should be named `Inputs` , and should include this file, [samplesystem.in](/tutorials/LAMMPSwithPLUMED/samplesystem.in). Rename this file `1.in`. For information regarding the naming convention of input files see [Inputs](/docs/Inputfiles). 


## Input interaction

With the system set up and the script compiled, 
