## Recommended Workflow

1. Run `./ManuelsAdvancedSampling` with no arguments to enter the login flow and generate job scripts
2. Submit `AdSampControl.sh` to the scheduler
3. Monitor progress via `Runinfo.txt` or wait for the email notification
4. Once complete, run `PostProcessing.sh` before proceeding to the next simulation step
5. Re-run `./ManuelsAdvancedSampling` to generate scripts for the next step and repeat
