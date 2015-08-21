//
//  main.cpp
//  Turbo
//
//  Created by Etienne Pierre-Doray on 2015-08-20.
//  Copyright (c) 2015 Etienne Pierre-Doray. All rights reserved.
//

#include <iostream>
#include "TurboCode/TurboCode.h"
#include "operations.h"

#include <itpp/comm/turbo.h>
#include <itpp/base/vec.h>
#include <itpp/base/random.h>
#include <itpp/comm/channel.h>
#include <itpp/comm/interleave.h>

using namespace fec;
using namespace itpp;

const size_t T = 32400;
const size_t N = 8;
const size_t M = 8;
const double z = 1.96;

int main(int argc, const char * argv[]) {
  std::vector<std::shared_ptr<fec::Code>> codes;
  std::vector<ElapsedTime> fec1EncEt;
  std::vector<ElapsedTime> fec1DecEt;
  std::vector<ElapsedTime> fec4EncEt;
  std::vector<ElapsedTime> fec4DecEt;
  std::vector<ElapsedTime> itppEncEt;
  std::vector<ElapsedTime> itppDecEt;
  
  TrellisStructure trellis({4}, {{015}}, {013});
  std::vector<size_t> systIdx(T);
  std::vector<size_t> permIdx(T);
  for (size_t i = 0; i < systIdx.size(); ++i) {
    permIdx[i] = i;
    systIdx[i] = i;
  }
  std::random_shuffle (permIdx.begin(), permIdx.end());
  codes.push_back( Code::create(TurboCodeStructure({trellis,trellis}, {systIdx, permIdx}, {fec::ConvolutionalCodeStructure::PaddingTail,fec::ConvolutionalCodeStructure::PaddingTail}, 4, TurboCodeStructure::Serial, ConvolutionalCodeStructure::LogMap), 1) );
  
  for (auto& code : codes) {
    std::vector<uint8_t> msg = randomBits<std::vector<uint8_t>>(N * T);
    fec1EncEt.push_back(fecEncode(code, msg, M));
    
    std::vector<uint8_t> parity;
    code->encode(msg, parity);
    std::vector<LlrType> llr = distort(parity, -5.0);
    fec1DecEt.push_back(fecDecode(code, llr, M));
  }
  
  /*std::vector<LlrType> codeL = distort(parity, -5.0);
  std::vector<uint8_t> msgDec;
  t1 = std::chrono::high_resolution_clock::now();
  code->decode(codeL, msgDec);
  t2 = std::chrono::high_resolution_clock::now();
  time_span = std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1);
  std::cout << time_span.count() << std::endl;*/
  
  /*{
  Turbo_Codec code;
  ivec gen(2);
  gen(0) = 013; gen(1) = 015;
  int constraint_length = 4;
  
  Sequence_Interleaver<int> inter(T);
  inter.randomize_interleaver_sequence();
  
  ivec interleaver_sequence = inter.get_interleaver_sequence();
  code.set_parameters(gen, gen, constraint_length, interleaver_sequence, 5);
  code.set_metric("LOGMAP");*/
    
  return 0;
}
