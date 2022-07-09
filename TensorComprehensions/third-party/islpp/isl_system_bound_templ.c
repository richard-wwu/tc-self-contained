/*
 * Copyright 2011      Sven Verdoolaege
 *
 * Use of this software is governed by the MIT license
 *
 * Written by Sven Verdoolaege
 */

#define xSF(TYPE,SUFFIX) TYPE ## SUFFIX
#define SF(TYPE,SUFFIX) xSF(TYPE,SUFFIX)

/* Bound the given variable of "system" from below (or above if "upper"
 * is set) to "value".
 */
__isl_give isl_system *SF(isl_system_bound,SUFFIX)(__isl_take isl_system *sys,
	unsigned pos, INT value, int upper)
{
	isl_int *ineq;

	if (isl_system_check_range(sys, pos, 1) < 0)
		return isl_system_free(sys);
	sys = isl_system_cow(sys);
	sys = isl_system_extend_constraints(sys, 0, 1);
	ineq = isl_system_alloc_inequality(sys);
	if (!ineq)
		return isl_system_free(sys);
	isl_seq_clr(ineq, 1 + isl_system_dim(sys));
	if (upper) {
		isl_int_set_si(ineq[1 + pos], -1);
		INT_SET(ineq[0], value);
	} else {
		isl_int_set_si(ineq[1 + pos], 1);
		INT_NEG(ineq[0], value);
	}
	sys = isl_system_finish_inequality(sys, ineq);
	return sys;
}

/* Bound the given variable of "system" from below to "value".
 */
__isl_give isl_system *SF(isl_system_lower_bound,SUFFIX)(
	__isl_take isl_system *sys, unsigned pos, INT value)
{
	return SF(isl_system_bound,SUFFIX)(sys, pos, value, 0);
}

/* Bound the given variable of "system" from above to "value".
 */
__isl_give isl_system *SF(isl_system_upper_bound,SUFFIX)(
	__isl_take isl_system *sys, unsigned pos, INT value)
{
	return SF(isl_system_bound,SUFFIX)(sys, pos, value, 1);
}
