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
 
 Definition of TurboCode class
 ******************************************************************************/

#include "TurboCode.h"

using namespace fec;

BOOST_CLASS_EXPORT_IMPLEMENT(TurboCode);

const char * TurboCode::get_key() const {
  return boost::serialization::type_info_implementation<TurboCode>::type::get_const_instance().get_key();
}

/*******************************************************************************
 *  TurboCode constructor
 *  \param  codeStructure Code structure used for encoding and decoding
 *  \param  workGroupSize Number of thread used for decoding
 ******************************************************************************/
TurboCode::TurboCode(const TurboCodeStructure& codeStructure, int workGroupdSize) :
  Code(workGroupdSize),
  codeStructure_(codeStructure)
{
  for (size_t i = 0; i < codeStructure_.structureCount(); ++i) {
    code_.push_back(ConvolutionalCode(codeStructure_.structure(i), workGroupdSize));
  }
}

void TurboCode::encodeBloc(std::vector<uint8_t>::const_iterator messageIn, std::vector<uint8_t>::iterator parityOut) const
{
  std::copy(messageIn, messageIn + codeStructure_.msgSize(), parityOut);
  parityOut += codeStructure_.msgSize();
  std::vector<uint8_t> messageInterl;
  for (size_t i = 0; i < codeStructure_.structureCount(); ++i) {
    messageInterl.resize(codeStructure_.structure(i).msgSize());
    codeStructure_.interleaver(i).interleaveBloc<uint8_t>(messageIn, messageInterl.begin());
    code_[i].encodeBloc(messageInterl.begin(), parityOut);
    parityOut += codeStructure_.structure(i).paritySize();
  }
}

void TurboCode::appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const
{
  TurboCodeImpl worker(codeStructure_);
  worker.appDecodeNBloc(parityIn, extrinsicIn, messageOut, extrinsicOut, n);
}

void TurboCode::softOutDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut, size_t n) const
{
  std::vector<LlrType> extrinsic(n * extrinsicSize(), 0);
  appDecodeNBloc(parityIn, extrinsic.begin(), messageOut, extrinsic.begin(), n);
}

void TurboCode::decodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<uint8_t>::iterator messageOut, size_t n) const
{
  std::vector<LlrType> messageAPosteriori(n * codeStructure_.msgSize());
  softOutDecodeNBloc(parityIn, messageAPosteriori.begin(), n);

  for (auto messageIt = messageAPosteriori.begin(); messageIt < messageAPosteriori.end(); ++messageIt, ++messageOut) {
    *messageOut = *messageIt > 0;
  }
}
