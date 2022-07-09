domain: "[N] -> { B[i] : 0 <= i < 2N; A[i] : 0 <= i < 4N; C[i] : 0 <= i < N }"
child:
  schedule: "[N] -> [{ B[i] -> [(2i)]; A[i] -> [(i)]; C[i] -> [(4i)] }]"
  permutable: 1
  coincident: [ 1 ]
  child:
    sequence:
    - filter: "[N] -> { C[i] }"
    - filter: "[N] -> { A[i] }"
    - filter: "[N] -> { B[i] }"
