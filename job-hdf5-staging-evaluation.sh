#!/bin/bash -x
#SBATCH --account=icei-hbp-2022-0013
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --time=1:00:00
#SBATCH --partition=batch

# *** start of job script ***
# Note: The current working directory at this point is
# the directory where sbatch was executed.

srun ./binary/hdf5-test_hdf5_no_staging 0 50

srun ./binary/hdf5-test_hdf5_staging 0 50
#srun ./binary/hdf5-test_hdf5_staging 50 100 
#srun ./binary/hdf5-test_hdf5_staging 100 150 
#srun ./binary/hdf5-test_hdf5_staging 150 200
#srun ./binary/hdf5-test_hdf5_staging 200 250
#srun ./binary/hdf5-test_hdf5_staging 250 300
#srun ./binary/hdf5-test_hdf5_staging 300 350
#srun ./binary/hdf5-test_hdf5_staging 350 400
#srun ./binary/hdf5-test_hdf5_staging 400 450
#srun ./binary/hdf5-test_hdf5_staging 450 500
#srun ./binary/hdf5-test_hdf5_staging 500 550
#srun ./binary/hdf5-test_hdf5_staging 550 600
#srun ./binary/hdf5-test_hdf5_staging 600 650
#srun ./binary/hdf5-test_hdf5_staging 650 700
#srun ./binary/hdf5-test_hdf5_staging 700 750
#srun ./binary/hdf5-test_hdf5_staging 750 800
#srun ./binary/hdf5-test_hdf5_staging 800 850
#srun ./binary/hdf5-test_hdf5_staging 850 900
#srun ./binary/hdf5-test_hdf5_staging 900 950
#srun ./binary/hdf5-test_hdf5_staging 950 1000
#srun ./binary/hdf5-test_hdf5_staging 1000 1050
#srun ./binary/hdf5-test_hdf5_staging 1050 1100
#srun ./binary/hdf5-test_hdf5_staging 1100 1150
#srun ./binary/hdf5-test_hdf5_staging 1150 1200