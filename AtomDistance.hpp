#pragma once
#include "OrderParameter.hpp"
 
class AtomDistance : public OrderParameter {
public:
    double update(LAMMPS_NS::LAMMPS* lmp) override;
private:
    static constexpr int TAG1 = 1;
    static constexpr int TAG2 = 2;
};
 