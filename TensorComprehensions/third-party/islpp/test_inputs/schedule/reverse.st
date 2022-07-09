domain: "[N] -> { B[i] : -N < i < N; A[i] : -N < i < N }"
child:
  schedule: "[N] -> [{ B[i] -> [(-i)]; A[i] -> [(i)] }]"
  permutable: 1
  coincident: [ 1 ]
  child:
    sequence:
    - filter: "[N] -> { A[i] }"
    - filter: "[N] -> { B[i] }"
