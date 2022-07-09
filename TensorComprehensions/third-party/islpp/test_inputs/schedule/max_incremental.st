domain: "[N] -> { B[i] : 0 <= i < 2N; A[i] : 0 <= i < 4N; C[i] : 0 <= i < N }"
child:
  sequence:
  - filter: "[N] -> { B[i]; C[i] }"
    child:
      schedule: "[N] -> [{ B[i] -> [(i)]; C[i] -> [(2i)] }]"
      permutable: 1
      coincident: [ 1 ]
      child:
        sequence:
        - filter: "[N] -> { C[i] }"
        - filter: "[N] -> { B[i] }"
  - filter: "[N] -> { A[i] }"
    child:
      schedule: "[N] -> [{ A[i] -> [(i)] }]"
      permutable: 1
      coincident: [ 1 ]
