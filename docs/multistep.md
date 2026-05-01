# Running a Full FFS Simulation

A complete Forward Flux Sampling simulation is a multi-step process. Each step builds on the outputs of the previous, and postprocessing must be run between steps before the next can begin.

---

## Overview

A typical FFS simulation consists of three phases:

1. **Exploration** — Relax all input structures to a stable baseline
2. **Basin** — Record crossing events from $\lambda_A$ to $\lambda_0$ across all relaxed structures
3. **Probabilistic steps** — Repeated milestoning from the basin outputs until a full transition is achieved, each step seeding from the successes of the last

> See [Algorithms](algorithms.md) for a detailed description of how each simulation type works.

---

## Per-Step Workflow

Each phase follows the same interaction pattern:

1. Run `./ManuelsAdvancedSampling` with no arguments to enter the login flow and generate job scripts
2. Submit `AdSampControl.sh` to the scheduler
3. Monitor progress via `Runinfo.txt` or wait for the email notification
4. Once the step completes, run `./ManuelsAdvancedSampling` with no arguments to enter the login flow. Specify postprocessing and submit `PostProcessing.sh` once generated.
5. Return to step 1 for the next phase, using the postprocessed outputs as inputs

---

## Probabilistic Step Guidance

The number of probabilistic steps is not fixed — iterations continue until a full transition is reached or the per-step transition probability hits 100%. A few things to keep in mind:

- The first milestone $\lambda_1$ should be placed so that the transition probability is between 5% and 10%
- Each subsequent $\lambda_1$ must exceed all successful crossing values from the previous step
- Each probabilistic run takes as input only the successful configurations from the previous step — failures are not carried forward

---

## Full Sequence at a Glance

```plaintext
[input structures]
        │
        ▼
  1. Exploration
        │
        ▼
  PostProcessing
        │
        ▼
    2. Basin
        │
        ▼
  PostProcessing
        │
        ▼
 3. Probabilistic ($\lambda_1$)
        │
        ▼
  PostProcessing
        │
        ▼
 4. Probabilistic ($\lambda_2$)
        │
       ...
        │
        ▼
  Full transition reached
```

> Skipping postprocessing between steps will cause the subsequent run to fail. See [/docs/Outputs](outputs.md) for details on what postprocessing produces and why it is required.
