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
 
 fec test
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
  
  std::vector<fec::BitField<bool>> msg(code.msgSize()*n, 1);
  std::vector<fec::BitField<uint8_t>> parity;
  code.encode(msg, parity);
  
  std::vector<fec::LlrType> parityIn = distort(parity, snr);
  
  std::vector<fec::LlrType> msgOut;
  std::vector<fec::LlrType> systOut;
  code.soDecode(fec::Codec::Input<>().parity(parityIn), fec::Codec::Output<>().msg(msgOut).syst(systOut));
  
  for (size_t i = 0; i < msg.size(); ++i) {
    BOOST_REQUIRE(msgOut[i] == systOut[i]);
  }
}

test_suite* test_convolutional(const fec::Convolutional::Structure& structure, double snr, const std::string& name)
{
  test_suite* ts = BOOST_TEST_SUITE(name);
  
  ts->add( BOOST_TEST_CASE(std::bind(&test_encodeBlock, structure )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_encode, fec::Convolutional(structure,1), 1 )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_encode, fec::Convolutional(structure,1), 5 )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_encode, fec::Convolutional(structure,2), 5 )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_encode_badMsgSize, fec::Convolutional(structure) )));
  
  ts->add( BOOST_TEST_CASE(std::bind( &test_decode, fec::Convolutional(structure,1), snr, 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_decode, fec::Convolutional(structure,1), snr, 5) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_decode, fec::Convolutional(structure,2), snr, 5) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_decode_badParitySize, fec::Convolutional(structure) )));
  
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode, fec::Convolutional(structure), snr, 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_parityOut, fec::Convolutional(structure), snr, 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_0stateIn, fec::Convolutional(structure), 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_0systIn, fec::Convolutional(structure), 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_systIn, fec::Convolutional(structure), -5.0, 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_2phases, fec::Convolutional(structure), fec::Convolutional(structure), 1)));
  ts->add( BOOST_TEST_CASE(std::bind( &test_convo_soDecode_systOut, fec::Convolutional(structure), 1)));
  
  ts->add( BOOST_TEST_CASE(std::bind(&test_soDecode_badParitySize, fec::Convolutional(structure) )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_soDecode_badSystSize, fec::Convolutional(structure) )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_soDecode_badStateSize, fec::Convolutional(structure) )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_soDecode_noParity, fec::Convolutional(structure) )));
  
  ts->add( BOOST_TEST_CASE(std::bind(&test_saveLoad, fec::Convolutional(structure) )));
  
  return ts;
}

test_suite* init_unit_test_suite( int argc, char* argv[] )
{
  fec::Trellis trellis({4}, {{015, 017}}, {015});
  size_t length = 1024;
  auto encoder = fec::Convolutional::EncoderOptions(trellis, length).termination(fec::Convolutional::Truncate);
  auto decoder = fec::Convolutional::DecoderOptions().algorithm(fec::Codec::Exact);
  
  auto structure = fec::Convolutional::Structure(encoder, decoder);
  framework::master_test_suite().add(test_convolutional(structure, 3.0, "default"));
  
  encoder.termination(fec::Convolutional::Tail);
  structure.setEncoderOptions(encoder);
  framework::master_test_suite().add(test_convolutional(structure, 3.0, "tail"));
  
  decoder.algorithm(fec::Codec::Linear);
  structure.setDecoderOptions(decoder);
  framework::master_test_suite().add(test_convolutional(structure, 3.0, "table"));

  decoder.algorithm(fec::Codec::Approximate);
  structure.setDecoderOptions(decoder);
  framework::master_test_suite().add(test_convolutional(structure, 3.0, "approximate"));
  
  encoder = fec::Convolutional::EncoderOptions(fec::Trellis({3, 3}, {{05, 03, 0}, {0, 03, 07}}, {07, 05}), length).termination(fec::Convolutional::Truncate);
  structure.setEncoderOptions(encoder);
  framework::master_test_suite().add(test_convolutional(structure, 6.0, "2 inputs"));
  
  encoder.termination(fec::Convolutional::Tail);
  structure.setEncoderOptions(encoder);
  framework::master_test_suite().add(test_convolutional(structure, 5.0, "2 inputs + tail"));
  
  return 0;
}