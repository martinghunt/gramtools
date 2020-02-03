/** @file
 * High-level tests:
 *  builds a coverage graph & gram index, maps reads to it, calls a genotyper, and tests the results!!
 */

#include "gtest/gtest.h"
#include "tests/common.hpp"
#include "genotype/quasimap/quasimap.hpp"
#include "genotype/infer/level_genotyping/runner.hpp"

/**
 * Test the following Genotyping approach: Level Genotyping (LG)
 */
TEST(LevelGenotyping, Given2SiteNonNestedPRG_CorrectGenotypes){
    std::string prg{"AATAA5C6G6AA7C8G8AA"};
    Sequences kmers{encode_dna_bases("AA")};
    prg_setup setup;
    setup.setup_numbered_prg(prg, kmers);

    allele_vector gt_alleles;
    GenomicRead_vector reads;
    // Multiple reads going through 5:1 and 7:1
    for (int i = 0; i < 5; i++) reads.push_back(GenomicRead("Read", "AATAACAACAA", "???????????"));

    // One read going through 5:2 and 7:1
    reads.push_back(GenomicRead("ErrorRead", "AATAAGAACAA", "???????????"));

    setup.quasimap_reads(reads);

    LevelGenotyper genotyper(setup.prg_info.coverage_graph, setup.coverage.grouped_allele_counts,
                             setup.read_stats, Ploidy::Haploid);
    auto gt_recs = genotyper.get_genotyped_records();

    gt_alleles = gt_recs.at(siteID_to_index(5))->get_unique_genotyped_alleles();
    allele_vector expected_alleles{ Allele{"C", {5}, 0} };
    EXPECT_EQ(gt_alleles, expected_alleles);

    gt_alleles = gt_recs.at(0)->get_unique_genotyped_alleles();
    expected_alleles = allele_vector{ Allele{"C", {5}, 0} };
    EXPECT_EQ(gt_alleles, expected_alleles);
}

TEST(LevelGenotyping, Given2SiteNestedPRG_CorrectGenotypes){
    std::string prg{"AATAA[CCC[A,G],T]AA"};
    Sequences kmers{encode_dna_bases("AA")};
    prg_setup setup;
    setup.setup_bracketed_prg(prg, kmers);

    allele_vector gt_alleles;
    GenomicRead_vector reads;
    // Multiple reads going through first allele of each site
    for (int i = 0; i < 5; i++) reads.push_back(GenomicRead("Read", "AATAACCCGAA", "???????????"));
    // One read going through second allele of site 1 and first allele of site 2
    reads.push_back(GenomicRead("ErrorRead", "AATAATAA", "????????"));

    setup.quasimap_reads(reads);

    LevelGenotyper genotyper(setup.prg_info.coverage_graph, setup.coverage.grouped_allele_counts,
                             setup.read_stats, Ploidy::Haploid);
    auto gt_recs = genotyper.get_genotyped_records();

    gt_alleles = gt_recs.at(1)->get_unique_genotyped_alleles();
    allele_vector expected_alleles{
            Allele{"G", {5}, 1}
    };
    EXPECT_EQ(gt_alleles, expected_alleles);

    gt_alleles = gt_recs.at(0)->get_unique_genotyped_alleles();
    expected_alleles = allele_vector{
            Allele{"CCCG", {5, 5, 5, 5}, 0}
    };
    EXPECT_EQ(gt_alleles, expected_alleles);
}

TEST(LevelGenotyper, GivenPRGWithDirectDeletion_CorrectlyCalledEmptyAllele){
    std::string prg{"GGGGG[CCC,]GG"};
    Sequences kmers{encode_dna_bases("GG")};
    prg_setup setup;
    setup.setup_bracketed_prg(prg, kmers);

    allele_vector gt_alleles;
    GenomicRead_vector reads;
    // Reads going through direct deletion
    for (int i = 0; i < 5; i++) reads.push_back(GenomicRead("Read", "GGGGGG", "??????"));
    setup.quasimap_reads(reads);

    LevelGenotyper genotyper(setup.prg_info.coverage_graph, setup.coverage.grouped_allele_counts,
                             setup.read_stats, Ploidy::Haploid);
    auto gt_recs = genotyper.get_genotyped_records();

    gt_alleles = gt_recs.at(0)->get_unique_genotyped_alleles();
    allele_vector expected_alleles{
            Allele{"", {}, 1}
    };
    EXPECT_EQ(gt_alleles, expected_alleles);
}

class LG_SnpsNestedInTwoHaplotypes : public ::testing::Test {
protected:
    void SetUp(){
        std::string _prg{"ATCGGC[TC[A,G]TC,GG[T,G]GG]AT"};
        auto all_kmers = gram::generate_all_kmers(2);
        Sequences all_kmers_vector{all_kmers.begin(), all_kmers.end()};
        setup.setup_bracketed_prg(_prg, all_kmers_vector);

        // This read goes through 5:1 and 7:2
        for (int num_mapped{0}; num_mapped < 7; num_mapped++) reads.push_back(
                    GenomicRead("Read1", "ATCGGCTCGTCAT", ".............") );

        // This read goes through 5:2 and 9:2
        reads.push_back( GenomicRead("Read2", "ATCGGCGGG", ".........") );
    }

    void MapReadsAndHaploidGenotype(){
        setup.quasimap_reads(reads);
        LevelGenotyper genotyper(setup.prg_info.coverage_graph, setup.coverage.grouped_allele_counts,
                                 setup.read_stats, Ploidy::Haploid);
        gt_recs = genotyper.get_genotyped_records();
    }

    GenomicRead_vector reads;
    allele_vector gt_alleles;
    allele_vector expected_alleles;
    gt_sites gt_recs;
    prg_setup setup;
};

TEST_F(LG_SnpsNestedInTwoHaplotypes, MapNoReads_AllGenotypesAreNull){
    LevelGenotyper genotyper(setup.prg_info.coverage_graph, setup.coverage.grouped_allele_counts,
                             setup.read_stats, Ploidy::Haploid);
    gt_recs = genotyper.get_genotyped_records();

    for (auto const& gt_rec : gt_recs){
        EXPECT_TRUE(gt_rec->is_null());
    }
}

TEST_F(LG_SnpsNestedInTwoHaplotypes, MapReads_CorrectlyGenotypedSites){
    MapReadsAndHaploidGenotype();

    gt_alleles = gt_recs.at(siteID_to_index(5))->get_unique_genotyped_alleles();
    expected_alleles = allele_vector{
            Allele{"TCGTC", {7, 7, 7, 7, 7}, 0}
    };
    EXPECT_EQ(gt_alleles, expected_alleles);

    gt_alleles = gt_recs.at(siteID_to_index(7))->get_unique_genotyped_alleles();
    expected_alleles = allele_vector{
            Allele{"G", {7}, 1}
    };
    EXPECT_EQ(gt_alleles, expected_alleles);
}

TEST_F(LG_SnpsNestedInTwoHaplotypes, MapReads_CorrectlyInvalidatedSites){
    // Since we called 5:1, we should invalidate whatever lives on 5:2; which is site ID 9.
    MapReadsAndHaploidGenotype();

    EXPECT_TRUE(gt_recs.at(siteID_to_index(9))->is_null());
}
