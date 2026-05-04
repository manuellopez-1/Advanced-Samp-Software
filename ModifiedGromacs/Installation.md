### About This File

GROMACS doesn't have a built in way to save a checkpoint file upon termination. As such, for AdSamp to work Gromacs must be modified such that it does. As such, this file is the MD software for GROMACS 2022.5 modified such that a checkpoint file is saved whenever an interruption happens. The difference is just the insertion of the following two lines following the MD loop.
"""
const EkindataState ekindataState2 = bGStat ? (bSumEkinhOld ? EkindataState::UsedNeedToReduce
                                                               : EkindataState::UsedDoNotNeedToReduce)
                                               : EkindataState::NotUsed; //this must be outside the master loop

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
                                    ekindataState2); //last writing
"""

### Installation

To install this version of GROMACS simply download Gromacs 2022.5 and swap its /src/gromacs/mdrun/md.cpp file for [md.cpp](/ModifiedGromacs/md.cpp) and then proceed with installation.

