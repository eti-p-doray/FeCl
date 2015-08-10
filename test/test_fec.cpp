/*******************************************************************************
 *  \file test_ldpc.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-07-15
 *  \version Last update : 2015-07-15
 *
 *  Definition of MapCode class
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

std::vector<uint8_t> randomBits(size_t n) {
  uint64_t seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::independent_bits_engine<std::mt19937,1,uint8_t> bitGenerator((uint32_t(seed)));
  std::vector<uint8_t> msg(n);
  for (size_t i = 0; i < msg.size(); i++) {
    msg[i] = bitGenerator();
  }
  return msg;
}

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
  std::vector<std::shared_ptr<fec::Code>> codes;
  codes.push_back( fec::Code::create(fec::ConvolutionalCodeStructure(fec::TrellisStructure({3}, {{04, 05}}), 8, fec::ConvolutionalCodeStructure::Truncation,  fec::ConvolutionalCodeStructure::MaxLogMap), 4) );
  codes.push_back( fec::Code::create(fec::ConvolutionalCodeStructure(fec::TrellisStructure({3}, {{04, 05}}), 8, fec::ConvolutionalCodeStructure::Truncation,  fec::ConvolutionalCodeStructure::LogMap), 4) );
  codes.push_back( fec::Code::create(fec::LdpcCodeStructure(fec::LdpcCodeStructure::gallagerConstruction(1024, 3, 5)), 4) );
  
  uint64_t seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::srand ( unsigned (seed ) );
  std::vector<size_t> randPerm1(1024);
  std::vector<size_t> randPerm2(1024);
  for (size_t i = 0; i < randPerm1.size(); i++) {
    randPerm1[i] = i;
    randPerm2[i] = i;
  }
  std::random_shuffle (randPerm2.begin(), randPerm2.end());
  
  auto trellis = fec::TrellisStructure({3}, {{05}});
  codes.push_back( fec::Code::create(fec::TurboCodeStructure({trellis, trellis}, {randPerm1,randPerm2}, 5, fec::TurboCodeStructure::Parallel,  fec::ConvolutionalCodeStructure::MaxLogMap), 4 ));
  codes.push_back( fec::Code::create(fec::TurboCodeStructure({trellis, trellis}, {randPerm1,randPerm2}, 5, fec::TurboCodeStructure::Serial,  fec::ConvolutionalCodeStructure::MaxLogMap), 4 ));
  
  for (auto& code : codes) {
    framework::master_test_suite().
    add( BOOST_TEST_CASE( std::bind( &encode_badMsgSize_test, code )));
    
    framework::master_test_suite().
    add( BOOST_TEST_CASE( std::bind( &decode_test, code, randomBits(code->msgSize()*1) )));
    framework::master_test_suite().
    add( BOOST_TEST_CASE( std::bind( &decode_test, code, randomBits(code->msgSize()*5) )));
    
    framework::master_test_suite().
    add( BOOST_TEST_CASE( std::bind( &softOutDecode_test, code, randomBits(code->msgSize()*1) )));
    framework::master_test_suite().
    add( BOOST_TEST_CASE( std::bind( &softOutDecode_test, code, randomBits(code->msgSize()*5) )));
    
    framework::master_test_suite().
    add( BOOST_TEST_CASE( std::bind( &appDecode_test, code, randomBits(code->msgSize()*1) )));
    framework::master_test_suite().
    add( BOOST_TEST_CASE( std::bind( &appDecode_test, code, randomBits(code->msgSize()*5) )));
  }
  return 0;
}