/*
 * Copyright 2009      Katholieke Universiteit Leuven
 * Copyright 2016      Sven Verdoolaege
 *
 * Use of this software is governed by the MIT license
 *
 * Written by Sven Verdoolaege,
 * K.U.Leuven, Departement Computerwetenschappen, Celestijnenlaan 200A,
 * B-3001 Leuven, Belgium
 */

/* An isl_system is a system of constraints.
 * For now, use an isl_basic_map as the underlying representation,
 * but ignore the space and the local variables.
 */
#define isl_system	isl_basic_map

#include <isl_ctx_private.h>
#include <isl_int.h>
#include <isl_seq.h>
#include <isl_vec_private.h>
#include <isl_map_private.h>
#include <isl_system_private.h>

/* Return the dimension of "sys".
 *
 * The underlying basic map is expected to only have set dimensions.
 */
unsigned isl_system_dim(__isl_keep isl_system *sys)
{
	return isl_basic_map_total_dim(sys);
}

/* Return the isl_ctx to which "sys" belongs.
 */
isl_ctx *isl_system_get_ctx(__isl_keep isl_system *sys)
{
	return isl_basic_map_get_ctx(sys);
}

/* Free "sys" and return NULL.
 */
__isl_null isl_system *isl_system_free(__isl_take isl_system *sys)
{
	return isl_basic_map_free(sys);
}

/* Create an isl_system of dimension "n_var", with room for "extra"
 * extra variables, "n_eq" equality constraints and
 * "n_ineq" inequality constraints.
 */
__isl_give isl_system *isl_system_alloc(isl_ctx *ctx,
	unsigned n_var, unsigned extra, unsigned n_eq, unsigned n_ineq)
{
	isl_space *space;

	space = isl_space_set_alloc(ctx, 0, n_var);
	return isl_basic_map_alloc_space(space, extra, n_ineq, n_ineq);
}

/* Return a pointer to a new inequality constraint in "sys",
 * which is assumed to have enough room for this extra constraint.
 * Return NULL if an error occurs.
 *
 * After filling up the constraint, the caller should call
 * isl_system_finish_inequality.
 */
isl_int *isl_system_alloc_inequality(__isl_keep isl_system *sys)
{
	int k;

	k = isl_basic_map_alloc_inequality(sys);
	if (k < 0)
		return NULL;
	return sys->ineq[k];
}

/* Finish adding the inequality constraint "ineq" to "sys".
 *
 * This currently does nothing, but in future, it could normalize
 * the constraint, perform fangcheng (Gaussian elimination) and/or
 * check whether any sample value is still valid.
 */
__isl_give isl_system *isl_system_finish_inequality(__isl_take isl_system *sys,
	isl_int *ineq)
{
	return sys;
}

/* Return an isl_system that is equal to "sys" and that has only
 * a single reference.
 */
__isl_give isl_system *isl_system_cow(__isl_take isl_system *sys)
{
	return isl_basic_map_cow(sys);
}

/* Return an isl_system that is equal to "sys" and that has room
 * for at least "n_eq" more equality constraints and
 * "n_ineq" more inequality constraints.
 */
__isl_give isl_system *isl_system_extend_constraints(
	__isl_take isl_system *sys, unsigned n_eq, unsigned n_ineq)
{
	return isl_basic_map_extend_constraints(sys, n_eq, n_ineq);
}

/* Check that there are "n" dimensions starting at "first" in "sys".
 */
static isl_stat isl_system_check_range(__isl_keep isl_system *sys,
	unsigned first, unsigned n)
{
	unsigned dim;

	if (!sys)
		return isl_stat_error;
	dim = isl_system_dim(sys);
	if (first + n > dim || first + n < first)
		isl_die(isl_system_get_ctx(sys), isl_error_invalid,
			"position or range out of bounds",
			return isl_stat_error);
	return isl_stat_ok;
}

#undef SUFFIX
#define SUFFIX		_si
#undef INT
#define INT		int
#undef INT_SET
#define INT_SET(r,i)	isl_int_set_si(r,i)
#undef INT_NEG
#define INT_NEG(r,i)	isl_int_set_si(r,-(i))

#include "isl_system_bound_templ.c"

#undef SUFFIX
#define SUFFIX
#undef INT
#define INT		isl_int
#undef INT_SET
#define INT_SET(r,i)	isl_int_set(r,i)
#undef INT_NEG
#define INT_NEG(r,i)	isl_int_neg(r,i)

#include "isl_system_bound_templ.c"

/* Create an isl_system of dimension "n_var", where the "n" variables
 * starting at position "pos" are non-negative.
 */
__isl_give isl_system *isl_system_nonneg(isl_ctx *ctx, unsigned n_var,
	unsigned pos, unsigned n)
{
	int i;
	isl_system *sys;

	sys = isl_system_alloc(ctx, n_var, 0, 0, n);
	for (i = 0; i < n; ++i)
		sys = isl_system_lower_bound_si(sys, pos + i, 0);
	return sys;
}
