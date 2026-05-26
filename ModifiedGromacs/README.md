# Installing Modified GROMACS 2022.5 with PLUMED

## About This File

GROMACS does not have a built-in way to immediately save a checkpoint file when a simulation is interrupted or terminated. For AdSamp, this behavior is important because Forward Flux Sampling trajectories may be stopped by scheduler time limits, preemption, or signal-based interruption. If no checkpoint is saved at interruption, useful trajectory progress can be lost.

This directory contains a modified version of the GROMACS 2022.5 `md.cpp` file [md.cpp](/ModifiedGromacs/md.cpp). The modified file causes GROMACS to write a final checkpoint/configuration output when an interruption occurs.

The modified file should replace:

```bash
gromacs-2022.5/src/gromacs/mdrun/md.cpp
```

with:

```bash
ModifiedGromacs/md.cpp
```

The only intended source-code difference is the insertion of the final trajectory-writing block after the MD loop.

```cpp
const EkindataState ekindataState2 = bGStat ? (bSumEkinhOld ? EkindataState::UsedNeedToReduce
                                                             : EkindataState::UsedDoNotNeedToReduce)
                                             : EkindataState::NotUsed; // this must be outside the master loop
do_md_trajectory_writing(fplog,
                         cr,
                         nfile,
                         fnm,
                         step,
                         step_rel,
                         t,
                         ir,
                         state,
                         state_global,
                         observablesHistory,
                         top_global,
                         fr,
                         outf,
                         energyOutput,
                         ekind,
                         f.view().force(),
                         true,
                         bRerunMD,
                         bLastStep,
                         mdrunOptions.writeConfout,
                         ekindataState2); // last writing
```

## Installation Overview

The installation process is:

1. Load compiler, MPI, CMake, FFTW, and PLUMED dependencies
2. Download GROMACS 2022.5
3. Replace GROMACS’ original `md.cpp` with the modified AdSamp version (path: gromacs-2022.5/src/gromacs/mdrun/md.cpp)
4. Patch GROMACS with PLUMED
5. Compile and install GROMACS
6. Test that the resulting `gmx_mpi` binary is linked to PLUMED


## 1. Load Modules

Example module setup on a cluster using EasyBuild-style modules:

```bash
module purge

module load foss/2022b
module load CMake/3.24.3-GCCcore-12.2.0
module load FFTW/3.3.10-GCC-12.2.0
```

If PLUMED is installed locally, set its path:

```bash
export PATH=/path/to/local/plumed/bin:$PATH
export LD_LIBRARY_PATH=/path/to/local/plumed/lib:$LD_LIBRARY_PATH
```

Check PLUMED:

```bash
which plumed
plumed info
```



## 1a. Install a Local Version of PLUMED (Required)

AdSamp requires a local PLUMED installation. The PLUMED module available on some clusters may not be compatible with locally compiled GROMACS builds.

Download PLUMED:

```bash
cd /path/to/installation/directory

wget https://github.com/plumed/plumed2/releases/download/v2.10.0/plumed-src-2.10.0.tgz
tar -xzf plumed-src-2.10.0.tgz
cd plumed-2.10.0
```

Configure PLUMED:

```bash
./configure --prefix=/path/to/local/plumed
```

Compile and install (once on an interactive session, salloc -n4) :

```bash
make -j 4
make install
```

Add PLUMED to your environment:

```bash
export PATH=/path/to/local/plumed/bin:$PATH
export LD_LIBRARY_PATH=/path/to/local/plumed/lib:$LD_LIBRARY_PATH
```

Check the installation:

```bash
which plumed
plumed info
```

Expected output should include the installation path and PLUMED version information.


## 2. Download GROMACS 2022.5

```bash
cd /path/to/installation/directory

wget https://ftp.gromacs.org/gromacs/gromacs-2022.5.tar.gz
tar -xzf gromacs-2022.5.tar.gz
cd gromacs-2022.5
```


## 3. Replace `md.cpp` with the Modified Version

From inside the `gromacs-2022.5` directory:

```bash
cp /path/to/Advanced-Samp-Software/ModifiedGromacs/md.cpp src/gromacs/mdrun/md.cpp
```

For example, if the repository is in your home directory:

```bash
cp /path/to/Advanced-Samp-Software/ModifiedGromacs/md.cpp src/gromacs/mdrun/md.cpp
```

## 4. Patch GROMACS with PLUMED

From inside the `gromacs-2022.5` directory:

```bash
plumed patch -p
```

When prompted, select the GROMACS 2022.x patch option.


## 5. Configure the Build

Create a clean build directory:

```bash
mkdir -p build
cd build
```

Configure with CMake:

```bash
cmake .. \
  -DGMX_BUILD_OWN_FFTW=OFF \
  -DGMX_MPI=ON \
  -DGMX_OPENMP=ON \
  -DGMX_DOUBLE=OFF \
  -DGMX_GPU=OFF \
  -DGMX_USE_RDTSCP=OFF \
  -DGMX_INSTALL_NBLIB_API=OFF \
  -DCMAKE_INSTALL_PREFIX=/path/to/local/gromacs
```


## 6. Compile and Install

```bash
make -j 8
make install
```

After installation:

```bash
source /path/to/local/gromacs/bin/GMXRC
```

Check the binary:

```bash
which gmx_mpi
gmx_mpi --version
```


## 7. Confirm PLUMED Support

```bash
gmx_mpi mdrun -h | grep -i plumed
```

Also check linked libraries:

```bash
ldd $(which gmx_mpi) | grep -i plumed
```

Expected output:

```bash
libplumed.so => /path/to/local/plumed/lib/libplumed.so
libplumedKernel.so => /path/to/local/plumed/lib/libplumedKernel.so
```


## 8. Runtime Environment

```bash
module purge

module load foss/2022b
module load CMake/3.24.3-GCCcore-12.2.0
module load FFTW/3.3.10-GCC-12.2.0

export PATH=/path/to/local/plumed/bin:$PATH
export LD_LIBRARY_PATH=/path/to/local/plumed/lib:$LD_LIBRARY_PATH

source /path/to/local/gromacs/bin/GMXRC
```

Verify:

```bash
which gmx_mpi
which plumed
gmx_mpi --version
plumed info
```


## 9. Compile AdSamp for GROMACS Mode

```bash
g++ -std=c++17 AdSamp.cpp orderparameterpicker.cpp -o AdSamp_gmx -I.
```

Run with:

```bash
./AdSamp_gmx
```

## Final Check

```bash
which gmx_mpi
gmx_mpi --version
gmx_mpi mdrun -h | grep -i plumed
ldd $(which gmx_mpi) | grep -i plumed
which plumed
plumed info
```

If these commands succeed, the modified GROMACS 2022.5 installation is ready to use with AdSamp.
