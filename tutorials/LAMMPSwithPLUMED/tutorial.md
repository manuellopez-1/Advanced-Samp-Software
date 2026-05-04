### Ad Samp LAMMPS with PLUMED Tutorial

This tutorial will instruct a user for how to run a full FFS calculation on a simple system, two atoms in a lennard-jones system. 

### System Description

### Installation

First, all dependencies must be installed and the AdSamp software must be compiled. Instructions may be found at [Installation](/README.md).


### Input File Setup

Next, copy the following files into the AdSamp working directory [ffs.mdp](/tutorials/LAMMPSwithPLUMED/ffs.mdp), [plumed.dat](/tutorials/LAMMPSwithPLUMED/plumed.dat). Finally, create an input structure working directory, for this example I will create `Inputs`, and copy in [samplesystem.in](/tutorials/LAMMPSwithPLUMED/samplesystem.in). Rename this file `1.in`.

For information regarding the naming convention of input files see [Inputs](/docs/Inputfiles).

