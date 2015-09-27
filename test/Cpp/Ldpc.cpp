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
  auto checkMatrix = fec::Ldpc::Gallager::matrix(2048, 3, 5, seed);
  auto encoder = fec::Ldpc::EncoderOptions(checkMatrix);
  auto decoder = fec::Ldpc::DecoderOptions().algorithm(fec::Codec::Exact).iterations(30);
  
  auto structure = fec::Ldpc::Structure(encoder, decoder);
  framework::master_test_suite().add(test_ldpc(structure, 2.0, "exact"));
  
  decoder.algorithm(fec::Codec::Linear);
  structure.setDecoderOptions(decoder);
  framework::master_test_suite().add(test_ldpc(structure, 2.0, "linear"));
  
  decoder.algorithm(fec::Codec::Approximate);
  structure.setDecoderOptions(decoder);
  framework::master_test_suite().add(test_ldpc(structure, 2.0, "approximate"));
  
  return 0;
}