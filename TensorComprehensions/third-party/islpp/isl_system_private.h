#ifndef ISL_SYSTEM_PRIVATE_H
#define ISL_SYSTEM_PRIVATE_H

#include <isl_system.h>

__isl_give isl_system *isl_system_alloc(isl_ctx *ctx,
	unsigned n_var, unsigned extra, unsigned n_eq, unsigned n_ineq);

#endif
