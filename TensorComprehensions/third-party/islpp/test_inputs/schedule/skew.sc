# Check that a skewing transformation is found
# if the skewing preventing option is not set.
# OPTIONS: --no-schedule-unit-max-var-coefficient-sum
domain: [N] -> { S[i, j] : 0 <= i, j < N }
coincidence: { S[i, j] -> S[i + 1, j - 1] }
