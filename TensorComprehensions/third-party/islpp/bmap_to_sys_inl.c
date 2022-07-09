#include <isl_system_type.h>
#include <isl/map_type.h>

/* Treat "bmap" as a pure system of linear constraints.
 */
static __isl_give isl_system *bmap_to_sys(__isl_take isl_basic_map *bmap)
{
	return (isl_system *) bmap;
}
