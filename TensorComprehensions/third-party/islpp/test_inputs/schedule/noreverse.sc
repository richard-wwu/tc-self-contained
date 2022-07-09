# Check that setting --schedule-nonneg-var-coefficient prevents
# the generation of a schedule with a reversal.
# OPTIONS: --schedule-nonneg-var-coefficient
domain: [N] -> { A[i] : -N < i < N; B[i] : -N < i < N }
validity: { A[i] -> A[i + 1] }
proximity: { A[i] -> B[-i] }
