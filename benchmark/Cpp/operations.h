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

#include <vector>
#include <random>
#include <memory>
#include <cstdint>
#include <iostream>

#include <boost/property_tree/ptree.hpp>

#include <itpp/comm/turbo.h>
#include <itpp/base/vec.h>
#include <itpp/base/random.h>
#include <itpp/comm/channel.h>
#include <itpp/comm/interleave.h>

#include "Turbo.h"

const double z = 1.96;

template <class V>
V randomBits(size_t n) {
  uint64_t seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::independent_bits_engine<std::mt19937,1,std::uint_fast64_t> bitGenerator((uint32_t(seed)));
  V msg(n);
  for (size_t i = 0; i < msg.size(); i++) {
    msg[i] = bitGenerator();
  }
  return msg;
}

std::vector<fec::LlrType> distort(const std::vector<fec::BitField<size_t>>& input, double snrdb)
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

itpp::vec distort(const itpp::bvec& input, double snrdb)
{
  const int8_t bpsk[2] = {-1, 1};
  
  double snr = pow(10.0, snrdb/10.0);
  
  uint64_t seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine randomGenerator;
  randomGenerator.seed(uint32_t(seed));
  std::normal_distribution<double> normalDistribution(snr*4.0, 4.0*sqrt(snr/2.0));
  
  itpp::vec llr(input.size());
  for (int i = 0; i < input.size(); i++) {
    llr[i] = bpsk[bool(input[i])] * normalDistribution(randomGenerator);
  }
  return llr;
}

boost::property_tree::ptree fecEncode(const fec::Codec& code, const std::vector<fec::BitField<size_t>>& msg, size_t M)
{
  std::vector<double> elapsedTimes(M);
  for (size_t i = 0; i < M; ++i) {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    std::vector<fec::BitField<size_t>> parity;
    code.encode(msg, parity);
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1);
    elapsedTimes[i] = time_span.count();
  }
  boost::property_tree::ptree et;
  double sum = std::accumulate(elapsedTimes.begin(), elapsedTimes.end(), 0.0);
  double avg = sum / elapsedTimes.size();
  et.put("avg", avg);
  
  double sq_sum = std::inner_product(elapsedTimes.begin(), elapsedTimes.end(), elapsedTimes.begin(), 0.0);
  et.put("intvl", std::sqrt(sq_sum / elapsedTimes.size() - avg * avg) * z / sqrt(M));
  return et;
}

boost::property_tree::ptree fecDecode(const fec::Codec& code, const std::vector<fec::LlrType>& llr, size_t M)
{
  std::vector<double> elapsedTimes(M);
  for (size_t i = 0; i < M; ++i) {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    std::vector<fec::BitField<size_t>> decodedMsg;
    code.decode(llr, decodedMsg);
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1);
    elapsedTimes[i] = time_span.count();
  }
  boost::property_tree::ptree et;
  double sum = std::accumulate(elapsedTimes.begin(), elapsedTimes.end(), 0.0);
  double avg = sum / elapsedTimes.size();
  et.put("avg", avg);
  
  double sq_sum = std::inner_product(elapsedTimes.begin(), elapsedTimes.end(), elapsedTimes.begin(), 0.0);
  et.put("intvl", std::sqrt(sq_sum / elapsedTimes.size() - avg * avg) * z / sqrt(M));
  return et;
}

template <class ItppCodec>
boost::property_tree::ptree itppEncode(ItppCodec& code, const itpp::bvec& msg, size_t M)
{
  std::vector<double> elapsedTimes(M);
  for (size_t i = 0; i < M; ++i) {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    itpp::bvec parity;
    code.encode(msg, parity);
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1);
    elapsedTimes[i] = time_span.count();
  }
  boost::property_tree::ptree et;
  double sum = std::accumulate(elapsedTimes.begin(), elapsedTimes.end(), 0.0);
  double avg = sum / elapsedTimes.size();
  et.put("avg", avg);
  
  double sq_sum = std::inner_product(elapsedTimes.begin(), elapsedTimes.end(), elapsedTimes.begin(), 0.0);
  et.put("intvl", std::sqrt(sq_sum / elapsedTimes.size() - avg * avg) * z / sqrt(M));
  return et;
}

template <class ItppCodec>
boost::property_tree::ptree itppDecode(ItppCodec& code, const itpp::vec& llr, size_t M, size_t N = 1)
{
  std::vector<double> elapsedTimes(M);
  for (size_t i = 0; i < M; ++i) {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    itpp::bvec decodedMsg;
    for (size_t j = 0; j < N; ++j) {
      code.decode(llr, decodedMsg);
    }
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1);
    elapsedTimes[i] = time_span.count();
  }
  boost::property_tree::ptree et;
  double sum = std::accumulate(elapsedTimes.begin(), elapsedTimes.end(), 0.0);
  double avg = sum / elapsedTimes.size();
  et.put("avg", avg);
  
  double sq_sum = std::inner_product(elapsedTimes.begin(), elapsedTimes.end(), elapsedTimes.begin(), 0.0);
  et.put("intvl", std::sqrt(sq_sum / elapsedTimes.size() - avg * avg) * z / sqrt(M));
  return et;
}