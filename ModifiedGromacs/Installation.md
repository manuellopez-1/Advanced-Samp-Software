### About This File
GROMACS doesn't have a built-in way to save a checkpoint file upon termination. For AdSamp to work, GROMACS must be modified to do so. This file is the MD software for GROMACS 2022.5, modified such that a checkpoint file is saved whenever an interruption occurs. The only difference is the insertion of the following two lines after the MD loop:

```cpp
const EkindataState ekindataState2 = bGStat ? (bSumEkinhOld ? EkindataState::UsedNeedToReduce
                                                             : EkindataState::UsedDoNotNeedToReduce)
                                             : EkindataState::NotUsed; // this must be outside the master loop
do_md_trajectory_writing(fplog,
                         cr,
                         nfile,
                         fnm,
                         step,
                         step_rel,
                         t,
                         ir,
                         state,
                         state_global,
                         observablesHistory,
                         top_global,
                         fr,
                         outf,
                         energyOutput,
                         ekind,
                         f.view().force(),
                         true,
                         bRerunMD,
                         bLastStep,
                         mdrunOptions.writeConfout,
                         ekindataState2); // last writing
``` 

### Installation
To install this version of GROMACS, download GROMACS 2022.5, swap its `/src/gromacs/mdrun/md.cpp` file for [md.cpp](/ModifiedGromacs/md.cpp), and then proceed with the installation.


