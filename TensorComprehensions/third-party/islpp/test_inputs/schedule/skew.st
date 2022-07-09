domain: "[N] -> { S[i, j] : 0 <= i < N and 0 <= j < N }"
child:
  schedule: "[N] -> [{ S[i, j] -> [(i + j)] }, { S[i, j] -> [(i)] }]"
  permutable: 1
  coincident: [ 1, 0 ]
