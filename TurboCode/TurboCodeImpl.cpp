/*******************************************************************************
 *  \file TurboCodeImpl.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-07-22
 *  \version Last update : 2015-07-24
 *
 *  Definition of TurboCodeImpl class
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
  for (size_t k = 0; k < codeStructure_.msgSize(); ++k) {
    extrinsicOut[k] = extrinsicIn[k] + parityIn[k];
  }
  std::vector<LlrType> extrinsicInterl(codeStructure_.msgSize(), 0);
  for (size_t i = 0; i < codeStructure_.iterationCount() - 1; ++i) {
    auto parityIt = parityIn + codeStructure_.msgSize();
    for (size_t j = 0; j < codeStructure_.structureCount(); ++j) {
      for (size_t k = 0; k < codeStructure_.structure(j).msgSize(); ++k) {
        extrinsicInterl[k] = extrinsicOut[codeStructure_.interleaver(j)[k]] + parityIn[codeStructure_.interleaver(j)[k]];
      }
      
      //extrinsicInterl.resize(codeStructure_.structure(j).msgSize(), 0);
      //codeStructure_.interleaver(j).interleaveBloc<LlrType>(extrinsicOut, extrinsicInterl.begin());
      code_[j]->appDecodeBloc(parityIt, extrinsicInterl.begin(), msg.begin(), extrinsicInterl.begin());
      codeStructure_.interleaver(j).deInterleaveBloc<LlrType>(extrinsicInterl.begin(), extrinsicOut);
      //std::fill(extrinsicInterl.begin(), extrinsicInterl.end(), 0);
      
      //for (size_t k = 0; k < codeStructure_.structure(j).msgSize(); ++k) {
      //  extrinsicOut[codeStructure_.interleaver(j)[k]] += parityIn[codeStructure_.interleaver(j)[k]];
     // }
      parityIt += codeStructure_.structure(j).paritySize();
    }
  }
  auto parityIt = parityIn + codeStructure_.msgSize();
  std::copy(parityIn, parityIn + codeStructure_.msgSize(), messageOut);
  for (size_t i = 0; i < codeStructure_.structureCount(); ++i) {
    for (size_t k = 0; k < codeStructure_.structure(i).msgSize(); ++k) {
      extrinsicInterl[k] = extrinsicOut[codeStructure_.interleaver(i)[k]] + parityIn[codeStructure_.interleaver(i)[k]];
    }
    
    //extrinsicInterl.resize(codeStructure_.structure(i).msgSize(), 0);
    //codeStructure_.interleaver(i).interleaveBloc<LlrType>(extrinsicOut, extrinsicInterl.begin());
    code_[i]->appDecodeBloc(parityIt, extrinsicOut, msg.begin(), extrinsicOut);
    //codeStructure_.interleaver(i).deInterleaveBloc<LlrType>(extrinsicInterl.begin(), extrinsicOut);
    //std::fill(extrinsicInterl.begin(), extrinsicInterl.end(), 0);
    
    for (size_t k = 0; k < codeStructure_.structure(i).msgSize(); ++k) {
      messageOut[codeStructure_.interleaver(i)[k]] += extrinsicInterl[k];
      extrinsicOut[codeStructure_.interleaver(i)[k]] = extrinsicInterl[k];
    }
    parityIt += codeStructure_.structure(i).paritySize();
  }
}

void TurboCodeImpl::parallelDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut) const
{
  std::vector<LlrType> msg(codeStructure_.msgSize());
  auto extrinsicOutIt = extrinsicOut;
  auto extrinsicInIt = extrinsicIn;
  for (size_t i = 0; i < codeStructure_.structureCount(); ++i) {
    for (size_t k = 0; k < codeStructure_.structure(i).msgSize(); ++k) {
      extrinsicOutIt[k] = parityIn[codeStructure_.interleaver(i)[k]] + extrinsicInIt[k];
    }
    extrinsicOutIt += codeStructure_.structure(i).msgSize();
    extrinsicInIt += codeStructure_.structure(i).msgSize();
  }
  
  std::vector<LlrType> parityBuffer(extrinsicSize());
  //std::vector<LlrType> extrinsicInterl(codeStructure_.msgSize() , 0);
  for (size_t i = 0; i < codeStructure_.iterationCount(); ++i) {
    auto parityIt = parityIn + codeStructure_.msgSize();
    auto extrinsicOutIt = extrinsicOut;
    for (size_t j = 0; j < codeStructure_.structureCount(); ++j) {
      //extrinsicInterl.resize(codeStructure_.structure(j).msgSize(), 0);
      //codeStructure_.interleaver(j).interleaveBloc<LlrType>(extrinsicOutIt, extrinsicInterl.begin());
      code_[j]->appDecodeBloc(parityIt, extrinsicOutIt, msg.begin(), extrinsicOutIt);
      //codeStructure_.interleaver(j).deInterleaveBloc<LlrType>(extrinsicInterl.begin(), extrinsicOutIt);
      //std::fill(extrinsicInterl.begin(), extrinsicInterl.end(), 0);
      extrinsicOutIt += codeStructure_.structure(j).msgSize();
      parityIt += codeStructure_.structure(j).paritySize();
    }
    
    extrinsicOutIt = extrinsicOut;
    auto parityTmp = parityBuffer.begin();
    std::fill(messageOut, messageOut + codeStructure_.msgSize(), 0);
    for (size_t j = 0; j < codeStructure_.structureCount(); ++j) {
      for (size_t k = 0; k < codeStructure_.structure(j).msgSize(); ++k) {
        parityTmp[k] = extrinsicOutIt[k];
        extrinsicOutIt[k] = messageOut[codeStructure_.interleaver(j)[k]];
        messageOut[codeStructure_.interleaver(j)[k]] += parityTmp[k];
      }
      extrinsicOutIt += codeStructure_.structure(j).msgSize();
      parityTmp += codeStructure_.structure(j).msgSize();
    }
    std::copy(parityIn, parityIn + codeStructure_.msgSize(), messageOut);
    
    for (int64_t j = codeStructure_.structureCount()-1; j >= 0; --j) {
      extrinsicOutIt -= codeStructure_.structure(j).msgSize();
      parityTmp -= codeStructure_.structure(j).msgSize();
      for (size_t k = 0; k < codeStructure_.structure(j).msgSize(); ++k) {
        extrinsicOutIt[k] += messageOut[codeStructure_.interleaver(j)[k]];
        messageOut[codeStructure_.interleaver(j)[k]] += parityTmp[k];
      }
    }
  }
}
