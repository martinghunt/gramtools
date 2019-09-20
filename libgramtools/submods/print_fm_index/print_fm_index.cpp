/**
 * @file Print out fm index information: index, BWT, SA, full text suffixes.
 * Used in testing code in gramtools,
 * usable more generally for illustrative purposes.
 */
#include <iostream>
#include "src_common/generate_prg.hpp"

std::string DNA_convert(const int& base){
    switch(base){
        case 1: return "A";
        case 2: return "C";
        case 3: return "G";
        case 4: return "T";
        default: throw std::out_of_range("The argument is not in [1,4]");
    }
}

int main(int argc, const char* argv[]){
    std::string prg_string;
    if (argc != 2){
        std::cout << "Usage: " << argv[0] << " prg_string" << std::endl;
        exit(1);
    }
    prg_string = argv[1];
    std::transform(prg_string.begin(), prg_string.end(), prg_string.begin(), ::toupper);

    gram::PRG_Info prg_info = generate_prg_info(prg_string);

    const auto &fm_index = prg_info.fm_index;

    std::cout << "PRG: " << prg_string << std::endl;
    std::cout << "i\tBWT\tSA\ttext_suffix" << std::endl;
    for (int i = 0; i < fm_index.size(); ++i){
        std::cout << i << "\t" << DNA_convert(fm_index.bwt[i]) << "\t" << fm_index[i] << "\t";
        for (auto j = fm_index[i]; j < fm_index.size(); ++j)
            // Note: we do not use the prg_string here, because it does not encode each variant marker as its own entity.
            // TODO: find how to use prg_info.encoded_prg
            std::cout << DNA_convert(fm_index.text[j]) << " ";
        std::cout << std::endl;
    }
    }
