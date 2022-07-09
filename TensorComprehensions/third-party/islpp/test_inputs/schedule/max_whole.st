domain: "[N] -> { B[i] : 0 <= i < 2N; A[i] : 0 <= i < 4N; C[i] : 0 <= i < N }"
child:
  sequence:
  - filter: "[N] -> { C[i] }"
    child:
      schedule: "[N] -> [{ C[i] -> [(i)] }]"
      permutable: 1
      coincident: [ 1 ]
  - filter: "[N] -> { A[i] }"
    child:
      schedule: "[N] -> [{ A[i] -> [(i)] }]"
      permutable: 1
      coincident: [ 1 ]
  - filter: "[N] -> { B[i] }"
    child:
      schedule: "[N] -> [{ B[i] -> [(i)] }]"
      permutable: 1
      coincident: [ 1 ]
