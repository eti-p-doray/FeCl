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

void test_ldpc_soDecode_systOut(const fec::Ldpc& code, size_t n = 1)
{
  double snr = -5.0;
  
  std::vector<fec::BitField<bool>> msg(code.msgSize()*n, 1);
  std::vector<fec::BitField<uint8_t>> parity;
  code.encode(msg, parity);
  
  std::vector<fec::LlrType> parityIn = distort(parity, snr);
  
  std::vector<fec::LlrType> parityOut;
  std::vector<fec::LlrType> systOut;
  code.soDecode(fec::Codec::Input<>().parity(parityIn), fec::Codec::Output<>().parity(parityOut).syst(systOut));
  
  for (size_t i = 0; i < systOut.size(); ++i) {
    BOOST_REQUIRE(parityOut[i] == systOut[i]);
  }
}

test_suite* test_ldpc(const fec::Ldpc::Structure& structure, double snr, const std::string& name)
{
  test_suite* ts = BOOST_TEST_SUITE(name);
  
  ts->add( BOOST_TEST_CASE(std::bind(&test_encodeBlock, structure )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_encode, fec::Ldpc(structure,1), 1 )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_encode, fec::Ldpc(structure,1), 5 )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_encode, fec::Ldpc(structure,2), 5 )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_encode_badMsgSize, fec::Ldpc(structure) )));
  
  ts->add( BOOST_TEST_CASE(std::bind( &test_decode, fec::Ldpc(structure,1), snr, 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_decode, fec::Ldpc(structure,1), snr, 5) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_decode, fec::Ldpc(structure,2), snr, 5) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_decode_badParitySize, fec::Ldpc(structure) )));
  
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode, fec::Ldpc(structure), -5.0, 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_parityOut, fec::Ldpc(structure), snr, 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_0stateIn, fec::Ldpc(structure), 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_0systIn, fec::Ldpc(structure), 1) ));
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_systIn, fec::Ldpc(structure), -5.0, 1) ));
  auto structure2 = structure;
  auto decoder = structure2.getDecoderOptions();
  decoder.iterations_ = 2*decoder.iterations_;
  structure2.setDecoderOptions(decoder);
  ts->add( BOOST_TEST_CASE(std::bind( &test_soDecode_2phases, fec::Ldpc(structure), fec::Ldpc(structure2), 1)));
  ts->add( BOOST_TEST_CASE(std::bind( &test_ldpc_soDecode_systOut, fec::Ldpc(structure), 1)));
  
  ts->add( BOOST_TEST_CASE(std::bind(&test_soDecode_badParitySize, fec::Ldpc(structure) )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_soDecode_badSystSize, fec::Ldpc(structure) )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_soDecode_badStateSize, fec::Ldpc(structure) )));
  ts->add( BOOST_TEST_CASE(std::bind(&test_soDecode_noParity, fec::Ldpc(structure) )));
  
  ts->add( BOOST_TEST_CASE(std::bind(&test_saveLoad, fec::Ldpc(structure) )));
  
  return ts;
}

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
  uint64_t seed = 0;
  auto checkMatrix = fec::Ldpc::gallagerConstruction(2048, 3, 5, seed);
  auto encoder = fec::Ldpc::EncoderOptions(checkMatrix);
  auto decoder = fec::Ldpc::DecoderOptions().algorithm(fec::Codec::Exact).iterations(30);
  
  auto structure = fec::Ldpc::Structure(encoder, decoder);
  std::cout << structure.msgSize() << std::endl;
  framework::master_test_suite().add(test_ldpc(structure, 2.0, "exact"));
  
  decoder.algorithm(fec::Codec::Linear);
  structure.setDecoderOptions(decoder);
  framework::master_test_suite().add(test_ldpc(structure, 2.0, "linear"));
  
  decoder.algorithm(fec::Codec::Approximate);
  structure.setDecoderOptions(decoder);
  framework::master_test_suite().add(test_ldpc(structure, 2.0, "approximate"));
  
  return 0;
}