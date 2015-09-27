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
 along with C3rel.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include <vector>
#include <random>
#include <memory>
#include <functional>

#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test;

#include "operations.h"

void test_turbo_soDecode_systOut(const fec::Turbo& code, size_t n = 1)
{
  double snr = -5.0;
  
  std::vector<fec::BitField<bool>> msg(code.msgSize()*n, 1);
  std::vector<fec::BitField<uint8_t>> parity;
  code.encode(msg, parity);
  
  std::vector<fec::LlrType> parityIn = distort(parity, snr);
  
  std::vector<fec::LlrType> parityOut;
  std::vector<fec::LlrType> systOut;
  code.soDecode(fec::Codec::Input<>().parity(parityIn), fec::Codec::Output<>().parity(parityOut).syst(systOut));
  
  if (code.structure().bitOrdering() == fec::Turbo::Group) {
    for (size_t i = 0; i < systOut.size(); ++i) {
      BOOST_REQUIRE(parityOut[i] == systOut[i]);
    }
  }
}

test_suite* test_turbo(const fec::Turbo::Structure& structure, double snr, const std::string& name)
{
  test_suite* ts = BOOST_TEST_SUITE(name);
  
  ts->add( BOOST_TEST_CASE(std::bind(&test_encodeBlock, structure )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_encode, fec::Turbo(structure,1), 1 )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_encode, fec::Turbo(structure,1), 5 )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_encode, fec::Turbo(structure,2), 5 )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_encode_badMsgSize, fec::Turbo(structure) )));
  
  ts->add( BOOST_TEST_CASE(std::bind( &test_decode, fec::Turbo(structure,1), snr, 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_decode, fec::Turbo(structure,1), snr, 5) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_decode, fec::Turbo(structure,2), snr, 5) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_decode_badParitySize, fec::Turbo(structure) )));
  
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode, fec::Turbo(structure), -5.0, 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_parityOut, fec::Turbo(structure), snr, 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_0stateIn, fec::Turbo(structure), 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_0systIn, fec::Turbo(structure), 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_systIn, fec::Turbo(structure), -5.0, 1) ));
  auto structure2 = structure;
  auto decoder = structure2.getDecoderOptions();
  decoder.iterations_ = 2*decoder.iterations_;
  structure2.setDecoderOptions(decoder);
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_2phases, fec::Turbo(structure), fec::Turbo(structure2), 1)));
  ts->add( BOOST_TEST_CASE(std::bind( &test_turbo_soDecode_systOut, fec::Turbo(structure), 1)));
  
  ts->add( BOOST_TEST_CASE(std::bind(&test_soDecode_badParitySize, fec::Turbo(structure) )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_soDecode_badSystSize, fec::Turbo(structure) )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_soDecode_badStateSize, fec::Turbo(structure) )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_soDecode_noParity, fec::Turbo(structure) )));
  
  ts->add( BOOST_TEST_CASE(std::bind(&test_saveLoad, fec::Turbo(structure) )));
  
  return ts;
}

const int n = 1024;

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
  uint64_t seed = 0;
  std::mt19937 randomGenerator((int)seed);
  std::vector<size_t> permIndex(n);
  for (size_t i = 0; i < permIndex.size(); i++) {
    permIndex[i] = i;
  }
  std::shuffle (permIndex.begin(), permIndex.end(), randomGenerator);
  
  fec::Trellis trellis({4}, {{017}}, {015});
  auto encoder = fec::Turbo::EncoderOptions(trellis, {{}, permIndex}).
    termination(fec::Convolutional::Truncate).
    bitOrdering(fec::Turbo::Alternate);
  auto decoder = fec::Turbo::DecoderOptions().algorithm(fec::Codec::Exact).iterations(4).scheduling(fec::Turbo::Parallel);
  
  auto structure = fec::Turbo::Structure(encoder, decoder);
  framework::master_test_suite().add(test_turbo(structure, -2.0, "default"));
  
  encoder.termination(fec::Convolutional::Tail);
  structure.setEncoderOptions(encoder);
  framework::master_test_suite().add(test_turbo(structure, -2.0, "tail"));
  
  decoder.algorithm(fec::Codec::Linear);
  structure.setDecoderOptions(decoder);
  framework::master_test_suite().add(test_turbo(structure, -2.0, "table"));
  
  decoder.algorithm(fec::Codec::Approximate);
  structure.setDecoderOptions(decoder);
  framework::master_test_suite().add(test_turbo(structure, -2.0, "approximate"));
  
  encoder.bitOrdering(fec::Turbo::Group);
  structure.setEncoderOptions(encoder);
  framework::master_test_suite().add(test_turbo(structure, -2.0, "group"));
  
  decoder.scheduling(fec::Turbo::Serial);
  structure.setDecoderOptions(decoder);
  framework::master_test_suite().add(test_turbo(structure, -2.0, "serial"));
  
  std::vector<size_t> permIndex2(n);
  for (size_t i = 0; i < permIndex2.size(); i++) {
    permIndex2[i] = i;
  }
  std::shuffle (permIndex2.begin(), permIndex2.end(), randomGenerator);
  std::vector<size_t> permIndex3(n);
  for (size_t i = 0; i < permIndex3.size(); i++) {
    permIndex3[i] = i;
  }
  std::shuffle (permIndex3.begin(), permIndex3.end(), randomGenerator);
  encoder = fec::Turbo::EncoderOptions({fec::Trellis({4}, {{017}}, {015}),
    fec::Trellis({3, 3}, {{05, 03, 0}, {0, 03, 07}}, {07, 05}),
    fec::Trellis({4}, {{017, 013}}, {015})}, {permIndex, permIndex2, permIndex3}).
  termination(fec::Convolutional::Truncate).
  bitOrdering(fec::Turbo::Group);
  encoder.bitOrdering(fec::Turbo::Group);
  structure.setEncoderOptions(encoder);
  framework::master_test_suite().add(test_turbo(structure, -2.0, "3 constituents"));
  
  encoder.termination(fec::Convolutional::Tail);
  decoder.algorithm(fec::Codec::Exact);
  structure.setDecoderOptions(decoder);
  structure.setEncoderOptions(encoder);
  framework::master_test_suite().add(test_turbo(structure, -2.0, "3 constituents + tail"));
  
  encoder.bitOrdering(fec::Turbo::Alternate);
  structure.setEncoderOptions(encoder);
  framework::master_test_suite().add(test_turbo(structure, -2.0, "3 constituents + alternate"));
  
  permIndex2.resize(n/2);
  for (size_t i = 0; i < permIndex2.size(); i++) {
    permIndex2[i] = i;
  }
  std::shuffle (permIndex2.begin(), permIndex2.end(), randomGenerator);
  
  permIndex3.resize(n/2);
  for (size_t i = 0; i < permIndex3.size(); i++) {
    permIndex3[i] = 2*i;
  }
  std::shuffle (permIndex3.begin(), permIndex3.end(), randomGenerator);
  
  decoder.scheduling(fec::Turbo::Serial);
  structure.setDecoderOptions(decoder);
  
  encoder.interleaver_ = {permIndex, permIndex2, permIndex3};
  structure.setEncoderOptions(encoder);
  framework::master_test_suite().add(test_turbo(structure, 0.0, "3 var length constituents + Serial"));
  
  decoder.scheduling(fec::Turbo::Parallel);
  structure.setDecoderOptions(decoder);
  framework::master_test_suite().add(test_turbo(structure, 0.0, "3 var length constituents + Parallel"));
  return 0;
}