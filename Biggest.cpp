#include <algorithm>
#include <limits>
#include "mpi.h"
#include "lammps/atom.h"
#include "lammps/update.h"
#include "Biggest.hpp"

double Biggest::update(LAMMPS_NS::LAMMPS* lmp){
    auto *atom = lmp->atom;
    double **x=atom->x;
    int nlocal = atom->nlocal;
    double local = -std::numeric_limits<double>::infinity();
    for (int i = 0; i< nlocal; i++){
        local= std::max(local, x[i][2]);
    }
    double global = 0.0;
    MPI_Allreduce(&local, &global, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
    return global;
}
