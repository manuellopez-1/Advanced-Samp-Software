#!/bin/bash
#SBATCH -p pi_balou
#SBATCH -n 1
#SBATCH -N 1
#SBATCH -t 7-00:00:00
#SBATCH --mail-type=BEGIN,END,FAIL   
#SBATCH --mail-user=manuel.lopez@yale.edu
#SBATCH --mem=128G
#SBATCH -a 1-19
module load TensorFlow/2.5.0-fosscuda-2020b
./mathematics ${SLURM_ARRAY_TASK_ID}