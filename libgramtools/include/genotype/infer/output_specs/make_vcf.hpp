#ifndef MAKE_VCF_HPP
#define MAKE_VCF_HPP

#include <htslib/vcf.h>
#include <htslib/synced_bcf_reader.h>

#include "genotype/infer/interfaces.hpp"
#include "genotype/parameters.hpp"

using namespace gram::genotype;
using namespace gram::genotype::infer;

namespace gram::genotype{class SegmentTracker;}

void write_vcf(gram::GenotypeParams const &params, gtyper_ptr const &gtyper, SegmentTracker &tracker);
void populate_vcf_hdr(bcf_hdr_t *hdr, gtyper_ptr gtyper, gram::GenotypeParams const &params, SegmentTracker &tracker);

void
write_sites(htsFile *fout, bcf_srs_t *reader, bcf_hdr_t *header, gtyper_ptr const &gtyper, SegmentTracker &tracker);
void populate_vcf_site(bcf_hdr_t *header, bcf1_t *record, gt_site_ptr site, SegmentTracker &tracker);

#endif //MAKE_VCF_HPP
