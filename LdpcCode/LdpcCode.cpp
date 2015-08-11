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
 
 Definition of MapCode class
 ******************************************************************************/

#include "LdpcCode.h"

using namespace fec;

BOOST_CLASS_EXPORT_IMPLEMENT(LdpcCode);

const char * LdpcCode::get_key() const {
  return boost::serialization::type_info_implementation<LdpcCode>::type::get_const_instance().get_key();
}

/**
 *  LdpcCode constructor
 *  \param  codeStructure Code structure used for encoding and decoding
 *  \param  workGroupSize Number of thread used for decoding
 */
LdpcCode::LdpcCode(const LdpcCodeStructure& codeStructure, int workGroupdSize) :
  Code(workGroupdSize),
  codeStructure_(codeStructure)
{
}

void LdpcCode::syndrome(const std::vector<uint8_t>& parity, std::vector<uint8_t>& syndrome) const
{
  uint64_t blocCount = parity.size() / (codeStructure_.paritySize());
  if (parity.size() != blocCount * codeStructure_.paritySize()) {
    throw std::invalid_argument("Invalid size for message");
  }
  
  syndrome.resize(blocCount * codeStructure_.parityCheck().rows());
  
  auto syndromeIt = syndrome.begin();
  auto parityIt = parity.begin();
  
  for (size_t i = 0; i < blocCount; ++i) {
    codeStructure_.syndrome(parityIt, syndromeIt);
    parityIt += codeStructure_.paritySize();
    syndromeIt += codeStructure_.parityCheck().rows();
  }
}

void LdpcCode::encodeBloc(std::vector<uint8_t>::const_iterator messageIt, std::vector<uint8_t>::iterator parityIt) const
{
  codeStructure_.encode(messageIt, parityIt);
}

void LdpcCode::appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const
{
  auto worker = BpDecoder::create(codeStructure_);
  worker->appDecodeNBloc(parityIn, extrinsicIn, messageOut, extrinsicOut, n);
}

void LdpcCode::softOutDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut, size_t n) const
{
  auto worker = BpDecoder::create(codeStructure_);
  worker->softOutDecodeNBloc(parityIn, messageOut, n);
}

void LdpcCode::decodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<uint8_t>::iterator messageOut, size_t n) const
{
  auto worker = BpDecoder::create(codeStructure_);
  std::vector<LlrType> messageAPosteriori(n * codeStructure_.msgSize());
  worker->softOutDecodeNBloc(parityIn, messageAPosteriori.begin(), n);
  
  for (auto messageIt = messageAPosteriori.begin(); messageIt < messageAPosteriori.end(); ++messageIt, ++messageOut) {
    *messageOut = *messageIt > 0;
  }
}
