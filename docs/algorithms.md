# Algorithms

AdSamp supports three simulation types that are run in sequence to complete a full Forward Flux Sampling workflow. Each builds on the outputs of the previous.

$$T = \frac{\sum^{N}_{i=0} T_i/C_i}{\prod^{M}_{i=0} P(\lambda_{i+1}|\lambda_i)}$$

The rate is calculated from the basin flux and the product of per-step transition probabilities accumulated across all probabilistic steps.

---

## 1. Exploration

**Purpose:** Relax a system or characterize how the order parameter evolves over time without recording crossing events.

**Inputs:**
- Timestep $\Delta t$
- Order Parameter function $\lambda(x)$
- Input Files (more info in [Input Files](docs/inputs.md))
- Order Parameter Recording Stride
- Total Number of steps

**What it does:**

A molecular dynamics simulation is initialized for each structure in the input directory and run for the specified number of steps. The order parameter is recorded at the specified interval. No crossing logic is applied.

**Outputs:**
- Order parameter progression over time for each trajectory
- Final configuration of each trajectory

**Book Keeping Files**
- Overall controller file relaying number of completed simulations over time
- Individual Trajectory Completeness file
- Tracking of input seeds
- Tracking of used initial trajectory files
For more information on output files see [Output Files](docs/outputs.md)


> Freezing functional groups can be achieved by modifying the `ffs.mdp` file (see [Sample ffs.mdp](docs/samplefiles/ffs.mdp)).

> Harmonic constraints on certain groups are set by modifying `.itp` and `.top` files, (see [harmonic constraints](docs/samplefiles/harmonicconstraints/))

---

## 2. Basin

**Purpose:** Measure the flux of trajectories crossing from an accessible state $\lambda_A$ to a target milestone $\lambda_0$, recording each crossing as a starting configuration for the probabilistic step.

**Inputs:**
- A directory of starting structure files
- Order Parameter function $\lambda(x)$
- Two order parameter values: λ_A and λ₀
- Number of simulation steps
- Order Parameter Recording Stride

**What it does:**

Rather than saving all configurations and extracting crossings in postprocessing, AdSamp uses order parameter stopping criteria to record crossings on the fly. This avoids large intermediate datasets and allows much longer basin runs.

Each trajectory alternates between two run modes:

1. **Run Type 1** — The simulation runs with $\lambda_0$ as the termination criterion. When $\lambda_0$  is reached, the crossing is recorded, the current state is saved, and the trajectory switches to Run Type 2.
2. **Run Type 2 (reset)** — The simulation runs with $\lambda_A$  as the termination criterion. When $\lambda_A$  is reached, the trajectory switches back to Run Type 1.

This cycle repeats for the duration of the basin run.

![Basin simulation diagram](assets/basininfo.png)

**Outputs:**
- Saved configurations at each $\lambda_0$ crossing
- Order parameter progression over time for each trajectory


**Basin Unique Book Keeping Files**
- `BasinCrossingRecorder.txt` gives the per trajectory crossing times.
- `basininfo.txt` gives the current run type per trajectory.

**Book Keeping Files**
- Overall controller file relaying number of crossings
- Individual Trajectory Completeness file
- Tracking of input seeds
- Tracking of used initial trajectory files
For more information on output files see [Output Files](docs/outputs.mdp)

---

## 3. Probabilistic

**Purpose:** Measure the probability of advancing from λ_{n-1} to λ_n against the probability of returning to λ_A. Repeated across successive milestones until the full transition is achieved.

**Inputs:**
- Saved crossing configurations from the previous basin or probabilistic step
- λ_A (return boundary)
- λ_n (target milestone)

**What it does:**

Each saved crossing from the previous step is reinitialized with a random velocity and run until it either reaches λ_n (success) or returns to λ_A (failure). The ratio of successes to total trials gives P(λ_n | λ_{n-1}).

![Probabilistic simulation diagram](assets/probabilistic.png)

This step is repeated with increasing milestones, each time seeding from the successes of the previous round, until the full transition is reached or the transition probability hits 100%.

**Choosing milestones:**

- The first milestone λ₁ after the basin should be placed so that the transition probability is between 5% and 10%. Too high and individual steps contribute little; too low and the run becomes inefficient.
- Each subsequent λ_n must exceed all successful crossing values from the previous step. Because crossings occur across a sampling window rather than at an exact value, the previous successes will be distributed around λ_{n-1} rather than sitting exactly on it.

**Outputs:**
- Saved configurations at each λ_n crossing (inputs for the next step)
- Per-step transition probability P(λ_n | λ_{n-1})
- Full rate T when combined with basin flux via the equation above
