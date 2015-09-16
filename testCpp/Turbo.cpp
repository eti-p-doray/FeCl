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

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
  uint64_t seed = 0;
  std::mt19937 randomGenerator((int)seed);
  std::vector<size_t> systIndex(1024);
  std::vector<size_t> permIndex(1024);
  for (size_t i = 0; i < systIndex.size(); i++) {
    systIndex[i] = i;
    permIndex[i] = i;
  }
  std::shuffle (permIndex.begin(), permIndex.end(), randomGenerator);
  
  fec::Trellis trellis({4}, {{017}}, {015});
  auto encoder = fec::Turbo::EncoderOptions(trellis, {{}, permIndex}).
    termination(fec::Convolutional::Tail).
    bitOrdering(fec::Turbo::Alternate);
  auto decoder = fec::Turbo::DecoderOptions().algorithm(fec::Codec::Approximate).iterations(10).scheduling(fec::Turbo::Serial);
  
  auto structure = fec::Turbo::Structure(encoder, decoder);
  test_suite* ts1 = BOOST_TEST_SUITE("default");
  ts1->add( BOOST_TEST_CASE(std::bind(&test_encodeBlock, structure )));
  ts1->add( BOOST_TEST_CASE(std::bind(&test_decodeBlock, fec::Turbo(structure), -4.0) ));
  ts1->add( BOOST_TEST_CASE(std::bind(&test_soDecodeBlock, fec::Turbo(structure), 0.0) ));
  ts1->add( BOOST_TEST_CASE(std::bind(&test_soDecodeBlock_parityOut, fec::Turbo(structure), 0.0) ));
  ts1->add( BOOST_TEST_CASE(std::bind(&test_soDecodeBlock_systIn, fec::Turbo(structure), 0.0) ));
  ts1->add( BOOST_TEST_CASE(std::bind(&test_soDecodeBlock_2phases, fec::Turbo(structure), 0.0) ));
  
  ts1->add( BOOST_TEST_CASE(std::bind(&test_encode, fec::Turbo(structure,1), 5 )));
  ts1->add( BOOST_TEST_CASE(std::bind(&test_encode, fec::Turbo(structure,2), 5 )));
  ts1->add( BOOST_TEST_CASE(std::bind(&test_encode_badMsgSize, fec::Turbo(structure,2) )));
  
  ts1->add( BOOST_TEST_CASE(std::bind(&test_decode, fec::Turbo(structure,1), 5 )));
  ts1->add( BOOST_TEST_CASE(std::bind(&test_decode, fec::Turbo(structure,2), 5 )));
  ts1->add( BOOST_TEST_CASE(std::bind(&test_decode_badParitySize, fec::Turbo(structure,2) )));
  
  ts1->add( BOOST_TEST_CASE(std::bind(&test_soDecode, fec::Turbo(structure,1), 5 )));
  ts1->add( BOOST_TEST_CASE(std::bind(&test_soDecode, fec::Turbo(structure,2), 5 )));
  ts1->add( BOOST_TEST_CASE(std::bind(&test_soDecode_badParitySize, fec::Turbo(structure) )));
  ts1->add( BOOST_TEST_CASE(std::bind(&test_soDecode_badSystSize, fec::Turbo(structure) )));
  ts1->add( BOOST_TEST_CASE(std::bind(&test_soDecode_badStateSize, fec::Turbo(structure) )));
  ts1->add( BOOST_TEST_CASE(std::bind(&test_soDecode_noParity, fec::Turbo(structure) )));
  
  encoder.termination(fec::Convolutional::Tail);
  structure = fec::Turbo::Structure(encoder, decoder);
  test_suite* ts2 = BOOST_TEST_SUITE("tail");
  ts2->add( BOOST_TEST_CASE(std::bind( &test_encodeBlock, structure) ));
  ts2->add( BOOST_TEST_CASE(std::bind( &test_decodeBlock, fec::Turbo(structure), 2.0) ));
  ts2->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock, fec::Turbo(structure), 2.0) ));
  ts2->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock_parityOut, fec::Turbo(structure), 2.0) ));
  ts2->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock_systIn, fec::Turbo(structure), 2.0) ));
  
  decoder.algorithm(fec::Codec::Table);
  structure = fec::Turbo::Structure(encoder, decoder);
  test_suite* ts3 = BOOST_TEST_SUITE("table");
  ts3->add( BOOST_TEST_CASE(std::bind( &test_encodeBlock, structure) ));
  ts3->add( BOOST_TEST_CASE(std::bind( &test_decodeBlock, fec::Turbo(structure), 2.0) ));
  ts3->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock, fec::Turbo(structure), 2.0) ));
  ts3->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock_parityOut, fec::Turbo(structure), 2.0) ));
  ts3->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock_systIn, fec::Turbo(structure), 2.0) ));
  
  decoder.algorithm(fec::Codec::Approximate);
  structure = fec::Turbo::Structure(encoder, decoder);
  test_suite* ts4 = BOOST_TEST_SUITE("approximate");
  ts4->add( BOOST_TEST_CASE(std::bind( &test_encodeBlock, structure) ));
  ts4->add( BOOST_TEST_CASE(std::bind( &test_decodeBlock, fec::Turbo(structure), 2.0) ));
  ts4->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock, fec::Turbo(structure), 2.0) ));
  ts4->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock_parityOut, fec::Turbo(structure), 2.0) ));
  ts4->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock_systIn, fec::Turbo(structure), 2.0) ));
  
  encoder.bitOrdering(fec::Turbo::Pack);
  structure = fec::Turbo::Structure(encoder, decoder);
  test_suite* ts5 = BOOST_TEST_SUITE("pack");
  ts5->add( BOOST_TEST_CASE(std::bind( &test_encodeBlock, structure) ));
  ts5->add( BOOST_TEST_CASE(std::bind( &test_decodeBlock, fec::Turbo(structure), 2.0) ));
  ts5->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock, fec::Turbo(structure), 2.0) ));
  ts5->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock_parityOut, fec::Turbo(structure), 2.0) ));
  ts5->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock_systIn, fec::Turbo(structure), 2.0) ));
  
  framework::master_test_suite().add(ts1);
  framework::master_test_suite().add(ts2);
  framework::master_test_suite().add(ts3);
  framework::master_test_suite().add(ts4);
  framework::master_test_suite().add(ts5);
  
  return 0;
}