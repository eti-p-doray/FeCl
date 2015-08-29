/*******************************************************************************
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 Operation code example
 ******************************************************************************/

#include <vector>
#include <random>
#include <memory>
#include <cstdint>

#include "Code.h"


std::vector<uint8_t> randomBits(size_t n) {
  uint64_t seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::independent_bits_engine<std::mt19937,1,std::uint_fast64_t> bitGenerator((uint32_t(seed)));
  std::vector<uint8_t> msg(n);
  for (size_t i = 0; i < msg.size(); i++) {
    msg[i] = bitGenerator();
  }
  return msg;
}

std::vector<fec::LlrType> distort(const std::vector<uint8_t>& input, double snrdb)
{
  const int8_t bpsk[2] = {-1, 1};
  
  double snr = pow(10.0, snrdb/10.0);
  
  uint64_t seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine randomGenerator;
  randomGenerator.seed(uint32_t(seed));
  std::normal_distribution<double> normalDistribution(snr*4.0, 4.0*sqrt(snr/2.0));
  
  std::vector<fec::LlrType> llr(input.size());
  for (int i = 0; i < input.size(); i++) {
    llr[i] = bpsk[input[i]] * normalDistribution(randomGenerator);
  }
  return llr;
}

int per(const std::unique_ptr<fec::Code>& code, double snrdb)
{
  std::vector<uint8_t> msg = randomBits(code->msgSize()*1024);
  std::vector<uint8_t> parity;
  
  code->encode(msg, parity);
  
  std::vector<fec::LlrType> llr = distort(parity, snrdb);
  
  std::vector<uint8_t> msgDec;
  code->decode(llr, msgDec);
  
  int errorCount = 0;
  for (size_t i = 0; i < msg.size(); ++i) {
    errorCount += (msg[i] != msgDec[i]);
  }
  
  return errorCount;
}