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

#include "Code.h"

#include "ConvolutionalCode/ConvolutionalCode.h"
#include "TurboCode/TurboCode.h"
#include "LdpcCode/LdpcCode.h"

void encode_badMsgSize_test(const std::shared_ptr<fec::Code>& code)
{
  std::vector<uint8_t> msg(code->msgSize()+1);
  
  std::vector<uint8_t> parity;
  try {
    code->encode(msg, parity);
  } catch (std::exception& e) {
    return;
  }
  BOOST_ERROR("Wrong msg size exception not thrown");
}

void decode_test(const std::shared_ptr<fec::Code>& code, const std::vector<uint8_t>& msg)
{
  std::vector<uint8_t> parity;
  code->encode(msg, parity);
  
  std::vector<fec::LlrType> codeL(parity.size());
  for (size_t i = 0; i < parity.size(); ++i) {
    codeL[i] = parity[i]*2 - 1.0;
  }
  
  std::vector<uint8_t> decodedMsg;
  code->decode(codeL, decodedMsg);
  
  for (size_t i = 0; i < code->msgSize(); i++) {
    BOOST_REQUIRE(decodedMsg[i] == msg[i]);
  }
}

/*void decode_badParitySize_test(const std::shared_ptr<fec::Code>& code)
{
  std::vector<fec::LlrType> codeL(parity.size()+1);
  
  std::vector<uint8_t> decodedMsg;
  try {
    code->decode(codeL, decodedMsg);
  } catch (std::exception& e) {
    return;
  }
  BOOST_ERROR("Wrong parity size exception not thrown");
}*/

void softOutDecode_test(const std::shared_ptr<fec::Code>& code, const std::vector<uint8_t>& msg)
{
  std::vector<uint8_t> parity;
  code->encode(msg, parity);
  
  std::vector<fec::LlrType> codeL(parity.size());
  for (size_t i = 0; i < parity.size(); ++i) {
    codeL[i] = parity[i]*2 - 1.0;
  }
  
  std::vector<fec::LlrType> decodedMsg;
  code->softOutDecode(codeL, decodedMsg);
  
  for (size_t i = 0; i < code->msgSize(); i++) {
    BOOST_REQUIRE((decodedMsg[i] > 0) == msg[i]);
  }
}

void appDecode_test(const std::shared_ptr<fec::Code>& code, const std::vector<uint8_t>& msg)
{
  std::vector<uint8_t> parity;
  code->encode(msg, parity);
  
  std::vector<fec::LlrType> codeL(parity.size());
  for (size_t i = 0; i < parity.size(); ++i) {
    codeL[i] = parity[i]*2 - 1.0;
  }
  
  std::vector<fec::LlrType> decodedMsg;
  code->softOutDecode(codeL, decodedMsg);
  
  std::vector<fec::LlrType> extrinsic(code->extrinsicSize() * msg.size() / code->msgSize(), 0);
  std::vector<fec::LlrType> decodedMsg1;
  code->appDecode(codeL, extrinsic, decodedMsg1, extrinsic);
  
  std::vector<fec::LlrType> decodedMsg2;
  code->appDecode(codeL, extrinsic, decodedMsg2, extrinsic);
  
  for (size_t i = 0; i < code->msgSize(); i++) {
    BOOST_REQUIRE((decodedMsg1[i] > 0) == msg[i]);
    BOOST_REQUIRE((decodedMsg2[i] > 0) == msg[i]);
    BOOST_REQUIRE(decodedMsg1[i] == decodedMsg[i]);
  }
}

void save_test(const std::shared_ptr<fec::Code>& code) {
  
}

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
  std::vector<std::shared_ptr<fec::Code>> codes;
  codes.push_back( fec::Code::create(fec::ConvolutionalCode::Structure(fec::Trellis({3}, {{04, 05}}), 8, fec::ConvolutionalCode::Truncation,  fec::ConvolutionalCode::MaxLogMap), 4) );
  codes.push_back( fec::Code::create(fec::ConvolutionalCode::Structure(fec::Trellis({3}, {{04, 05}}), 8, fec::ConvolutionalCode::Truncation,  fec::ConvolutionalCode::LogMap), 4) );
  codes.push_back( fec::Code::create(fec::ConvolutionalCode::Structure(fec::Trellis({3}, {{04, 05}}), 8, fec::ConvolutionalCode::Tail,  fec::ConvolutionalCode::LogMap), 4) );
  
  codes.push_back( fec::Code::create(fec::LdpcCode::Structure(fec::LdpcCode::Structure::gallagerConstruction(1024, 3, 5)), 4) );
  
  uint64_t seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::srand ( unsigned (seed ) );
  std::vector<size_t> randPerm1(1024);
  std::vector<size_t> randPerm2(1024);
  for (size_t i = 0; i < randPerm1.size(); i++) {
    randPerm1[i] = i;
    randPerm2[i] = i;
  }
  std::random_shuffle (randPerm2.begin(), randPerm2.end());
  
  auto trellis = fec::Trellis({3}, {{05}});
  codes.push_back( fec::Code::create(fec::TurboCode::Structure({trellis, trellis}, {randPerm1,randPerm2}, {fec::ConvolutionalCode::Truncation,fec::ConvolutionalCode::Truncation}, 5, fec::TurboCode::Parallel,  fec::ConvolutionalCode::MaxLogMap), 4 ));
  codes.push_back( fec::Code::create(fec::TurboCode::Structure({trellis, trellis}, {randPerm1,randPerm2}, {fec::ConvolutionalCode::Truncation,fec::ConvolutionalCode::Truncation}, 5, fec::TurboCode::Serial,  fec::ConvolutionalCode::MaxLogMap), 4 ));
  codes.push_back( fec::Code::create(fec::TurboCode::Structure({trellis, trellis}, {randPerm1,randPerm2}, {fec::ConvolutionalCode::Tail,fec::ConvolutionalCode::Tail}, 5, fec::TurboCode::Parallel,  fec::ConvolutionalCode::MaxLogMap), 4 ));
  codes.push_back( fec::Code::create(fec::TurboCode::Structure({trellis, trellis}, {randPerm1,randPerm2}, {fec::ConvolutionalCode::Truncation,fec::ConvolutionalCode::Tail}, 5, fec::TurboCode::Serial,  fec::ConvolutionalCode::MaxLogMap), 4 ));
  
  for (auto& code : codes) {
    framework::master_test_suite().
    add( BOOST_TEST_CASE( std::bind( &encode_badMsgSize_test, code )));
    
    framework::master_test_suite().
    add( BOOST_TEST_CASE( std::bind( &decode_test, code, std::vector<uint8_t>(code->msgSize()*5, 0) )));
    framework::master_test_suite().
    add( BOOST_TEST_CASE( std::bind( &decode_test, code, std::vector<uint8_t>(code->msgSize()*5, 1) )));
    
    framework::master_test_suite().
    add( BOOST_TEST_CASE( std::bind( &softOutDecode_test, code, std::vector<uint8_t>(code->msgSize()*5, 0) )));
    framework::master_test_suite().
    add( BOOST_TEST_CASE( std::bind( &softOutDecode_test, code, std::vector<uint8_t>(code->msgSize()*5, 1) )));
    
    framework::master_test_suite().
    add( BOOST_TEST_CASE( std::bind( &appDecode_test, code, std::vector<uint8_t>(code->msgSize()*5, 0) )));
    framework::master_test_suite().
    add( BOOST_TEST_CASE( std::bind( &appDecode_test, code, std::vector<uint8_t>(code->msgSize()*5, 1) )));
    
    framework::master_test_suite().
    add( BOOST_TEST_CASE( std::bind( &save_test, code )));
  }
  return 0;
}