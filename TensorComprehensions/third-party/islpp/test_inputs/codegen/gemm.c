for (int c0 = 0; c0 < ni; c0 += 1) // coincident
  for (int c1 = 0; c1 < nj; c1 += 1) { // coincident
    S_2(c0, c1);
    for (int c2 = 0; c2 < nk; c2 += 1)
      S_4(c0, c1, c2);
  }
