/** @file
 * Procedures supporting variant aware backward searching through the prg.
 * @note `char2comp` attribute of `fm_index` gives the lexicographic ordering of the queried symbol. This allows for
 * finding symbol's first occurrence in the SA using the `C` array. For eg, we do not assume that site marker '5' is
 * the 5th element of the `C` array, because we can be given a prg which can have discontinuous integers marking variant sites.
 */

#ifndef GRAMTOOLS_SEARCH_HPP
#define GRAMTOOLS_SEARCH_HPP

#include "common/utils.hpp"
#include "quasimap/search_types.hpp"


namespace gram {

    /**
     * Updates each SearchState with the next character in the read.
     * @param pattern_char the next character in the read to look for in the prg.
     * @param SearchStates a set of SearchState elements; each contains an SA interval.
     */
    SearchStates search_base_backwards(const int_Base &pattern_char,
                                       const SearchStates &search_states,
                                       const PRG_Info &prg_info);

    /**
     * Update the current SA interval to include the next character.
     * This is a backward search. SA interval is updated using rank queries on the bwt.
     * @param next_char the next character to look for.
     * @param next_char_first_sa_index the position of the first occurrence of `next_char` in the SA.
     */
    SA_Interval base_next_sa_interval(const Marker &next_char,
                                      const SA_Index &next_char_first_sa_index,
                                      const SA_Interval &current_sa_interval,
                                      const PRG_Info &prg_info);


    std::string serialize_search_state(const SearchState &search_state);

    std::ostream &operator<<(std::ostream &os, const SearchState &search_state);

}

#endif //GRAMTOOLS_SEARCH_HPP
