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

#include <iostream>
#include <iomanip>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <itpp/comm/turbo.h>
#include <itpp/base/vec.h>
#include <itpp/base/random.h>
#include <itpp/comm/channel.h>
#include <itpp/comm/interleave.h>
#include <itpp/comm/ldpc.h>

#include "Turbo.h"
#include "Convolutional.h"
#include "Ldpc.h"
#include "operations.h"

using namespace fec;
using namespace itpp;
using namespace boost::property_tree;

const size_t T = 32400;
const size_t N = 256;

ptree speed_Convolutional(std::vector<double> snrdb)
{
  ptree results;
  ptree encResults;
  ptree decResults;
  
  std::vector<fec::Convolutional> codecs;
  std::vector<itpp::Convolutional_Code> itppCodecs;
  
  fec::Trellis trellis({4}, {{017, 015}}, {017});
  auto encOpt = fec::Convolutional::EncoderOptions(trellis, T).termination(fec::Convolutional::Truncate);
  codecs.push_back(fec::Convolutional(encOpt,1));
  
  itppCodecs.push_back(itpp::Convolutional_Code());
  ivec generator(2);
  generator(0)=017;
  generator(1)=015;
  generator(2)=017;
  itppCodecs[0].set_generator_polynomials(generator, 4);
  itppCodecs[0].set_method(Trunc);
  
  std::vector<std::vector<BitField<size_t>>> msg(snrdb.size());
  std::vector<std::vector<bvec>> itppMsg(snrdb.size());
  std::vector<std::vector<LlrType>> llr(snrdb.size());
  std::vector<std::vector<vec>> itppLlr(snrdb.size());
  for (int i = 0; i < snrdb.size(); ++i) {
    msg[i] = randomBits<std::vector<BitField<size_t>>>(N * T);
    itppMsg[i].resize(N);
    for (int j = 0; j < N; ++j) {
      itppMsg[i][j].set_size(T);
      for (int k = 0; k < codecs[0].msgSize(); ++k) {
        itppMsg[i][j][k] = msg[i][j*codecs[0].msgSize()+k];
      }
    }
    std::vector<BitField<size_t>> parity = codecs[0].encode(msg[i]);
    llr[i] = distort(parity, snrdb[i]);
    
    itppLlr[i].resize(N);
    for (int j = 0; j < N; ++j) {
      itppLlr[i][j].set_size(codecs[0].paritySize());
      for (int k = 0; k < codecs[0].paritySize(); ++k) {
        itppLlr[i][j][k] = -llr[i][j*codecs[0].paritySize()+k];
      }
    }
  }
  
  codecs[0].setWorkGroupSize(1);
  encResults.put_child("fecl1", fecEncode(codecs[0], msg));
  codecs[0].setWorkGroupSize(4);
  encResults.put_child("fecl4", fecEncode(codecs[0], msg));
  encResults.put_child("itpp", itppEncode(itppCodecs[0], itppMsg));
  
  ptree snr; for(auto it = snrdb.begin(); it != snrdb.end(); ++it) {ptree el; el.put_value(*it); snr.push_back(std::make_pair("", el));}
  
  codecs[0].setWorkGroupSize(1);
  decResults.put_child("fecl1", fecDecode(codecs[0], msg, llr));
  decResults.put_child("fecl1.snr", snr);
  decResults.put_child("fecl1.snr", snr);
  codecs[0].setWorkGroupSize(4);
  decResults.put_child("fecl4", fecDecode(codecs[0], msg, llr));
  decResults.put_child("fecl4.snr", snr);
  
  decResults.put_child("itpp", itppDecode(itppCodecs[0],itppMsg,itppLlr));
  decResults.put_child("itpp.snr", snr);
  
  results.put_child("encoding", encResults);
  results.put_child("decoding", decResults);
  return results;
}

ptree speed_Turbo(std::vector<double> snrdb)
{
  ptree results;
  ptree encResults;
  ptree decResults;
  
  std::vector<fec::Turbo> codecs;
  std::vector<itpp::Turbo_Codec> itppCodecs;
  
  fec::Trellis trellis({4}, {{013}}, {015});
  std::vector<size_t> permIdx(T);
  for (size_t i = 0; i < permIdx.size(); ++i) {permIdx[i] = i;}
  std::random_shuffle (permIdx.begin(), permIdx.end());
  ivec itppPermIdx(permIdx.size());
  for (size_t i = 0; i < permIdx.size(); ++i) {itppPermIdx[i] = permIdx[i];}
  
  auto encOpt = fec::Turbo::EncoderOptions(trellis, {{}, permIdx}).termination(fec::Convolutional::Tail);
  auto decOpt = fec::Turbo::DecoderOptions().algorithm(fec::Exact).iterations(4).scheduling(fec::Turbo::Serial);
  codecs.push_back(fec::Turbo(encOpt, decOpt,1));
  decOpt.algorithm(fec::Linear);
  codecs.push_back(fec::Turbo(encOpt, decOpt,1));
  decOpt.algorithm(fec::Approximate);
  codecs.push_back(fec::Turbo(encOpt, decOpt,1));
  
  auto punc = codecs[0].puncturing(fec::Turbo::PunctureOptions({}).mask({{1,1}, {1,0}, {1,0}}));
  
  itppCodecs.push_back(itpp::Turbo_Codec());
  itppCodecs.push_back(itpp::Turbo_Codec());
  itppCodecs.push_back(itpp::Turbo_Codec());
  ivec gen(2);
  gen[0] = 013;
  gen[1] = 015;
  itppCodecs[0].set_parameters(gen, gen, 4, itppPermIdx, 4, "LOGMAP");
  itppCodecs[1].set_parameters(gen, gen, 4, itppPermIdx, 4, "TABLE");
  itppCodecs[2].set_parameters(gen, gen, 4, itppPermIdx, 4, "LOGMAX");
  
  std::vector<std::vector<BitField<size_t>>> msg(snrdb.size());
  std::vector<std::vector<bvec>> itppMsg(snrdb.size());
  std::vector<std::vector<LlrType>> llr(snrdb.size());
  std::vector<std::vector<vec>> itppLlr(snrdb.size());
  for (int i = 0; i < snrdb.size(); ++i) {
    msg[i] = randomBits<std::vector<BitField<size_t>>>(N * T);
    itppMsg[i].resize(N);
    for (int j = 0; j < N; ++j) {
      itppMsg[i][j].set_size(T);
      for (int k = 0; k < codecs[0].msgSize(); ++k) {
        itppMsg[i][j][k] = msg[i][j*codecs[0].msgSize()+k];
      }
    }
    std::vector<BitField<size_t>> parity = codecs[0].encode(msg[i]);
    llr[i] = distort(parity, snrdb[i]);
    llr[i] = punc.permute(llr[i]);
    llr[i] = punc.dePermute(llr[i]);
    
    itppLlr[i].resize(N);
    for (int j = 0; j < N; ++j) {
      itppLlr[i][j].set_size(codecs[0].paritySize());
      for (int k = 0; k < codecs[0].paritySize(); ++k) {
        itppLlr[i][j][k] = -llr[i][j*codecs[0].paritySize()+k];
      }
    }
  }
  
  codecs[0].setWorkGroupSize(1);
  encResults.put_child("fecl1", fecEncode(codecs[0], msg));
  codecs[0].setWorkGroupSize(4);
  encResults.put_child("fecl4", fecEncode(codecs[0], msg));
  encResults.put_child("itpp", itppEncode(itppCodecs[0], itppMsg));
  
  boost::property_tree::ptree snr; for(auto it = snrdb.begin(); it != snrdb.end(); ++it) {ptree el; el.put_value(*it); snr.push_back(std::make_pair("", el));}
  
  std::vector<std::string> config = {"Exact", "Table", "Approximate"};
  for (int i = 0; i < codecs.size(); ++i) {
    codecs[i].setWorkGroupSize(1);
    decResults.put_child(config[i] + ".fecl1", fecDecode(codecs[i], msg, llr));
    decResults.put_child(config[i] + ".fecl1.snr", snr);
    codecs[i].setWorkGroupSize(4);
    decResults.put_child(config[i] + ".fecl4", fecDecode(codecs[i], msg, llr));
    decResults.put_child(config[i] + ".fecl4.snr", snr);
  }

  std::vector<std::string> itppConfig = {"Exact", "Table", "Approximate"};
  for (size_t i = 0; i < itppCodecs.size(); ++i) {
    decResults.put_child(itppConfig[i] + ".itpp", itppDecode(itppCodecs[i],itppMsg,itppLlr));
    decResults.put_child(config[i] + ".itpp.snr", snr);
  }
  
  results.put_child("encoding", encResults);
  results.put_child("decoding", decResults);
  return results;
}

ptree speed_Ldpc(std::vector<double> snrdb)
{
  ptree results;
  ptree encResults;
  ptree decResults;
  
  std::vector<fec::Ldpc> codecs;
  std::vector<std::unique_ptr<itpp::LDPC_Code>> itppCodecs;
  
  auto checkMatrix = fec::Ldpc::DvbS2::matrix(64800, 0.5);
  
  auto encOpt = fec::Ldpc::EncoderOptions(checkMatrix);
  auto decOpt = fec::Ldpc::DecoderOptions().algorithm(fec::Exact).iterations(20);
  codecs.push_back(fec::Ldpc(encOpt, decOpt,1));
  decOpt.algorithm(fec::Linear);
  codecs.push_back(fec::Ldpc(encOpt, decOpt,1));
  decOpt.algorithm(fec::Approximate);
  codecs.push_back(fec::Ldpc(encOpt, decOpt,1));
  
  itpp::LDPC_Parity_Irregular itppCheckMatrix;
  itppCheckMatrix.initialize(64800-T, 64800);
  auto row =  checkMatrix.begin();
  for (size_t i = 0; i < checkMatrix.rows(); ++i, ++row) {
    for (auto elem = row->begin(); elem < row->end(); ++elem) {
      itppCheckMatrix.set(i, *elem, 1);
    }
  }
  itppCodecs.push_back(std::unique_ptr<itpp::LDPC_Code>(new itpp::LDPC_Code(&itppCheckMatrix)));
  itppCodecs[0]->set_exit_conditions(20, true);
  
  std::vector<std::vector<BitField<size_t>>> msg(snrdb.size());
  std::vector<std::vector<bvec>> itppMsg(snrdb.size());
  std::vector<std::vector<LlrType>> llr(snrdb.size());
  std::vector<std::vector<vec>> itppLlr(snrdb.size());
  for (int i = 0; i < snrdb.size(); ++i) {
    msg[i] = randomBits<std::vector<BitField<size_t>>>(N * T);
    itppMsg[i].resize(N);
    for (int j = 0; j < N; ++j) {
      itppMsg[i][j].set_size(T);
      for (int k = 0; k < codecs[0].msgSize(); ++k) {
        itppMsg[i][j][k] = msg[i][j*codecs[0].msgSize()+k];
      }
    }
    std::vector<BitField<size_t>> parity = codecs[0].encode(msg[i]);
    llr[i] = distort(parity, snrdb[i]);
    
    itppLlr[i].resize(N);
    for (int j = 0; j < N; ++j) {
      itppLlr[i][j].set_size(codecs[0].paritySize());
      for (int k = 0; k < codecs[0].paritySize(); ++k) {
        itppLlr[i][j][k] = -llr[i][j*codecs[0].paritySize()+k];
      }
    }
  }
  
  codecs[0].setWorkGroupSize(1);
  encResults.put_child("fecl1", fecEncode(codecs[0], msg));
  codecs[0].setWorkGroupSize(4);
  encResults.put_child("fecl4", fecEncode(codecs[0], msg));
  
  boost::property_tree::ptree snr; for(auto it = snrdb.begin(); it != snrdb.end(); ++it) {ptree el; el.put_value(*it); snr.push_back(std::make_pair("", el));}
  
  std::vector<std::string> config = {"Exact", "Table", "Approximate"};
  for (int i = 0; i < codecs.size(); ++i) {
    codecs[i].setWorkGroupSize(1);
    decResults.put_child(config[i] + ".fecl1", fecDecode(codecs[i], msg, llr));
    decResults.put_child(config[i] + ".fecl1.snr", snr);
    codecs[i].setWorkGroupSize(4);
    decResults.put_child(config[i] + ".fecl4", fecDecode(codecs[i], msg, llr));
    decResults.put_child(config[i] + ".fecl4.snr", snr);
  }
  
  std::vector<std::string> itppConfig = {"Table"};
  for (size_t i = 0; i < itppCodecs.size(); ++i) {
    decResults.put_child(itppConfig[i] + ".itpp", itppDecode(*itppCodecs[i].get(),itppMsg, itppLlr));
    decResults.put_child(itppConfig[i] + ".itpp.snr", snr);
  }
  
  results.put_child("encoding", encResults);
  results.put_child("decoding", decResults);
  return results;
}


int main(int argc, const char * argv[]) {
  ptree allResults;
  allResults.put("msgSize", T);
  allResults.put("blocks", N);

  std::vector<double> snrdb;
  for (double i = -2.8; i <= -1.0; i += 0.1) {
    snrdb.push_back(i);
  }
  
  allResults.put_child("Convolutional", speed_Convolutional(snrdb));
  allResults.put_child("Turbo", speed_Turbo(snrdb));
  allResults.put_child("Ldpc", speed_Ldpc(snrdb));
  
  json_parser::write_json("benchmarkCppResult.json", allResults);
  std::cout << "done" << std::endl;
  
  return 0;
}
