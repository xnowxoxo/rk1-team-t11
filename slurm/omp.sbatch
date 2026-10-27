#!/bin/bash
#SBATCH --job-name=t11_omp
#SBATCH --partition=standard
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=48
#SBATCH --exclusive
#SBATCH --mem=16G
#SBATCH --nodelist=pbcn02   # pbcn01/pbcn03 have no C headers, gcc fails there
#SBATCH --time=00:40:00
#SBATCH --output=logs/omp_%j.out
# Track 11, histogram + counting sort. Submit from the repository root: sbatch slurm/omp.sbatch

mkdir -p bin
gcc -O3 src/seq_omp.c -o bin/seq_omp || exit 1
gcc -O3 -fopenmp src/omp.c -o bin/omp || exit 1
echo "# node: $(hostname)"
lscpu | grep -E '^(Model name|Socket|Core|Thread|NUMA node[0-9])' | sed 's/^/# /'
echo "# $(gcc --version | head -1)"

# spread: on this node it puts p = 2, 4, 8, 24 threads all on socket 0 (see README, «Узел»)
export OMP_PLACES=cores OMP_PROC_BIND=spread
echo "version;p;time_s;CHECK"

for r in 1 2 3; do ./bin/seq_omp; done

for v in atomic local; do
  for p in 1 2 4 8 16 32; do
    for r in 1 2 3; do
      OMP_NUM_THREADS=$p ./bin/omp $v
    done
  done
done
