# Check that a schedule with a coefficient of 4 can be generated
# when the maximal size of the coefficients is set to 4.
# OPTIONS: --schedule-max-var-coefficient=4
domain: [N] -> { A[i] : 0 <= i < 4N; B[i] : 0 <= i < 2N; C[i] : 0 <= i < N }
proximity: { A[i] -> B[j] : i = 2j; B[i] -> C[j] : i = 2j }
