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
  fec::Trellis trellis({4}, {{015, 017}}, {015});
  size_t length = 64;
  auto encoder = fec::Convolutional::EncoderOptions(trellis, length).termination(fec::Convolutional::Truncation);
  auto decoder = fec::Convolutional::DecoderOptions().decoderType(fec::Codec::Approximate);
  
  auto structure = fec::Convolutional::Structure(encoder, decoder);
  test_suite* ts1 = BOOST_TEST_SUITE("default");
  ts1->add( BOOST_TEST_CASE(std::bind(&test_encodeBlock, structure )));
  ts1->add( BOOST_TEST_CASE(std::bind( &test_decodeBlock, fec::Convolutional(structure), 4.0) ));
  ts1->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock, fec::Convolutional(structure), 4.0) ));
  ts1->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock_parityOut, fec::Convolutional(structure), 4.0) ));
  ts1->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock_systIn, fec::Convolutional(structure), 4.0) ));
  ts1->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock_2phases, fec::Convolutional(structure), 4.0) ));
  
  ts1->add( BOOST_TEST_CASE(std::bind(&test_encode, fec::Convolutional(structure,1), 5 )));
  ts1->add( BOOST_TEST_CASE(std::bind(&test_encode, fec::Convolutional(structure,2), 5 )));
  ts1->add( BOOST_TEST_CASE(std::bind(&test_encode_badMsgSize, fec::Convolutional(structure,2) )));
  
  ts1->add( BOOST_TEST_CASE(std::bind(&test_decode, fec::Convolutional(structure,1), 5 )));
  ts1->add( BOOST_TEST_CASE(std::bind(&test_decode, fec::Convolutional(structure,2), 5 )));
  ts1->add( BOOST_TEST_CASE(std::bind(&test_decode_badParitySize, fec::Convolutional(structure,2) )));
  
  ts1->add( BOOST_TEST_CASE(std::bind(&test_soDecode, fec::Convolutional(structure,1), 5 )));
  ts1->add( BOOST_TEST_CASE(std::bind(&test_soDecode, fec::Convolutional(structure,2), 5 )));
  ts1->add( BOOST_TEST_CASE(std::bind(&test_soDecode_badParitySize, fec::Convolutional(structure,2) )));
  ts1->add( BOOST_TEST_CASE(std::bind(&test_soDecode_badSystSize, fec::Convolutional(structure,2) )));
  ts1->add( BOOST_TEST_CASE(std::bind(&test_soDecode_badStateSize, fec::Convolutional(structure,2) )));
  ts1->add( BOOST_TEST_CASE(std::bind(&test_soDecode_noParity, fec::Convolutional(structure,2) )));
  
  encoder.termination(fec::Convolutional::Tail);
  structure = fec::Convolutional::Structure(encoder, decoder);
  test_suite* ts2 = BOOST_TEST_SUITE("tail");
  ts2->add( BOOST_TEST_CASE(std::bind( &test_encodeBlock, structure) ));
  ts2->add( BOOST_TEST_CASE(std::bind( &test_decodeBlock, fec::Convolutional(structure), 4.0) ));
  ts2->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock, fec::Convolutional(structure), 4.0) ));
  ts2->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock_parityOut, fec::Convolutional(structure), 4.0) ));
  ts2->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock_systIn, fec::Convolutional(structure), 4.0) ));
  
  decoder.decoderType(fec::Codec::Table);
  structure = fec::Convolutional::Structure(encoder, decoder);
  test_suite* ts3 = BOOST_TEST_SUITE("table");
  ts3->add( BOOST_TEST_CASE(std::bind( &test_encodeBlock, structure) ));
  ts3->add( BOOST_TEST_CASE(std::bind( &test_decodeBlock, fec::Convolutional(structure), 4.0) ));
  ts3->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock, fec::Convolutional(structure), 4.0) ));
  ts3->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock_parityOut, fec::Convolutional(structure), 4.0) ));
  ts3->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock_systIn, fec::Convolutional(structure), 4.0) ));
  
  decoder.decoderType(fec::Codec::Approximate);
  structure = fec::Convolutional::Structure(encoder, decoder);
  test_suite* ts4 = BOOST_TEST_SUITE("approximate");
  ts4->add( BOOST_TEST_CASE(std::bind( &test_encodeBlock, structure) ));
  ts4->add( BOOST_TEST_CASE(std::bind( &test_decodeBlock, fec::Convolutional(structure), 4.0) ));
  ts4->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock, fec::Convolutional(structure), 4.0) ));
  ts4->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock_parityOut, fec::Convolutional(structure), 4.0) ));
  ts4->add( BOOST_TEST_CASE(std::bind( &test_soDecodeBlock_systIn, fec::Convolutional(structure), 4.0) ));
  
  
  framework::master_test_suite().add(ts1);
  framework::master_test_suite().add(ts2);
  framework::master_test_suite().add(ts3);
  framework::master_test_suite().add(ts4);
  
  return 0;
}