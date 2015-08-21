//
//  main.cpp
//  Turbo
//
//  Created by Etienne Pierre-Doray on 2015-08-20.
//  Copyright (c) 2015 Etienne Pierre-Doray. All rights reserved.
//

#include <vector>
#include <random>
#include <memory>
#include <cstdint>
#include <iostream>
#include "TurboCode/TurboCode.h"

#include <itpp/comm/turbo.h>
#include <itpp/base/vec.h>
#include <itpp/base/random.h>
#include <itpp/comm/channel.h>
#include <itpp/comm/interleave.h>

struct ElapsedTime
{
  double mean;
  double std;
};

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

itpp::Vec<fec::LlrType> distort(const itpp::Vec<uint8_t>& input, double snrdb)
{
  const int8_t bpsk[2] = {-1, 1};
  
  double snr = pow(10.0, snrdb/10.0);
  
  uint64_t seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine randomGenerator;
  randomGenerator.seed(uint32_t(seed));
  std::normal_distribution<double> normalDistribution(snr*4.0, 4.0*sqrt(snr/2.0));
  
  itpp::Vec<fec::LlrType> llr(input.size());
  for (int i = 0; i < input.size(); i++) {
    llr[i] = bpsk[input[i]] * normalDistribution(randomGenerator);
  }
  return llr;
}

ElapsedTime fecEncode(const std::shared_ptr<fec::Code>& code, const std::vector<uint8_t>& msg, size_t M)
{
  std::vector<double> elapsedTimes(M);
  for (size_t i = 0; i < M; ++i) {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    std::vector<uint8_t> parity;
    code->encode(msg, parity);
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1);
    elapsedTimes[i] = time_span.count();
  }
  ElapsedTime elapsedTime;
  double sum = std::accumulate(elapsedTimes.begin(), elapsedTimes.end(), 0.0);
  elapsedTime.mean = sum / elapsedTimes.size();
  
  double sq_sum = std::inner_product(elapsedTimes.begin(), elapsedTimes.end(), elapsedTimes.begin(), 0.0);
  elapsedTime.std = std::sqrt(sq_sum / elapsedTimes.size() - elapsedTime.mean * elapsedTime.mean);
  return elapsedTime;
}

ElapsedTime fecDecode(const std::shared_ptr<fec::Code>& code, const std::vector<fec::LlrType>& llr, size_t M)
{
  std::vector<double> elapsedTimes(M);
  for (size_t i = 0; i < M; ++i) {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    std::vector<uint8_t> decodedMsg;
    code->decode(llr, decodedMsg);
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1);
    elapsedTimes[i] = time_span.count();
  }
  ElapsedTime elapsedTime;
  double sum = std::accumulate(elapsedTimes.begin(), elapsedTimes.end(), 0.0);
  elapsedTime.mean = sum / elapsedTimes.size();
  
  double sq_sum = std::inner_product(elapsedTimes.begin(), elapsedTimes.end(), elapsedTimes.begin(), 0.0);
  elapsedTime.std = std::sqrt(sq_sum / elapsedTimes.size() - elapsedTime.mean * elapsedTime.mean);
  return elapsedTime;
}

ElapsedTime itppEncode(itpp::Turbo_Codec& code, const itpp::bvec& msg, size_t M)
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
  ElapsedTime elapsedTime;
  double sum = std::accumulate(elapsedTimes.begin(), elapsedTimes.end(), 0.0);
  elapsedTime.mean = sum / elapsedTimes.size();
  
  double sq_sum = std::inner_product(elapsedTimes.begin(), elapsedTimes.end(), elapsedTimes.begin(), 0.0);
  elapsedTime.std = std::sqrt(sq_sum / elapsedTimes.size() - elapsedTime.mean * elapsedTime.mean);
  return elapsedTime;
}

ElapsedTime itppDecode(itpp::Turbo_Codec& code, const itpp::vec& llr, size_t M)
{
  std::vector<double> elapsedTimes(M);
  for (size_t i = 0; i < M; ++i) {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    itpp::bvec decodedMsg;
    code.decode(llr, decodedMsg);
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1);
    elapsedTimes[i] = time_span.count();
  }
  ElapsedTime elapsedTime;
  double sum = std::accumulate(elapsedTimes.begin(), elapsedTimes.end(), 0.0);
  elapsedTime.mean = sum / elapsedTimes.size();
  
  double sq_sum = std::inner_product(elapsedTimes.begin(), elapsedTimes.end(), elapsedTimes.begin(), 0.0);
  elapsedTime.std = std::sqrt(sq_sum / elapsedTimes.size() - elapsedTime.mean * elapsedTime.mean);
  return elapsedTime;
}