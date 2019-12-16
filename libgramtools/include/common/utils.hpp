/** @file
 * Defines a set of data structures and functions common to the backend.
 */
#ifndef GRAMTOOLS_UTILS_HPP
#define GRAMTOOLS_UTILS_HPP

#include <vector>
#include <list>
#include <cstdint>
#include <string>
#include <stack>
#include <unordered_set>
#include <unordered_map>

#include <boost/functional/hash.hpp>

#include "sequence_read/seqread.hpp"


namespace gram {
    template<typename SEQUENCE>
    struct sequence_hash {
        std::size_t operator()(const SEQUENCE &seq) const {
            std::size_t hash = 0; // Used as an initial seed
            boost::hash_range(hash, seq.begin(), seq.end());
            return hash;
        }
    };

    template<typename SEQUENCE, typename T>
    using SequenceHashMap = std::unordered_map<SEQUENCE, T, sequence_hash<SEQUENCE>>;

    template<typename PAIR, typename T>
    using PairHashMap = std::unordered_map<PAIR, T, boost::hash<PAIR>>;

    template<typename T>
    using HashSet = std::unordered_set<T, boost::hash<T>>;


    using int_Base = uint8_t; /**< nucleotide represented as byte-sized integer */
    using Pattern = std::vector<int_Base>; /** A string of nucleotides is represented as a vector of `Base`s. */
    using Patterns = std::vector<Pattern>;

    using Marker = uint32_t ; /**< An integer >=5 describing a site or allele marker in the prg. */
    using AlleleId = uint32_t; /**< An integer describing which allele is referred to within a given variant site. */

    using marker_vec = std::vector<Marker>;

    using VariantLocus = std::pair<Marker, AlleleId>; /**< A Variant site/`AlleleId` combination.*/
    using parental_map = std::unordered_map<Marker, VariantLocus>; /** Map of a site to its parental Locus */

    /**
     * Unified definition of a site marker
     */
    bool is_site_marker(Marker const& variant_marker);

    /**
     * Unified definition of an allele marker
     */
    bool is_allele_marker(Marker const& variant_marker);

    /**
     * Produce the reverse complement of a `read`.
     */
    Pattern reverse_complement_read(const Pattern &read);

    Pattern encode_dna_bases(const std::string &dna_str);

    Pattern encode_dna_bases(const GenomicRead &read_sequence);

    struct EncodeResult {
        bool is_dna;
        uint32_t character;
    };

    /**
     * Encode a character read from the prg as an integer.
     * Use `EncodeResult` object to additionally store if the encoded character is DNA or not.
     * @see EncodeResult()
     */
    EncodeResult encode_char(const char &c);

    /**
     * Encode dna base as integer (range: 1-4)
     */
    int_Base encode_dna_base(const char &base_str);

    /**
     * Decode integer into dna base, as std::string.
     */
    std::string decode_dna_base(const int_Base& base);

    std::string full_path(const std::string &gram_dirpath,
                          const std::string &file_name);

    std::string ints_to_prg_string(std::vector<Marker> const& int_vec); /**< Prg string as int vector, to readable string */
    /**
     * Convert a nested PRG string to int representation, with linear site numbering.
     * CAUTION: the site numbering is based on the fixed order in which '[' chars are encountered
     * String to int conversion might have lost the initial ordering.
     */
    std::vector<Marker> prg_string_to_ints(std::string const& string_prg);

}

#endif //GRAMTOOLS_UTILS_HPP
