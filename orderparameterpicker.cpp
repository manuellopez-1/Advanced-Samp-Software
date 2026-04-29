#include "orderparameterpicker.hpp"
#include "Biggest.hpp"
#include "AtomDistance.hpp"
#include "OrderParameter.hpp"
#include <stdexcept>

std::unique_ptr<OrderParameter> createOrderParameter(const std::string& name){
    if (name == "Biggest")
        return std::make_unique<Biggest>();

    if (name == "AtomDistance")
        return std::make_unique<AtomDistance>();

    throw std::runtime_error(
        "Unknown CUSTOMOP '" + name +
        "'. Valid options: Biggest, AtomDistance");
}