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

boost::property_tree::ptree fecEncode(const fec::Codec& code, const std::vector<std::vector<fec::BitField<size_t>>>& msg)
{
  std::vector<double> elapsedTimes(msg.size());
  for (size_t i = 0; i < msg.size(); ++i) {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    std::vector<fec::BitField<size_t>> parity;
    code.encode(msg[i], parity);
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1);
    elapsedTimes[i] = time_span.count();
  }
  boost::property_tree::ptree et;
  double sum = std::accumulate(elapsedTimes.begin(), elapsedTimes.end(), 0.0);
  double avg = sum / elapsedTimes.size();
  et.put("avg", avg);
  
  double sq_sum = std::inner_product(elapsedTimes.begin(), elapsedTimes.end(), elapsedTimes.begin(), 0.0);
  et.put("intvl", std::sqrt(sq_sum / elapsedTimes.size() - avg * avg) * z / sqrt(msg.size()));
  return et;
}

boost::property_tree::ptree fecDecode(const fec::Codec& code, const std::vector<std::vector<fec::BitField<size_t>>>& msg, const std::vector<std::vector<fec::LlrType>>& llr)
{
  std::vector<double> elapsedTimes(llr.size());
  std::vector<double> errorCount(llr.size(), 0);
  std::vector<double> blocErrorCount(llr.size(), 0);
  for (size_t i = 0; i < llr.size(); ++i) {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    std::vector<fec::BitField<size_t>> decodedMsg;
    code.decode(llr[i], decodedMsg);
    for (size_t j = 0; j < msg[i].size()/code.msgSize(); ++j) {
      bool error = false;
      for (size_t k = 0; k < code.msgSize(); ++k) {
        errorCount[i] += msg[i][j*code.msgSize()+k] != decodedMsg[j*code.msgSize()+k];
        error |= msg[i][j*code.msgSize()+k] != decodedMsg[j*code.msgSize()+k];
      }
      blocErrorCount[i] += error;
    }
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1);
    elapsedTimes[i] = time_span.count();
  }
  boost::property_tree::ptree results;
  double sum = std::accumulate(elapsedTimes.begin(), elapsedTimes.end(), 0.0);
  double avg = sum / elapsedTimes.size();
  results.put("avg", avg);
  
  double sq_sum = std::inner_product(elapsedTimes.begin(), elapsedTimes.end(), elapsedTimes.begin(), 0.0);
  results.put("intvl", std::sqrt(sq_sum / elapsedTimes.size() - avg * avg) * z / sqrt(llr.size()));
  
  for (size_t i = 0; i < errorCount.size(); ++i) {
    errorCount[i] /= msg[0].size();
    blocErrorCount[i] /= msg[0].size()/code.msgSize();
  }
  boost::property_tree::ptree ber; for(auto it = errorCount.begin(); it != errorCount.end(); ++it) {boost::property_tree::ptree el; el.put_value(*it); ber.push_back(std::make_pair("", el));}
  results.put_child("ber", ber);
  boost::property_tree::ptree wer; for(auto it = blocErrorCount.begin(); it != blocErrorCount.end(); ++it) {boost::property_tree::ptree el; el.put_value(*it); wer.push_back(std::make_pair("", el));}
  results.put_child("wer", wer);
  
  return results;
}

template <class ItppCodec>
boost::property_tree::ptree itppEncode(ItppCodec& code, const std::vector<std::vector<itpp::bvec>>& msg)
{
  std::vector<double> elapsedTimes(msg.size());
  for (size_t i = 0; i < msg.size(); ++i) {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    itpp::bvec parity;
    for (size_t j = 0; j < msg[i].size(); ++j) {
      code.encode(msg[i][j], parity);
    }
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1);
    elapsedTimes[i] = time_span.count();
  }
  boost::property_tree::ptree results;
  double sum = std::accumulate(elapsedTimes.begin(), elapsedTimes.end(), 0.0);
  double avg = sum / elapsedTimes.size();
  results.put("avg", avg);
  
  double sq_sum = std::inner_product(elapsedTimes.begin(), elapsedTimes.end(), elapsedTimes.begin(), 0.0);
  results.put("intvl", std::sqrt(sq_sum / elapsedTimes.size() - avg * avg) * z / sqrt(msg.size()));
  
  return results;
}

template <class ItppCodec>
boost::property_tree::ptree itppDecode(ItppCodec& code, const std::vector<std::vector<itpp::bvec>>& msg, const std::vector<std::vector<itpp::vec>>& llr)
{
  std::vector<double> elapsedTimes(llr.size());
  std::vector<double> errorCount(llr.size(), 0);
  std::vector<double> blocErrorCount(llr.size(), 0);
  for (size_t i = 0; i < llr.size(); ++i) {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    itpp::bvec decodedMsg;
    for (size_t j = 0; j < llr[i].size(); ++j) {
      code.decode(llr[i][j], decodedMsg);
      bool error = false;
      for (size_t k = 0; k < msg[i][j].size(); ++k) {
        errorCount[i] += msg[i][j][k] != decodedMsg[k];
        error |= msg[i][j][k] != decodedMsg[k];
      }
      blocErrorCount[i] += error;
    }
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1);
    elapsedTimes[i] = time_span.count();
  }
  boost::property_tree::ptree results;
  double sum = std::accumulate(elapsedTimes.begin(), elapsedTimes.end(), 0.0);
  double avg = sum / elapsedTimes.size();
  results.put("avg", avg);
  
  double sq_sum = std::inner_product(elapsedTimes.begin(), elapsedTimes.end(), elapsedTimes.begin(), 0.0);
  results.put("intvl", std::sqrt(sq_sum / elapsedTimes.size() - avg * avg) * z / sqrt(llr.size()));
  
  for (size_t i = 0; i < errorCount.size(); ++i) {
    errorCount[i] /= msg[0].size()*msg[0][0].size();
    blocErrorCount[i] /= msg[0].size();
  }
  boost::property_tree::ptree ber; for(auto it = errorCount.begin(); it != errorCount.end(); ++it) {boost::property_tree::ptree el; el.put_value(*it); ber.push_back(std::make_pair("", el));}
  results.put_child("ber", ber);
  boost::property_tree::ptree wer; for(auto it = blocErrorCount.begin(); it != blocErrorCount.end(); ++it) {boost::property_tree::ptree el; el.put_value(*it); wer.push_back(std::make_pair("", el));}
  results.put_child("wer", wer);

  return results;
}