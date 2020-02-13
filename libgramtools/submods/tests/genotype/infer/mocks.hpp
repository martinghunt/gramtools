#include "genotype/infer/interfaces.hpp"
#include "genotype/infer/level_genotyping/probabilities.hpp"
#include "gmock/gmock.h"

using namespace gram;
using namespace gram::genotype::infer;

class MockGenotypedSite : public GenotypedSite{
public:
    MOCK_METHOD(GenotypeOrNull const, get_genotype, (), (const, override));
    MOCK_METHOD(allele_vector const, get_alleles, (), (const, override));
    MOCK_METHOD(covG_ptr const, get_site_end_node, (), (const, override));
    MOCK_METHOD(bool, is_null, (), (const, override));
    MOCK_METHOD(void, make_null, (), (override));
    MOCK_METHOD(void, add_model_specific_JSON, (JSON& input_json), (override));
};

namespace gram::genotype::infer::probabilities{
    class MockPmf : public AbstractPmf{
    public:
        MOCK_METHOD(double, compute_prob, (params const& query), (const, override));
    };
}
