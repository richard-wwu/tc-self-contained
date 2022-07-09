#pragma once

/*
 * CUB does not play well with NVRTC, comment system includes out aggressively and
 * patch things up here.
 */

#define CUB_NS_PREFIX
#define CUB_NS_POSTFIX

#define FLT_MIN     1.17549435E-38F
#define FLT_MAX     3.40282347E+38F
#define FLT_EPSILON 1.19209290E-07F

#define DBL_MIN    2.2250738585072014E-308
#define DBL_MAX    1.7976931348623157E+308
#define DBL_EPSILON 2.2204460492503131E-16

namespace std {

// Minimal bits of iterator_traits (from stl_iterator_base_types.h)

///  Marking input iterators.
struct input_iterator_tag { };

///  Marking output iterators.
struct output_iterator_tag { };

/// Forward iterators support a superset of input iterator operations.
struct forward_iterator_tag : public input_iterator_tag { };

/// Bidirectional iterators support a superset of forward iterator
/// operations.
struct bidirectional_iterator_tag : public forward_iterator_tag { };

/// Random-access iterators support a superset of bidirectional
/// iterator operations.
struct random_access_iterator_tag : public bidirectional_iterator_tag { };

template<typename _Iterator>
  struct iterator_traits
  {
    typedef typename _Iterator::iterator_category iterator_category;
    typedef typename _Iterator::value_type        value_type;
    typedef typename _Iterator::difference_type   difference_type;
    typedef typename _Iterator::pointer           pointer;
    typedef typename _Iterator::reference         reference;
  };

/// Partial specialization for pointer types.
template<typename _Tp>
  struct iterator_traits<_Tp*>
  {
    typedef random_access_iterator_tag iterator_category;
    typedef _Tp                         value_type;
    typedef ptrdiff_t                   difference_type;
    typedef _Tp*                        pointer;
    typedef _Tp&                        reference;
  };

/// Partial specialization for const pointer types.
template<typename _Tp>
  struct iterator_traits<const _Tp*>
  {
    typedef random_access_iterator_tag iterator_category;
    typedef _Tp                         value_type;
    typedef ptrdiff_t                   difference_type;
    typedef const _Tp*                  pointer;
    typedef const _Tp&                  reference;
  };

} // namespace std

#include "util_macro.cuh"
#include "util_arch.cuh"
#include "util_type.cuh"
#include "util_ptx.cuh"
#include "thread/thread_operators.cuh"

#include "thread/thread_load.cuh"
#include "thread/thread_store.cuh"
#include "thread/thread_scan.cuh"

#include "warp/specializations/warp_reduce_shfl.cuh"
#include "warp/specializations/warp_reduce_smem.cuh"
#include "warp/warp_reduce.cuh"

#include "warp/specializations/warp_scan_shfl.cuh"
#include "warp/specializations/warp_scan_smem.cuh"
#include "warp/warp_scan.cuh"

// #include "block/block_exchange.cuh"
#include "block/block_raking_layout.cuh"

// #include "block/specializations/block_reduce_raking.cuh"
// #include "block/specializations/block_reduce_raking_commutative_only.cuh"
#include "block/specializations/block_reduce_warp_reductions.cuh"
#include "block/block_reduce.cuh"

#include "block/specializations/block_scan_raking.cuh"
#include "block/specializations/block_scan_warp_scans.cuh"
#include "block/block_scan.cuh"
