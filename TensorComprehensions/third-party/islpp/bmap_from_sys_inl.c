#include <isl_system_type.h>
#include <isl/map_type.h>

/* Return the basic map that was treated as a pure system of linear constraints.
 */
static __isl_give isl_basic_map *bmap_from_sys(__isl_take isl_system *sys)
{
	return (isl_basic_map *) sys;
}
