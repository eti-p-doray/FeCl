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
#include <functional>

#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test;

#include "operations.h"

void test_turbo_soDecode_systOut(const fec::Turbo& code, size_t n = 1)
{
  double snr = -5.0;
  
  std::vector<fec::BitField<size_t>> msg(code.msgSize()*n, 1);
  std::vector<fec::BitField<size_t>> parity;
  code.encode(msg, parity);
  
  std::vector<double> parityIn = distort(parity, snr);
  
  std::vector<double> parityOut;
  std::vector<double> systOut;
  code.soDecode(fec::Codec::Input::parity(parityIn), fec::Codec::Output::parity(parityOut).syst(systOut));
  
  for (size_t i = 0; i < systOut.size(); ++i) {
    BOOST_REQUIRE(parityOut[i] == systOut[i]);
  }
}

test_suite* test_turbo(const fec::Turbo::EncoderOptions& encoder, const fec::Turbo::DecoderOptions& decoder, const fec::Turbo::PunctureOptions& puncture, double snr, const std::string& name)
{
  test_suite* ts = BOOST_TEST_SUITE(name);
  
  auto structure = fec::detail::Turbo::Structure(encoder, decoder);
  auto codec = fec::Turbo(encoder, decoder);
  
  //ts->add( BOOST_TEST_CASE(std::bind(&test_encodeBlock, structure )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_encode, codec, 1 )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_encode, codec, 5 )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_encode_badMsgSize, codec )));
  
  ts->add( BOOST_TEST_CASE(std::bind( &test_decode, codec, snr, 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_decode, codec, snr, 5) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_decode_badParitySize, codec )));
  
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode, codec, -5.0, 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_parityOut, codec, snr, 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_0stateIn, codec, 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_0systIn, codec, 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_systIn, codec, -5.0, 1) ));
  
  auto decoder2 = decoder;
  decoder2.iterations(2*decoder.iterations());
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_2phases, codec, fec::Turbo(encoder, decoder2), 1)));
  ts->add( BOOST_TEST_CASE(std::bind( &test_turbo_soDecode_systOut, codec, 1)));
  
  ts->add( BOOST_TEST_CASE(std::bind(&test_soDecode_badParitySize, codec )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_soDecode_badSystSize, codec )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_soDecode_badStateSize, codec )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_soDecode_noParity, codec )));
  
  ts->add( BOOST_TEST_CASE(std::bind(&test_saveLoad, codec )));
  
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
  termination(fec::Trellis::Truncate);
  auto decoder = fec::Turbo::DecoderOptions().algorithm(fec::Exact).iterations(4).scheduling(fec::Parallel);
  auto puncture = fec::Turbo::PunctureOptions().mask({{1}, {1, 0}, {1, 0}});
  
  framework::master_test_suite().add(test_turbo(encoder, decoder, puncture, -2.0, "default"));
  
  encoder.termination(fec::Trellis::Tail);
  framework::master_test_suite().add(test_turbo(encoder, decoder, puncture, -2.0, "tail"));
  
  decoder.algorithm(fec::Linear);
  framework::master_test_suite().add(test_turbo(encoder, decoder, puncture, -2.0, "table"));
  
  decoder.algorithm(fec::Approximate);
  framework::master_test_suite().add(test_turbo(encoder, decoder, puncture, -2.0, "approximate"));
  
  decoder.scheduling(fec::Serial);
  framework::master_test_suite().add(test_turbo(encoder, decoder, puncture, -2.0, "serial"));
  
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
  termination(fec::Trellis::Truncate);
  framework::master_test_suite().add(test_turbo(encoder, decoder, {}, -2.0, "3 constituents"));
  
  encoder.termination(fec::Trellis::Tail);
  decoder.algorithm(fec::Exact);
  framework::master_test_suite().add(test_turbo(encoder, decoder, {}, -2.0, "3 constituents + tail"));
  
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
  
  decoder.scheduling(fec::Serial);
  encoder.interleaver({permIndex, permIndex2, permIndex3});
  framework::master_test_suite().add(test_turbo(encoder, decoder, {}, 0.0, "3 var length constituents + Serial"));
  
  decoder.scheduling(fec::Parallel);
  framework::master_test_suite().add(test_turbo(encoder, decoder, {}, 0.0, "3 var length constituents + Parallel"));
  return 0;
}