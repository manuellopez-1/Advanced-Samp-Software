cmake_minimum_required(VERSION 3.16)
project(AdSamp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(MPI REQUIRED)

execute_process(
    COMMAND bash -c "module -t list 2>&1 | grep -i openmpi | head -n1"
    OUTPUT_VARIABLE DETECTED_OPENMPI
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET)

execute_process(
    COMMAND bash -c "module -t list 2>&1 | grep -i fftw | head -n1"
    OUTPUT_VARIABLE DETECTED_FFTW
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET)

message(STATUS "OpenMPI module: '${DETECTED_OPENMPI}'")
message(STATUS "FFTW module:    '${DETECTED_FFTW}'")

set(MD_ENGINE "" CACHE STRING "Which MD engine to build for: LAMMPS or GROMACS")
set_property(CACHE MD_ENGINE PROPERTY STRINGS LAMMPS GROMACS)

if(NOT MD_ENGINE STREQUAL "LAMMPS" AND NOT MD_ENGINE STREQUAL "GROMACS")
    message(FATAL_ERROR
        "You must choose an MD engine:\n"
        "  cmake .. -DMD_ENGINE=LAMMPS -DLAMMPS_ROOT=/path/to/lammps -DPLUMED_DIR=/path/to/plumed\n"
        "  cmake .. -DMD_ENGINE=GROMACS -DGROMACS_DIR=/path -DPLUMED_DIR=/path [-DFFTW_MODULE=fftw/x.y.z]")
endif()

if(MD_ENGINE STREQUAL "LAMMPS")
	find_path(LAMMPS_INCLUDE_DIR NAMES lammps/lammps.h HINTS $ENV{LAMMPS_DIR} ${LAMMPS_ROOT} PATH_SUFFIXES include)
	find_library(LAMMPS_LIBRARY NAMES lammps HINTS $ENV{LAMMPS_DIR} ${LAMMPS_ROOT} PATH_SUFFIXES lib lib64)
	set(FFTW_MODULE "" CACHE STRING "FFTW module name, e.g. fftw/3.3.10 (only needed if not auto-detected from loaded modules)")
	if(FFTW_MODULE STREQUAL "")
        	set(FFTW_MODULE "${DETECTED_FFTW}")
    	endif()

    	if(FFTW_MODULE STREQUAL "")
        	message(FATAL_ERROR "No FFTW module found. Either `module load` one before running cmake, or pass -DFFTW_MODULE=fftw/x.y.z")
	endif()

	set(PLUMED_DIR "" CACHE PATH "PLUMED install prefix (contains bin/, lib/)")
	find_library(PLUMED_LIBRARY NAMES plumed HINTS $ENV{PLUMED_DIR} ${PLUMED_DIR} PATH_SUFFIXES lib lib64)
	find_library(PLUMED_KERNEL_LIBRARY NAMES plumedKernel HINTS $ENV{PLUMED_DIR} ${PLUMED_DIR} PATH_SUFFIXES lib lib64)

	if(NOT LAMMPS_INCLUDE_DIR OR NOT LAMMPS_LIBRARY)
		message(FATAL_ERROR "LAMMPS not found, point to it with one of:\n cmake .. -DMD_ENGINE=LAMMPS -DLAMMPS_ROOT=/path/to/lammps\n export LAMMPS_DIR=/path/to/lammps (before running cmake)")
	endif()

	if(NOT PLUMED_LIBRARY OR NOT PLUMED_KERNEL_LIBRARY)
		message(FATAL_ERROR "PLUMED libraries not found, point to them with one of:\n cmake .. -DMD_ENGINE=LAMMPS -DLAMMPS_ROOT=/path/to/lammps -DPLUMED_DIR=/path/to/plumed\n export PLUMED_DIR=/path/to/plumed (before running cmake)")
	endif()

	message(STATUS "LAMMPS include: ${LAMMPS_INCLUDE_DIR}")
	message(STATUS "LAMMPS library: ${LAMMPS_LIBRARY}")
	message(STATUS "PLUMED library: ${PLUMED_LIBRARY}")
	message(STATUS "PLUMED kernel:  ${PLUMED_KERNEL_LIBRARY}")
endif()

if(MD_ENGINE STREQUAL "GROMACS")
    set(GROMACS_DIR "" CACHE PATH "GROMACS install prefix (contains bin/, lib/)")
    set(PLUMED_DIR "" CACHE PATH "PLUMED install prefix (contains bin/, lib/)")
    set(FFTW_MODULE "" CACHE STRING "FFTW module name, e.g. fftw/3.3.10 (only needed if not auto-detected from loaded modules)")

    if(NOT GROMACS_DIR OR NOT EXISTS "${GROMACS_DIR}/bin")
        message(FATAL_ERROR "GROMACS not found. Pass -DGROMACS_DIR=/path/to/gromacs (must contain bin/)")
    endif()

    if(NOT PLUMED_DIR OR NOT EXISTS "${PLUMED_DIR}/bin")
        message(FATAL_ERROR "PLUMED not found. Pass -DPLUMED_DIR=/path/to/plumed (must contain bin/)")
    endif()

    if(FFTW_MODULE STREQUAL "")
        set(FFTW_MODULE "${DETECTED_FFTW}")
    endif()

    if(FFTW_MODULE STREQUAL "")
        message(FATAL_ERROR "No FFTW module found. Either `module load` one before running cmake, or pass -DFFTW_MODULE=fftw/x.y.z")
    endif()

    message(STATUS "GROMACS_DIR: ${GROMACS_DIR}")
    message(STATUS "PLUMED_DIR: ${PLUMED_DIR}")
    message(STATUS "FFTW module: ${FFTW_MODULE}")
endif()

configure_file(${CMAKE_SOURCE_DIR}/BuildConfig.hpp.in ${CMAKE_BINARY_DIR}/BuildConfig.hpp @ONLY)

add_executable(AdSamp AdSamp.cpp orderparameterpicker.cpp)

target_include_directories(AdSamp PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
target_link_libraries(AdSamp PRIVATE MPI::MPI_CXX)

if(MD_ENGINE STREQUAL "LAMMPS")
	target_compile_definitions(AdSamp PRIVATE USE_LAMMPS)
	target_include_directories(AdSamp PRIVATE ${LAMMPS_INCLUDE_DIR})
	target_link_libraries(AdSamp PRIVATE ${LAMMPS_LIBRARY} ${PLUMED_LIBRARY} ${PLUMED_KERNEL_LIBRARY})

	get_filename_component(LAMMPS_LIB_DIR ${LAMMPS_LIBRARY} DIRECTORY)
	get_filename_component(PLUMED_LIB_DIR ${PLUMED_LIBRARY} DIRECTORY)

	set_target_properties(AdSamp PROPERTIES
		BUILD_RPATH "${LAMMPS_LIB_DIR};${PLUMED_LIB_DIR}"
		INSTALL_RPATH "${LAMMPS_LIB_DIR};${PLUMED_LIB_DIR}")
endif()