#include "common/utils.hpp"
#include "prg/masks.hpp"
#include "kmer_index/build.hpp"
#include "generate_prg.hpp"


using namespace gram;


PRG_Info generate_prg_info(const marker_vec &prg_raw) {
    Parameters parameters = {};
   parameters.encoded_prg_fpath = "encoded_prg_file_name";
    parameters.fm_index_fpath = "@fm_index";
    parameters.cov_graph_fpath = "@cov_graph";
    parameters.gram_dirpath = "@gram_dir";


    PRG_String ps{prg_raw};
    auto encoded_prg = ps.get_PRG_string();
    // Write the int vector to disk so that it can be read by sdsl for building fm index
    ps.write(parameters.encoded_prg_fpath, endianness::small);

    PRG_Info prg_info;
    prg_info.encoded_prg = encoded_prg;
    prg_info.fm_index = generate_fm_index(parameters);
    prg_info.coverage_graph = generate_cov_graph(parameters, ps);
    prg_info.last_allele_positions = ps.get_end_positions();
    prg_info.sites_mask = generate_sites_mask(encoded_prg);
    prg_info.allele_mask = generate_allele_mask(encoded_prg);

    prg_info.prg_markers_mask = generate_prg_markers_mask(encoded_prg);
    prg_info.prg_markers_rank = sdsl::rank_support_v<1>(&prg_info.prg_markers_mask);
    prg_info.prg_markers_select = sdsl::select_support_mcl<1>(&prg_info.prg_markers_mask);

    prg_info.markers_mask_count_set_bits =
            prg_info.prg_markers_rank(prg_info.prg_markers_mask.size());

    prg_info.bwt_markers_mask = generate_bwt_markers_mask(prg_info.fm_index);

    prg_info.dna_bwt_masks = generate_bwt_masks(prg_info.fm_index, parameters);
    prg_info.rank_bwt_a = sdsl::rank_support_v<1>(&prg_info.dna_bwt_masks.mask_a);
    prg_info.rank_bwt_c = sdsl::rank_support_v<1>(&prg_info.dna_bwt_masks.mask_c);
    prg_info.rank_bwt_g = sdsl::rank_support_v<1>(&prg_info.dna_bwt_masks.mask_g);
    prg_info.rank_bwt_t = sdsl::rank_support_v<1>(&prg_info.dna_bwt_masks.mask_t);

    prg_info.max_alphabet_num = get_max_alphabet_num(encoded_prg);
    return prg_info;
}
