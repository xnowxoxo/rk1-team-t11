#!/bin/bash
#SBATCH --job-name=rk1-t11-omp
#SBATCH --partition=standard
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32
#SBATCH --time=00:30:00
#SBATCH --output=logs/omp_%j.out

# запуск из папки репозитория: sbatch slurm/omp.sbatch

cd "$SLURM_SUBMIT_DIR"
echo "# node: $(hostname)"
echo "version;p;time_s;CHECK"

# без эталона CHECK не посчитать
[ -f ref_500000000.txt ] || ./bin/seq

export OMP_PLACES=cores
export OMP_PROC_BIND=close

for mode in atomic local; do
    export OMP_MODE=$mode
    for t in 1 2 4 8 16 32; do
        export OMP_NUM_THREADS=$t
        for r in 1 2 3; do
            ./bin/omp
        done
    done
done
