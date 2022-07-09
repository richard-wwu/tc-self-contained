# Check that the generated schedule does not have coefficients
# greater than 2.
# OPTIONS: --schedule-whole-component --schedule-max-var-coefficient=2
domain: [N] -> { A[i] : 0 <= i < 4N; B[i] : 0 <= i < 2N; C[i] : 0 <= i < N }
proximity: { A[i] -> B[j] : i = 2j; B[i] -> C[j] : i = 2j }
