#include "sdsl/suffix_arrays.hpp"
#include "sdsl/wavelet_trees.hpp"
#include <cassert>
#include "bwt_search.h"

uint32_t bidir_search(csa_wt<wt_int<rrr_vector<63>>> csa, uint32_t& l, uint32_t& r, uint32_t& l_rev, uint32_t& r_rev, uint32_t c)
{
  assert(l < r); assert(r <= csa.size());
  uint32_t c_begin = csa.C[csa.char2comp[c]];
  auto r_s_b =  csa.wavelet_tree.lex_count(l, r, c);
  uint32_t rank_l = std::get<0>(r_s_b);
  uint32_t s = std::get<1>(r_s_b) 
  uint32_t b = std::get<2>(r_s_b);
  uint32_t rank_r = r - l - s - b + rank_l;
  l = c_begin + rank_l;
  r = c_begin + rank_r+1;
  assert(r >=l);
  l_rev = l_rev + s;
  r_rev = r_rev - b+1;
  assert(r_rev-l_rev == r-l);
  return r-l;
}