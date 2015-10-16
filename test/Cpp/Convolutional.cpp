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

void test_convo_soDecode_systOut(const fec::Codec& code, size_t n = 1)
{
  double snr = -5.0;
  
  std::vector<fec::BitField<size_t>> msg(code.msgSize()*n, 1);
  std::vector<fec::BitField<size_t>> parity;
  code.encode(msg, parity);
  
  std::vector<fec::LlrType> parityIn = distort(parity, snr);
  
  std::vector<fec::LlrType> msgOut;
  std::vector<fec::LlrType> systOut;
  code.soDecode(fec::Codec::Input<>().parity(parityIn), fec::Codec::Output<>().msg(msgOut).syst(systOut));
  
  for (size_t i = 0; i < msg.size(); ++i) {
    BOOST_REQUIRE(msgOut[i] == systOut[i]);
  }
}

test_suite* test_convolutional(const fec::Convolutional::EncoderOptions& encoder, const fec::Convolutional::DecoderOptions& decoder, const fec::Convolutional::PunctureOptions& puncture, double snr, const std::string& name)
{
  test_suite* ts = BOOST_TEST_SUITE(name);
  
  auto structure = fec::Convolutional::Structure(encoder, decoder);
  auto puncturedStructure = fec::PuncturedConvolutional::Structure(encoder, puncture, decoder);
  
  auto codec = fec::Convolutional(structure);
  auto puncturedCodec = fec::PuncturedConvolutional(puncturedStructure);
  auto perm = puncturedStructure.permutation();
  
  ts->add( BOOST_TEST_CASE(std::bind(&test_encodeBlock, structure )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_encode, codec, 1 )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_encode_puncture, codec, perm, puncturedCodec, 1 )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_encode, codec, 5 )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_encode_puncture, codec, perm, puncturedCodec, 5 )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_encode_badMsgSize, codec )));
  
  ts->add( BOOST_TEST_CASE(std::bind( &test_decode, codec, snr, 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_decode, codec, snr, 5) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_decode_puncture, codec, perm, puncturedCodec, -5.0, 5) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_decode_badParitySize, codec )));
  
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode, codec, snr, 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_puncture, codec, perm, puncturedCodec, -5.0, 5) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_parityOut, codec, snr, 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_0stateIn, codec, 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_0systIn, codec, 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_systIn, codec, -5.0, 1) ));
  
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_2phases, codec, codec, 1)));
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_2phases, puncturedCodec, puncturedCodec, 1)));
  ts->add( BOOST_TEST_CASE(std::bind( &test_convo_soDecode_systOut, codec, 1)));
  
  ts->add( BOOST_TEST_CASE(std::bind(&test_soDecode_badParitySize, codec )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_soDecode_badSystSize, codec )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_soDecode_badStateSize, codec )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_soDecode_noParity, codec )));
  
  ts->add( BOOST_TEST_CASE(std::bind(&test_saveLoad, fec::Convolutional(structure) )));
  
  return ts;
}

test_suite* init_unit_test_suite( int argc, char* argv[] )
{
  fec::Trellis trellis({4}, {{015, 017}}, {015});
  size_t length = 32;
  auto encoder = fec::Convolutional::EncoderOptions(trellis, length).termination(fec::Convolutional::Truncate);
  auto decoder = fec::Convolutional::DecoderOptions().algorithm(fec::Codec::Exact);
  auto puncture = fec::Convolutional::PunctureOptions().mask({1, 0, 0, 1});
  
  auto structure = fec::Convolutional::Structure(encoder, decoder);
  framework::master_test_suite().add(test_convolutional(encoder, decoder, puncture, 3.0, "default"));
  
  encoder.termination(fec::Convolutional::Tail);
  framework::master_test_suite().add(test_convolutional(encoder, decoder, puncture, 3.0, "tail"));
  
  decoder.algorithm(fec::Codec::Linear);
  framework::master_test_suite().add(test_convolutional(encoder, decoder, puncture, 3.0, "table"));

  decoder.algorithm(fec::Codec::Approximate);
  framework::master_test_suite().add(test_convolutional(encoder, decoder, puncture, 3.0, "approximate"));
  
  encoder = fec::Convolutional::EncoderOptions(fec::Trellis({3, 3}, {{05, 03, 0}, {0, 03, 07}}, {07, 05}), length).termination(fec::Convolutional::Truncate);
  framework::master_test_suite().add(test_convolutional(encoder, decoder, {}, 6.0, "2 inputs"));
  
  encoder.termination(fec::Convolutional::Tail);
  framework::master_test_suite().add(test_convolutional(encoder, decoder, {}, 5.0, "2 inputs + tail"));
  
  return 0;
}