# Check that a schedule with a reversal is generated
# when --schedule-nonneg-var-coefficient is not set.
domain: [N] -> { A[i] : -N < i < N; B[i] : -N < i < N }
validity: { A[i] -> A[i + 1] }
proximity: { A[i] -> B[-i] }
