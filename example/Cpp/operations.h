/*******************************************************************************
 This file is part of FeCl.
 
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 Copyright (c) 2015, Leszek Szczecinski, INRS
 All rights reserved.
 
 FeCl is free software: you can redistribute it and/or modify
 it under the terms of the Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 FeCl is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the Lesser General Public License
 along with FeCl.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <vector>
#include <random>
#include <memory>
#include <cstdint>

#include "Codec.h"

template <int width>
std::vector<fec::BitField<size_t>> randomBits(size_t n) {
  uint64_t seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::independent_bits_engine<std::mt19937,width,std::uint_fast64_t> bitGenerator((uint32_t(seed)));
  std::vector<fec::BitField<size_t>> msg(n);
  for (size_t i = 0; i < msg.size(); i++) {
    msg[i] = bitGenerator();
  }
  return msg;
}

std::vector<double> distort(const std::vector<fec::BitField<size_t>>& input, double snrdb)
{
  const int8_t bpsk[2] = {-1, 1};
  
  double snr = pow(10.0, snrdb/10.0);
  
  uint64_t seed = 0;//std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine randomGenerator;
  randomGenerator.seed(uint32_t(seed));
  std::normal_distribution<double> normalDistribution(snr*4.0, 4.0*sqrt(snr/2.0));
  
  std::vector<double> llr(input.size());
  for (int i = 0; i < input.size(); i++) {
    llr[i] = bpsk[input[i]] * normalDistribution(randomGenerator);
  }
  return llr;
}

std::vector<double> distort(const std::vector<double>& input, double snrdb, int dim)
{
  double snr = pow(10.0, snrdb/10.0);
  
  uint64_t seed = 0;//std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine randomGenerator;
  randomGenerator.seed(uint32_t(seed));
  std::normal_distribution<double> normalDistribution(0.0, 1.0/sqrt(dim*snr));
  
  std::vector<double> llr(input.size());
  for (int i = 0; i < input.size(); i++) {
    llr[i] = input[i] + normalDistribution(randomGenerator);
  }
  return llr;
}

int per(const fec::Codec& codec, double snrdb)
{
  auto msg = randomBits<1>(codec.msgSize());
  auto parity = codec.encode(msg);
  
  auto llr = distort(parity, snrdb);
  
  auto msgDec = codec.decode(llr);
  
  std::vector<double> msgPost;
  codec.soDecode(codec.Input.parity(llr), codec.Output.msg(msgPost));
  
  
  int errorCount = 0;
  for (size_t i = 0; i < msg.size(); ++i) {
    errorCount += (msg[i] != msgDec[i]);
  }
  
  return errorCount;
}

#endif