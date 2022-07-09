domain: "[N] -> { S[i, j] : 0 <= i < N and 0 <= j < N }"
child:
  schedule: "[N] -> [{ S[i, j] -> [(i)] }, { S[i, j] -> [(j)] }]"
  permutable: 1
