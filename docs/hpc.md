# HPC Usage

AdSamp is designed to run on high performance computing clusters, including on scavenge partitions. Once a job is submitted it manages itself — requeueing after interruptions or timeouts, monitoring progress, and notifying the user on completion without further input.

## Scavenge Partitions

Scavenge partitions are available on most HPC clusters and allow access to cores owned by other users when those cores are idle. When the owning user reclaims their cores, any jobs running on them are automatically cancelled. AdSamp handles this with the following steps:

1. The simulation saves a checkpoint at the point of interruption
2. The job requeues itself on the cluster
3. The run continues from the checkpoint with no data loss

This allows simulations to be distributed across a large number of cores at all times without requiring a dedicated allocation.

## Instantaneous Save

Demonstrated by the previous section, this software relies on accurate checkpointing during interruption.  LAMMPS simulations may be used as pointer objects and so checkpointing can be done internally. GROMACS, on the other hand cannot be interracted with directly in the script, it may only exchange system calls. As such, to gurrantee accurate saving, the internal GROMACS code was modified to save a checkpoint automatically upon interruption, for more information see [Input Files](/docs/MDengines.md). 


## Job Structure

AdSamp runs are composed of two script types that must be submitted together. Only `AdSampControl.sh` should be submitted manually — it handles submission of `AdSamp.sh` internally.

| Script | Role |
|---|---|
| `AdSampControl.sh` | Controller — monitors all running trajectories, updates `Runinfo.txt`, and manages job lifecycle |
| `AdSamp.sh` | Worker — runs the actual MD simulations, one subdirectory per batch array task |

Both scripts have requeue and interruption handling enabled and will continue running automatically until the simulation completes.

> **Submit only `AdSampControl.sh`.** Do not submit `AdSamp.sh` directly.


## Controller Behavior

The controller is smart about its own lifecycle:

- If a new controller instance starts while one is already running, the existing controller is cancelled and the new one takes over monitoring the current runs
- If all worker jobs finish, the controller detects this and shuts itself down
- Email notifications are sent when the job terminates


## Live Monitoring

Progress is written continuously to `Runinfo.txt` in the working directory. This file is updated by the controller based on the contents of each `path.txt` file across all subdirectories. The fields written depend on the simulation type:

| Simulation type | Runinfo.txt contents |
|---|---|
| Exploration | Complete or incomplete status per trajectory |
| Basin | Number of crossings recorded per trajectory |
| Probabilistic | Success, failure, or incomplete per trajectory |


## Continuous Monitoring with tmux

Because the controller requeues itself, there is a window between requeues during which it is not running and `Runinfo.txt` will not update. For most use cases this is acceptable. If continuous live monitoring is required, copy the run commands from `AdSampControl.sh` into a tmux session instead of submitting it to the scheduler:

```plaintext
tmux new -s adsamp
# paste controller run commands here
```

The tmux session will persist across disconnections and keep the controller running without interruption.


## Postprocessing Scripts
Scavenge compatibility is provided for exploration, basin, and probabilistic steps (for more information see [Algorithms](/docs/algorithms.md)), however, postprocessing scripts are not interruption compatible, it is therefore not recommended to use the scavenge partition for these jobs. 

