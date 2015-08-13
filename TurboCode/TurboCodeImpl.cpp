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
 
 Definition of TurboCodeImpl class
 ******************************************************************************/

#include "TurboCodeImpl.h"

using namespace fec;

void TurboCodeImpl::appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const
{
  switch (codeStructure_.structureType()) {
    case TurboCodeStructure::Serial:
      for (size_t i = 0; i < n; ++i) {
        serialDecodeBloc(parityIn, extrinsicIn, messageOut, extrinsicOut);
        parityIn += codeStructure_.paritySize();
        extrinsicIn += extrinsicSize();
        messageOut += codeStructure_.msgSize();
        extrinsicOut += extrinsicSize();
      }
      break;
      
    case TurboCodeStructure::Parallel:
      for (size_t i = 0; i < n; ++i) {
        parallelDecodeBloc(parityIn, extrinsicIn, messageOut, extrinsicOut);
        parityIn += codeStructure_.paritySize();
        extrinsicIn += extrinsicSize();
        messageOut += codeStructure_.msgSize();
        extrinsicOut += extrinsicSize();
      }
      
    default:
      break;
  }
}

void TurboCodeImpl::serialDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut) const
{
  std::vector<LlrType> msg(codeStructure_.msgSize());

  std::copy(extrinsicIn, extrinsicIn+codeStructure_.msgSize()+codeStructure_.msgTailSize(), extrinsicOut);
  std::vector<LlrType> extrinsicInterl(codeStructure_.msgSize() + codeStructure_.msgTailSize(), 0);
  for (size_t i = 0; i < codeStructure_.iterationCount() - 1; ++i) {
    auto parityIt = parityIn + codeStructure_.msgSize() + codeStructure_.msgTailSize();
    auto extrinsicInterlTailIt = extrinsicInterl.begin() + codeStructure_.msgSize();
    auto extrinsicTailIt = extrinsicOut + codeStructure_.msgSize();
    auto parityTailIt = parityIn + codeStructure_.msgSize();
    for (size_t j = 0; j < codeStructure_.structureCount(); ++j) {
      for (size_t k = 0; k < codeStructure_.structure(j).msgSize(); ++k) {
        extrinsicInterl[k] = extrinsicOut[codeStructure_.interleaver(j)[k]] + parityIn[codeStructure_.interleaver(j)[k]];
      }
      for (size_t k = 0; k < codeStructure_.structure(j).msgTailSize(); ++k) {
        extrinsicInterlTailIt[k] = extrinsicTailIt[k] + parityTailIt[k];
      }
      
      code_[j]->appDecodeBloc(parityIt, extrinsicInterl.begin(), msg.begin(), extrinsicInterl.begin());
      codeStructure_.interleaver(j).deInterleaveBloc<LlrType>(extrinsicInterl.begin(), extrinsicOut);
      std::copy(extrinsicInterlTailIt, extrinsicInterlTailIt + codeStructure_.structure(j).msgTailSize(), extrinsicTailIt);

      parityIt += codeStructure_.structure(j).paritySize();
      extrinsicTailIt += codeStructure_.structure(j).msgTailSize();
      parityTailIt += codeStructure_.structure(j).msgTailSize();
    }
  }
  auto parityIt = parityIn + codeStructure_.msgSize() + codeStructure_.msgTailSize();
  auto extrinsicInterlTailIt = extrinsicInterl.begin() + codeStructure_.msgSize();
  auto extrinsicTailIt = extrinsicOut + codeStructure_.msgSize();
  auto parityTailIt = parityIn + codeStructure_.msgSize();
  std::copy(parityIn, parityIn + codeStructure_.msgSize(), messageOut);
  for (size_t i = 0; i < codeStructure_.structureCount(); ++i) {
    for (size_t k = 0; k < codeStructure_.structure(i).msgSize(); ++k) {
      extrinsicInterl[k] = extrinsicOut[codeStructure_.interleaver(i)[k]] + parityIn[codeStructure_.interleaver(i)[k]];
    }
    for (size_t k = 0; k < codeStructure_.structure(i).msgTailSize(); ++k) {
      extrinsicInterlTailIt[k] = extrinsicTailIt[k] + parityTailIt[k];
    }
    
    code_[i]->appDecodeBloc(parityIt, extrinsicInterl.begin(), msg.begin(), extrinsicInterl.begin());
    std::copy(extrinsicInterlTailIt, extrinsicInterlTailIt + codeStructure_.structure(i).msgTailSize(), extrinsicTailIt);
    
    for (size_t k = 0; k < codeStructure_.structure(i).msgSize(); ++k) {
      messageOut[codeStructure_.interleaver(i)[k]] += extrinsicInterl[k];
      extrinsicOut[codeStructure_.interleaver(i)[k]] = extrinsicInterl[k];
    }
    parityIt += codeStructure_.structure(i).paritySize();
    extrinsicTailIt += codeStructure_.structure(i).msgTailSize();
    parityTailIt += codeStructure_.structure(i).msgTailSize();
  }
}

void TurboCodeImpl::parallelDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut) const
{
  std::vector<LlrType> msg(codeStructure_.msgSize());
  auto extrinsicOutIt = extrinsicOut;
  auto extrinsicInIt = extrinsicIn;
  auto parityTail = parityIn + codeStructure_.msgSize();
  for (size_t i = 0; i < codeStructure_.structureCount(); ++i) {
    for (size_t k = 0; k < codeStructure_.structure(i).msgSize(); ++k) {
      extrinsicOutIt[k] = parityIn[codeStructure_.interleaver(i)[k]] + extrinsicInIt[k];
    }
    extrinsicOutIt += codeStructure_.structure(i).msgSize();
    extrinsicInIt += codeStructure_.structure(i).msgSize();
    for (size_t k = 0; k < codeStructure_.structure(i).msgTailSize(); ++k) {
      extrinsicOutIt[k] = parityTail[k] + extrinsicInIt[k];
    }
    extrinsicOutIt += codeStructure_.structure(i).msgTailSize();
    extrinsicInIt += codeStructure_.structure(i).msgTailSize();
    parityTail += codeStructure_.structure(i).msgTailSize();
  }
  
  std::vector<LlrType> parityBuffer(extrinsicSize());
  for (size_t i = 0; i < codeStructure_.iterationCount(); ++i) {
    auto parityIt = parityIn + codeStructure_.msgSize() + codeStructure_.msgTailSize();
    auto extrinsicOutIt = extrinsicOut;
    for (size_t j = 0; j < codeStructure_.structureCount(); ++j) {

      code_[j]->appDecodeBloc(parityIt, extrinsicOutIt, msg.begin(), extrinsicOutIt);

      extrinsicOutIt += codeStructure_.structure(j).msgSize() + codeStructure_.structure(j).msgTailSize();
      parityIt += codeStructure_.structure(j).paritySize();
    }
    
    extrinsicOutIt = extrinsicOut;
    auto parityTmp = parityBuffer.begin();
    auto parityTail = parityIn + codeStructure_.msgSize();
    std::fill(messageOut, messageOut + codeStructure_.msgSize(), 0);
    for (size_t j = 0; j < codeStructure_.structureCount(); ++j) {
      for (size_t k = 0; k < codeStructure_.structure(j).msgSize(); ++k) {
        parityTmp[k] = extrinsicOutIt[k];
        extrinsicOutIt[k] = messageOut[codeStructure_.interleaver(j)[k]];
        messageOut[codeStructure_.interleaver(j)[k]] += parityTmp[k];
      }
      extrinsicOutIt += codeStructure_.structure(j).msgSize();
      parityTmp += codeStructure_.structure(j).msgSize();
      for (size_t k = 0; k < codeStructure_.structure(j).msgTailSize(); ++k) {
        extrinsicOutIt[k] += parityTail[k];
      }
      extrinsicOutIt += codeStructure_.structure(j).msgTailSize();
      parityTail += codeStructure_.structure(j).msgTailSize();
    }
    std::copy(parityIn, parityIn + codeStructure_.msgSize(), messageOut);
    
    for (int64_t j = codeStructure_.structureCount()-1; j >= 0; --j) {
      extrinsicOutIt -= codeStructure_.structure(j).msgSize() + codeStructure_.structure(j).msgTailSize();
      parityTmp -= codeStructure_.structure(j).msgSize();
      for (size_t k = 0; k < codeStructure_.structure(j).msgSize(); ++k) {
        extrinsicOutIt[k] += messageOut[codeStructure_.interleaver(j)[k]];
        messageOut[codeStructure_.interleaver(j)[k]] += parityTmp[k];
      }
    }
  }
}
