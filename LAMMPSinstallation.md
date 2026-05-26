# Installing LAMMPS with PLUMED

## About This File

Unlike the modified GROMACS build used by AdSamp, LAMMPS does not require any source-code modification. A standard LAMMPS installation compiled with PLUMED support is sufficient.

AdSamp interacts with LAMMPS through the shared LAMMPS library (`liblammps.so`), so LAMMPS must be compiled with:

- MPI support
- Shared library support
- PLUMED support

This document describes how to install a local version of PLUMED and compile a compatible version of LAMMPS.

## Installation Overview

The installation process is:

1. Load compiler, MPI, and CMake dependencies
2. Install a local version of PLUMED
3. Download and configure LAMMPS
4. Compile LAMMPS with PLUMED and shared-library support
5. Verify the installation
6. Compile AdSamp in LAMMPS mode

## 1. Load Modules

Example module setup on a cluster using EasyBuild-style modules:

```bash
module purge

module load foss/2022b
module load CMake/3.24.3-GCCcore-12.2.0
```

Check MPI:

```bash
which mpicxx
mpicxx --version
```

## 2. Install a Local Version of PLUMED

AdSamp requires a local PLUMED installation. System-wide PLUMED modules are not always compatible with locally compiled LAMMPS builds.

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

Compile and install (with interactive allocation: salloc -n4) :

```bash
make -j 4
make install
```

Add PLUMED to your environment:

```bash
export PATH=/path/to/local/plumed/bin:$PATH
export LD_LIBRARY_PATH=/path/to/local/plumed/lib:$LD_LIBRARY_PATH
```

Verify the installation:

```bash
which plumed
plumed info
```

Expected output should include the PLUMED installation directory and version information.

## 3. Download LAMMPS

Download LAMMPS:

```bash
cd /path/to/installation/directory

git clone https://github.com/lammps/lammps.git
cd lammps
```

Optionally checkout a stable version:

```bash
git checkout stable
```

## 4. Configure LAMMPS with PLUMED

Create a clean build directory:

```bash
mkdir build
cd build
```

Configure using CMake:

```bash
cmake ../cmake \
  -DBUILD_MPI=yes \
  -DBUILD_SHARED_LIBS=yes \
  -DPKG_PLUMED=yes \
  -DPLUMED_ROOT=/path/to/local/plumed \
  -DCMAKE_INSTALL_PREFIX=/path/to/local/lammps
```

Important notes:

- `BUILD_SHARED_LIBS=yes` is required because AdSamp links against `liblammps.so`
- `PKG_PLUMED=yes` enables PLUMED support
- `BUILD_MPI=yes` enables MPI support
- `PLUMED_ROOT` should point to the local PLUMED installation

## 5. Compile and Install LAMMPS

Compile:

```bash
make -j 8
```

Install:

```bash
make install
```

The resulting installation should contain:

```text
/path/to/local/lammps/bin
/path/to/local/lammps/include
/path/to/local/lammps/lib64
```

The shared library:

```text
liblammps.so
```

should exist inside the `lib64` directory.

## 6. Runtime Environment

Before using LAMMPS or compiling AdSamp, load the same compiler/MPI environment used during compilation and export the PLUMED and LAMMPS paths:

```bash
module purge

module load foss/2022b
module load CMake/3.24.3-GCCcore-12.2.0

export PATH=/path/to/local/plumed/bin:$PATH
export LD_LIBRARY_PATH=/path/to/local/plumed/lib:$LD_LIBRARY_PATH

export PATH=/path/to/local/lammps/bin:$PATH
export LD_LIBRARY_PATH=/path/to/local/lammps/lib64:$LD_LIBRARY_PATH
```

Verify:

```bash
which lmp
which plumed
```

## 7. Verify PLUMED Support

Check that LAMMPS was compiled with PLUMED:

```bash
lmp -h | grep -i plumed
```

Check linked libraries:

```bash
ldd /path/to/local/lammps/lib64/liblammps.so | grep -i plumed
```

Expected output should resemble:

```bash
libplumed.so => /path/to/local/plumed/lib/libplumed.so
libplumedKernel.so => /path/to/local/plumed/lib/libplumedKernel.so
```

You can also confirm that PLUMED symbols are present:

```bash
strings /path/to/local/lammps/lib64/liblammps.so | grep -i plumed | head
```

## 8. Compile AdSamp in LAMMPS Mode

Compile AdSamp using:

```bash
mpicxx -std=c++17 -DUSE_LAMMPS \
  AdSamp.cpp orderparameterpicker.cpp \
  -o AdSamp \
  -I. \
  -I/path/to/local/lammps/include \
  -L/path/to/local/lammps/lib64 \
  -llammps \
  -Wl,-rpath,/path/to/local/lammps/lib64
```

Run using:

```bash
./AdSamp
```

## Final Check

A working installation should satisfy all of the following:

```bash
which lmp
which plumed

lmp -h | grep -i plumed

ldd /path/to/local/lammps/lib64/liblammps.so | grep -i plumed

plumed info
```

If these commands succeed, the LAMMPS installation is ready to use with AdSamp.
