# Custom Order Parameters

The order parameter (OP) is the collective variable used by AdSamp to classify simulation snapshots — every basin check, interface crossing, and trajectory decision routes through it. The architecture is deliberately minimal: one abstract base class, one factory function, and one `.cpp`/`.hpp` pair per OP. Adding a new OP means writing those two files and registering a name in the factory. No other file needs to change.

---

## Architecture

### `OrderParameter.hpp` — the interface

```cpp
#pragma once
namespace LAMMPS_NS { class LAMMPS; }

class OrderParameter {
public:
    virtual ~OrderParameter() = default;
    virtual double update(LAMMPS_NS::LAMMPS* lmp) = 0;
};
```

Every OP inherits from this class and implements a single method:

| Method | What it must do |
|---|---|
| `double update(LAMMPS_NS::LAMMPS* lmp)` | Compute and return the current value of the order parameter from the live LAMMPS state. Called once per stride. |

### `orderparameterpicker.hpp` / `orderparameterpicker.cpp` — the factory

The factory maps a string name (passed in at runtime via the `CUSTOMOP` command-line argument) to a heap-allocated `OrderParameter` object. When AdSamp starts a run it calls:

```cpp
std::unique_ptr<OrderParameter> op = createOrderParameter(CUSTOMOP);
```

and then calls `op->update(lmp)` each stride. To register your new OP, add one `if` block to `orderparameterpicker.cpp` (see [Registering your OP](#3-register-your-op-in-the-factory) below). Nothing else in AdSamp needs to know your OP exists.

---

## Writing a Custom Order Parameter

### 1. Create the header

Create `MyOP.hpp` in the same directory as `OrderParameter.hpp`.

```cpp
#pragma once
#include "OrderParameter.hpp"

class MyOP : public OrderParameter {
public:
    double update(LAMMPS_NS::LAMMPS* lmp) override;
    // Add any private data members your computation needs here.
};
```

Rules:
- Inherit publicly from `OrderParameter`.
- Declare `update` with `override`.
- Keep any persistent state (e.g. reference positions, cached values) as private members.

### 2. Implement `update`

Create `MyOP.cpp`.

```cpp
#include "MyOP.hpp"
#include "mpi.h"
#include "lammps/atom.h"
#include "lammps/domain.h"
// include whatever else you need

double MyOP::update(LAMMPS_NS::LAMMPS* lmp) {
    // Access atom data
    auto* atom   = lmp->atom;
    auto* domain = lmp->domain;
    double** x   = atom->x;
    int  nlocal  = atom->nlocal;

    // --- your per-rank local computation ---
    double local_val = 0.0;
    // ...

    // --- reduce across MPI ranks ---
    double global_val = 0.0;
    MPI_Allreduce(&local_val, &global_val, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

    return global_val;
}
```

Key points:

- `atom->x` holds the positions of atoms **local to this MPI rank** only. Always reduce with `MPI_Allreduce` before returning.
- Use `domain->minimum_image(dx, dy, dz)` any time you compute a difference in positions across periodic boundaries.
- `atom->tag[i]` gives the global atom ID for local atom `i`. Use this to select specific atoms.
- The returned `double` is what AdSamp compares against interface/basin bounds, so make sure its units are consistent with your `leftbound` / `rightbound` arguments.

### 3. Register your OP in the factory

Open `orderparameterpicker.cpp` and add an `if` block before the `throw`:

```cpp
#include "MyOP.hpp" //add this include

std::unique_ptr<OrderParameter> createOrderParameter(const std::string& name) {
    if (name == "MyOP") //add this block
        return std::make_unique<MyOP>();

    throw std::runtime_error("Unknown CUSTOMOP '" + name +"'. Valid options: Biggest, AtomDistance, MyOP");
}
```

### 4. Add to the build

Add `MyOP.cpp` to the compile command within the `CMakeLists.txt` file, find the following line (102)/

```bash
add_executable(AdSamp AdSamp.cpp orderparameterpicker.cpp)
```
and insert your `.cpp` file, example:

```bash
add_executable(AdSamp AdSamp.cpp orderparameterpicker.cpp MyOP.cpp)
```


Then CMAKE as usual.

### Example (Atomic Distance)

`AtomDistance` measures the minimum-image distance between two specific atoms identified by their global tags (atom 1 and atom 2). It is a useful template for any OP that operates on a small fixed set of atoms.

The files are given in [AtomDistance.hpp](/tutorials/LAMMPSnoPLUMED/AtomDistance.hpp) and [AtomDistance.cpp](/tutorials/LAMMPSnoPLUMED/AtomDistance.cpp). Once copied into your working directory, make two changes:

**`orderparameterpicker.cpp`** — add one include and one `if` block:
```cpp
#include "AtomDistance.hpp"   // add at the top

// inside createOrderParameter, before the throw:
if (name == "AtomDistance"){
    return std::make_unique<AtomDistance>();
}
```

**Compile Command`** — Add the Custom OP to the `CMakeLists.txt`
find the following line (102)/

```bash
add_executable(AdSamp AdSamp.cpp orderparameterpicker.cpp)
```
and insert the new `.cpp` file, example:

```bash
add_executable(AdSamp AdSamp.cpp orderparameterpicker.cpp AtomDistance.cpp)
```


To use a different pair of atoms, change `TAG1` and `TAG2` in `AtomDistance.hpp` to the desired global atom IDs. To generalise to a group of atoms (e.g. a centre of mass), accumulate weighted sums in the local arrays and change the final reduction and return expression accordingly.

---

## Quick Reference

| Task | Where to edit |
|---|---|
| Define OP interface | `OrderParameter.hpp` — do not modify |
| Implement a new OP | New `MyOP.hpp` + `MyOP.cpp` |
| Make AdSamp aware of it | `orderparameterpicker.cpp` — add one `if` block and one `#include` |
| Build | Add `MyOP.cpp` to your build system source list |
| Run | Pass `MyOP` as the `CUSTOMOP` (10th) command-line argument |
