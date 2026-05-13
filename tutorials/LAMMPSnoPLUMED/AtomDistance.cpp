#include <cmath>
#include <stdexcept>
#include "mpi.h"
#include "lammps/atom.h"
#include "lammps/domain.h"
#include "AtomDistance.hpp"
 
double AtomDistance::update(LAMMPS_NS::LAMMPS* lmp){
    auto* atom   = lmp->atom;
    auto* domain = lmp->domain;
    double** x=atom->x;
    int* tag= atom->tag;
    int nlocal = atom->nlocal;
    double local1[3] ={0.0, 0.0, 0.0};
    double local2[3] = {0.0, 0.0, 0.0};
 
    for (int i = 0; i < nlocal; i++) {
        if (tag[i] == TAG1) {
            local1[0]= x[i][0];
            local1[1] =x[i][1];
            local1[2]=x[i][2];
        }
        if (tag[i] == TAG2) {
            local2[0]= x[i][0];
            local2[1] =x[i][1];local2[2]=
            x[i][2];
        }
    }
    double global1[3]={0.0, 0.0, 0.0};
    double global2[3]={0.0, 0.0, 0.0};
    MPI_Allreduce(local1, global1, 3, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(local2, global2, 3, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    double dx[3] = {global1[0] - global2[0],global1[1] - global2[1],global1[2] - global2[2]};
    domain->minimum_image(dx[0], dx[1], dx[2]);
 
    return std::sqrt(dx[0]*dx[0] + dx[1]*dx[1] + dx[2]*dx[2]);
}
