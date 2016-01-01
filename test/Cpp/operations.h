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
#include <cstdint>

#include "Serialization.h"
#include "Codec.h"
#include "Convolutional.h"
#include "Turbo.h"
#include "Ldpc.h"

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& a)
{
  for (uint64_t i = 0; i < a.size(); i++) {
    os << float(a[i]) << " ";
  }
  return os;
}

template <typename T>
std::vector<double> distort(const std::vector<fec::BitField<T>>& input, double snrdb)
{
  const int8_t bpsk[2] = {-1, 1};
  
  double snr = pow(10.0, snrdb/10.0);
  
  uint64_t seed = 0;
  std::minstd_rand0 randomGenerator;
  randomGenerator.seed(uint32_t(seed));
  std::normal_distribution<double> normalDistribution(snr*4.0, 4.0*sqrt(snr/2.0));
  
  std::vector<double> llr(input.size());
  for (int i = 0; i < input.size(); i++) {
    llr[i] = bpsk[input[i]] * normalDistribution(randomGenerator);
  }
  return llr;
}

void test_encode_puncture(const fec::Codec& codec, const fec::Permutation& perm, const fec::Codec& puncturedCodec, size_t n)
{
  std::vector<fec::BitField<size_t>> msg(codec.msgSize()*n, 1);
  std::vector<fec::BitField<size_t>> parity = codec.encode(msg);
  std::vector<fec::BitField<size_t>> puncturedParity1 = perm.permute(parity);
  std::vector<fec::BitField<size_t>> puncturedParity2 = puncturedCodec.encode(msg);
  
  BOOST_REQUIRE(puncturedParity1.size() == puncturedParity2.size());
  for (size_t i = 0; i < puncturedParity1.size(); ++i) {
    BOOST_REQUIRE(puncturedParity1[i] == puncturedParity2[i]);
  }
}

void test_encode(const fec::Codec& code, size_t n)
{
  std::vector<fec::BitField<size_t>> msg(code.msgSize()*n, 1);
  std::vector<fec::BitField<size_t>> parity;
  code.encode(msg, parity);
  BOOST_CHECK(code.check(parity));
}

void test_encode_badMsgSize(const fec::Codec& code)
{
  std::vector<fec::BitField<size_t>> msg(code.msgSize()+1);
  std::vector<fec::BitField<size_t>> parity;
  try {
    code.encode(msg, parity);
  } catch (std::exception& e) {
    return;
  }
  BOOST_ERROR("Exception not thrown");
}

void test_saveLoad(const fec::Codec& code) {
  fec::detail::DerivedTypeHolder<fec::Convolutional, fec::Turbo, fec::Ldpc> derived;
  std::vector<char> archive(archiveSize(&code, derived));
  save(&code, &archive[0], archive.size(), derived);
  auto recover = fec::detail::load<fec::Codec>(&archive[0], archive.size(), derived);
  BOOST_CHECK(recover->msgSize() == code.msgSize());
  BOOST_CHECK(recover->systSize() == code.systSize());
  BOOST_CHECK(recover->paritySize() == code.paritySize());
  BOOST_CHECK(recover->stateSize() == code.stateSize());
}

void test_decode(const fec::Codec& code, double snr, size_t n = 1)
{
  std::vector<fec::BitField<size_t>> msg(code.msgSize()*n, 1);
  std::vector<fec::BitField<size_t>> parity(code.paritySize());
  code.encode(msg, parity);
  
  std::vector<double> parityIn = distort(parity, snr);
  std::vector<fec::BitField<size_t>> msgOut;
  code.decode(parityIn, msgOut);
  
  BOOST_REQUIRE(msgOut.size() == code.msgSize()*n);
  for (size_t i = 0; i < msg.size(); ++i) {
    BOOST_REQUIRE(msg[i] == msgOut[i]);
  }
}

void test_decode_badParitySize(const fec::Codec& code)
{
  std::vector<double> parityIn(code.paritySize()+1);
  std::vector<fec::BitField<size_t>> msgOut(code.msgSize());
  try {
    code.decode(parityIn, msgOut);
  } catch (std::exception& e) {
    return;
  }
  BOOST_ERROR("Exception not thrown");
}

void test_soDecode(const fec::Codec& code, double snr, size_t n = 1)
{
  std::vector<fec::BitField<size_t>> msg(code.msgSize()*n, 1);
  std::vector<fec::BitField<size_t>> parity;
  code.encode(msg, parity);
  
  std::vector<double> parityIn = distort(parity, snr);
  
  std::vector<double> msgOut;
  code.soDecode(fec::Codec::Input::parity(parityIn), fec::Codec::Output::msg(msgOut));
  
  std::vector<fec::BitField<size_t>> msgDec;
  code.decode(parityIn, msgDec);
  
  BOOST_REQUIRE(msgOut.size() == code.msgSize()*n);
  for (size_t i = 0; i < msg.size(); ++i) {
    BOOST_REQUIRE(msgDec[i] == (msgOut[i]>0));
  }
}

void test_soDecode_parityOut(const fec::Codec& code, double snr, size_t n = 1)
{
  std::vector<fec::BitField<size_t>> msg(code.msgSize()*n, 1);
  std::vector<fec::BitField<size_t>> parity;
  code.encode(msg, parity);
  
  std::vector<double> parityIn = distort(parity, snr);
  std::vector<double> msgOut;
  std::vector<double> parityOut;
  code.soDecode(fec::Codec::Input::parity(parityIn), fec::Codec::Output::msg(msgOut).parity(parityOut));
  
  BOOST_REQUIRE(msgOut.size() == code.msgSize()*n);
  BOOST_REQUIRE(parityOut.size() == code.paritySize()*n);
  for (size_t i = 0; i < msg.size(); ++i) {
    BOOST_REQUIRE(msg[i] == (msgOut[i]>0));
  }
  for (size_t i = 0; i < parity.size(); ++i) {
    if (parity[i] != ((parityIn[i]+parityOut[i])>0)) {
      std::cout << i << std::endl;
      std::cout << parity[i] << std::endl;
      std::cout << parityIn[i] << std::endl;
      std::cout << parityOut[i] << std::endl;
    }
    BOOST_REQUIRE(parity[i] == ((parityIn[i]+parityOut[i])>0));
  }
}

void test_soDecode_systIn(const fec::Codec& code, double snr, size_t n = 1)
{
  std::vector<fec::BitField<size_t>> msg(code.msgSize()*n, 1);
  std::vector<fec::BitField<size_t>> parity;
  code.encode(msg, parity);
  
  std::vector<double> parityIn = distort(parity, snr);
  std::vector<double> systIn = distort(msg, snr);
  systIn.resize(code.systSize());
  std::vector<double> msgOut;
  std::vector<double> parityOut;
  code.soDecode(fec::Codec::Input::parity(parityIn), fec::Codec::Output::msg(msgOut).parity(parityOut));
  
  size_t msgErrorCount1 = 0;
  for (size_t i = 0; i < msg.size(); ++i) {
    msgErrorCount1 += (msg[i] != msgOut[i]);
  }
  
  size_t parityErrorCount1 = 0;
  for (size_t i = 0; i < parity.size(); ++i) {
    parityErrorCount1 += (parity[i] != parityOut[i]);
  }
  
  code.soDecode(fec::Codec::Input::parity(parityIn).syst(systIn), fec::Codec::Output::msg(msgOut).parity(parityOut));
  
  size_t msgErrorCount2 = 0;
  for (size_t i = 0; i < msg.size()*n; ++i) {
    msgErrorCount2 += (msg[i] != msgOut[i]);
  }
  
  size_t parityErrorCount2 = 0;
  for (size_t i = 0; i < parity.size()*n; ++i) {
    parityErrorCount2 += (parity[i] != parityOut[i]);
  }
  
  BOOST_CHECK(msgErrorCount2 <=  msgErrorCount1);
  BOOST_CHECK(parityErrorCount2 <=  parityErrorCount1);
}

void test_soDecode_0systIn(const fec::Codec& code, size_t n = 1)
{
  double snr = -5.0;
  
  std::vector<fec::BitField<size_t>> msg(code.msgSize()*n, 1);
  std::vector<fec::BitField<size_t>> parity;
  code.encode(msg, parity);
  
  std::vector<double> parityIn = distort(parity, snr);
  std::vector<double> msgOut1;
  std::vector<double> systOut1;
  std::vector<double> parityOut1;
  code.soDecode(fec::Codec::Input::parity(parityIn), fec::Codec::Output::msg(msgOut1).syst(systOut1).parity(parityOut1));
  
  std::vector<double> syst(code.systSize()*n, 0);
  std::vector<double> msgOut2;
  std::vector<double> systOut2;
  std::vector<double> parityOut2;
  code.soDecode(fec::Codec::Input::parity(parityIn).syst(syst), fec::Codec::Output::msg(msgOut2).syst(systOut2).parity(parityOut2));
  
  BOOST_REQUIRE(msgOut1.size() == msgOut2.size());
  BOOST_REQUIRE(systOut1.size() == systOut2.size());
  BOOST_REQUIRE(parityOut1.size() == parityOut2.size());
  for (size_t i = 0; i < msgOut1.size(); ++i) {
    BOOST_REQUIRE(msgOut1[i] == msgOut2[i]);
  }
  for (size_t i = 0; i < systOut1.size(); ++i) {
    BOOST_REQUIRE(systOut1[i] == systOut2[i]);
  }
  for (size_t i = 0; i < parityOut1.size(); ++i) {
    BOOST_REQUIRE(parityOut1[i] == parityOut2[i]);
  }
}

void test_soDecode_0stateIn(const fec::Codec& code, size_t n = 1)
{
  double snr = -5.0;
  
  std::vector<fec::BitField<size_t>> msg(code.msgSize()*n, 1);
  std::vector<fec::BitField<size_t>> parity;
  code.encode(msg, parity);
  
  std::vector<double> parityIn = distort(parity, snr);
  std::vector<double> msgOut1;
  std::vector<double> systOut1;
  std::vector<double> parityOut1;
  code.soDecode(fec::Codec::Input::parity(parityIn), fec::Codec::Output::msg(msgOut1).syst(systOut1).parity(parityOut1));
  
  std::vector<double> state(code.stateSize()*n, 0);
  std::vector<double> msgOut2;
  std::vector<double> systOut2;
  std::vector<double> parityOut2;
  code.soDecode(fec::Codec::Input::parity(parityIn).state(state), fec::Codec::Output::msg(msgOut2).syst(systOut2).parity(parityOut2));
  
  BOOST_REQUIRE(msgOut1.size() == msgOut2.size());
  BOOST_REQUIRE(systOut1.size() == systOut2.size());
  BOOST_REQUIRE(parityOut1.size() == parityOut2.size());
  for (size_t i = 0; i < msgOut1.size(); ++i) {
    BOOST_REQUIRE(msgOut1[i] == msgOut2[i]);
  }
  for (size_t i = 0; i < systOut1.size(); ++i) {
    BOOST_REQUIRE(systOut1[i] == systOut2[i]);
  }
  for (size_t i = 0; i < parityOut1.size(); ++i) {
    BOOST_REQUIRE(parityOut1[i] == parityOut2[i]);
  }
}

void test_soDecode_2phases(const fec::Codec& code1, const fec::Codec& code2, size_t n = 1)
{
  double snr = -5.0;
  
  std::vector<fec::BitField<size_t>> msg(code1.msgSize()*n, 1);
  std::vector<fec::BitField<size_t>> parity1;
  std::vector<fec::BitField<size_t>> parity2;
  code1.encode(msg, parity1);
  code2.encode(msg, parity2);
  BOOST_REQUIRE(parity1.size() == parity2.size());
  for (size_t i = 0; i < parity1.size(); ++i) {
    BOOST_REQUIRE(parity1[i] == parity2[i]);
  }
  
  std::vector<double> parityIn = distort(parity1, snr);
  std::vector<double> msgOut1;
  std::vector<double> systOut1;
  std::vector<double> parityOut1;
  std::vector<double> state;
  code1.soDecode(fec::Codec::Input::parity(parityIn), fec::Codec::Output::state(state));
  code1.soDecode(fec::Codec::Input::parity(parityIn).state(state), fec::Codec::Output::msg(msgOut1).syst(systOut1).parity(parityOut1));
  
  std::vector<double> msgOut2;
  std::vector<double> systOut2;
  std::vector<double> parityOut2;
  code2.soDecode(fec::Codec::Input::parity(parityIn), fec::Codec::Output::msg(msgOut2).syst(systOut2).parity(parityOut2));
  
  BOOST_REQUIRE(msgOut1.size() == msgOut2.size());
  BOOST_REQUIRE(systOut1.size() == systOut2.size());
  BOOST_REQUIRE(parityOut1.size() == parityOut2.size());
  for (size_t i = 0; i < msgOut1.size(); ++i) {
    BOOST_REQUIRE(msgOut1[i] == msgOut2[i]);
  }
  for (size_t i = 0; i < systOut1.size(); ++i) {
    BOOST_REQUIRE(systOut1[i] == systOut2[i]);
  }
  for (size_t i = 0; i < parityOut1.size(); ++i) {
    BOOST_REQUIRE(parityOut1[i] == parityOut2[i]);
  }
}

void test_soDecode_badParitySize(const fec::Codec& code)
{
  std::vector<double> parityIn(code.paritySize()+1);
  std::vector<double> syst;
  try {
    code.soDecode(fec::Codec::Input::parity(parityIn), fec::Codec::Output::syst(syst));
  } catch (std::exception& e) {
    return;
  }
  BOOST_ERROR("Exception not thrown");
}

void test_soDecode_badSystSize(const fec::Codec& code)
{
  std::vector<double> parityIn(code.paritySize());
  std::vector<double> systIn(code.systSize()+1);
  std::vector<double> syst;
  try {
    code.soDecode(fec::Codec::Input::syst(systIn), fec::Codec::Output::syst(syst));
  } catch (std::exception& e) {
    return;
  }
  BOOST_ERROR("Exception not thrown");
}

void test_soDecode_badStateSize(const fec::Codec& code)
{
  std::vector<double> parityIn(code.paritySize());
  std::vector<double> stateIn(code.stateSize()+1);
  std::vector<double> syst;
  try {
    code.soDecode(fec::Codec::Input::state(stateIn), fec::Codec::Output::syst(syst));
  } catch (std::exception& e) {
    return;
  }
  BOOST_ERROR("Exception not thrown");
}

void test_soDecode_noParity(const fec::Codec& code)
{
  std::vector<double> syst;
  try {
    code.soDecode({}, fec::Codec::Output::syst(syst));
  } catch (std::exception& e) {
    return;
  }
  BOOST_ERROR("Exception not thrown");
}
