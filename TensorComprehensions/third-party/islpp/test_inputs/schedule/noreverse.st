domain: "[N] -> { B[i] : -N < i < N; A[i] : -N < i < N }"
child:
  sequence:
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
