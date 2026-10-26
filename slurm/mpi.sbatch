#!/bin/bash
#SBATCH --job-name=rk1-t11-mpi
#SBATCH --partition=standard
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=32
#SBATCH --time=00:30:00
#SBATCH --output=logs/mpi_%j.out

# запуск из папки репозитория: sbatch slurm/mpi.sbatch
# перед этим собрать на bilim-master: make

module load intel/mpi/2021.14
export I_MPI_PMI_LIBRARY=/usr/lib64/libpmi.so

cd "$SLURM_SUBMIT_DIR"
echo "# node: $(hostname); nodes: $SLURM_JOB_NODELIST"
echo "version;p;time_s;CHECK"

# seq 3 раза, первый запуск пишет эталон ref_500000000.txt
for r in 1 2 3; do
    srun --mpi=none -N 1 -n 1 ./bin/seq
done

for p in 1 2 4 8 16 32 64; do
    nodes=$(( p > 32 ? 2 : 1 ))
    for r in 1 2 3; do
        srun --mpi=none -N $nodes -n $p --cpu-bind=cores ./bin/mpi
    done
done
