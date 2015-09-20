//
//  main.cpp
//  Turbo
//
//  Created by Etienne Pierre-Doray on 2015-08-20.
//  Copyright (c) 2015 Etienne Pierre-Doray. All rights reserved.
//

#include <iostream>
#include "Turbo/Turbo.h"
#include "operations.h"

#include <itpp/comm/turbo.h>
#include <itpp/base/vec.h>
#include <itpp/base/random.h>
#include <itpp/comm/channel.h>
#include <itpp/comm/interleave.h>

using namespace fec;
using namespace itpp;

const size_t T = 32400;
const size_t N = 2;
const size_t M = 2;
const double z = 1.96;

int main(int argc, const char * argv[]) {
  std::vector<std::shared_ptr<fec::Codec>> codes;
  std::vector<ElapsedTime> fec1EncEt;
  std::vector<ElapsedTime> fec1DecEt;
  std::vector<ElapsedTime> fec4EncEt;
  std::vector<ElapsedTime> fec4DecEt;
  std::vector<ElapsedTime> itppEncEt;
  std::vector<ElapsedTime> itppDecEt;
  
  fec::Trellis trellis({4}, {{017}}, {015});
  std::vector<size_t> permIdx(T);
  for (size_t i = 0; i < permIdx.size(); ++i) {
    permIdx[i] = i;
  }
  std::random_shuffle (permIdx.begin(), permIdx.end());
  ivec itppPermIdx(permIdx.size());
  for (size_t i = 0; i < permIdx.size(); ++i) {
    itppPermIdx[i] = permIdx[i];
  }
  
  auto encoder = fec::Turbo::EncoderOptions(trellis, {{}, permIdx}).
  termination(fec::Convolutional::Tail).
  bitOrdering(fec::Turbo::Alternate);
  auto decoder = fec::Turbo::DecoderOptions().algorithm(fec::Codec::Exact).iterations(4).scheduling(fec::Turbo::Serial);
  
  auto structure = fec::Turbo::Structure(encoder, decoder);
  codes.push_back( std::shared_ptr<fec::Codec>(new fec::Turbo(structure,1)) );
  
  decoder.algorithm(fec::Codec::Approximate);
  structure = fec::Turbo::Structure(encoder, decoder);
  codes.push_back( std::shared_ptr<fec::Codec>(new fec::Turbo(structure,1)) );
  
  for (auto& code : codes) {
    std::vector<BitField<bool>> msg = randomBits<std::vector<BitField<bool>>>(N * T);
    fec1EncEt.push_back(fecEncode(*code.get(), msg, M));
    
    std::vector<BitField<uint8_t>> parity;
    code->encode(msg, parity);
    std::vector<LlrType> llr = distort(parity, -5.0);
    fec1DecEt.push_back(fecDecode(*code.get(), llr, M));
  }
  
  itpp::Turbo_Codec itppTurbo;
  ivec gen(2);
  gen[0] = 017;
  gen[1] = 015;
  itppTurbo.set_parameters(gen, gen, 4, itppPermIdx, 4, "LOGMAP");
  bvec msg = randomBits<bvec>(N * T);
  itppEncEt.push_back(itppEncode(itppTurbo, msg, M));
  
  bvec parity;
  itppTurbo.encode(msg, parity);
  vec llr = distort(parity, -5.0);
  itppDecEt.push_back(itppDecode(itppTurbo, llr, M));
  
  itppTurbo.set_parameters(gen, gen, 4, itppPermIdx, 4, "LOGMAX");
  itppDecEt.push_back(itppDecode(itppTurbo, llr, M));
  
  for (int i = 0; i < fec1DecEt.size(); ++i) {
    std::cout << fec1DecEt[i].mean << std::endl;
  }
  for (int i = 0; i < fec1DecEt.size(); ++i) {
    std::cout << itppDecEt[i].mean << std::endl;
  }
    
  return 0;
}
