#pragma once
#include "OrderParameter.hpp"

class Biggest: public OrderParameter{
public:
    double update(LAMMPS_NS::LAMMPS* lmp) override;
};