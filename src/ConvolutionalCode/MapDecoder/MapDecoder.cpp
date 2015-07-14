/*******************************************************************************
 *  \file MapDecoder.cpp
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-27
 *  \version Last update : 2015-05-27
 *
 *  Declaration of MapDecoder abstract class
 ******************************************************************************/

#include "MapDecoder.h"
#include "MaxLogMap.h"
#include "LogMap.h"
#include "MapDecoderImpl.h"

std::unique_ptr<MapDecoder> MapDecoder::create(const ConvolutionalCodeStructure& codeStructure)
{
  switch (codeStructure.decoderType()) {
    default:
    case ConvolutionalCodeStructure::MaxLogMap:
      return std::unique_ptr<MapDecoder>(new MapDecoderImpl<MaxLogMap>(codeStructure));
      break;
      
    case ConvolutionalCodeStructure::LogMap:
      return std::unique_ptr<MapDecoder>(new MapDecoderImpl<LogMap>(codeStructure));
      break;
  }
}

void MapDecoder::softOutDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut, size_t n)
{
  for (size_t i = 0; i < n; i++) {
    softOutDecodeBloc(parityIn,messageOut);
    parityIn += codeStructure_.paritySize();
    messageOut += codeStructure_.messageSize();
  }
}

void MapDecoder::appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n)
{
  for (size_t i = 0; i < n; i++) {
    appDecodeBloc(parityIn,extrinsicIn,messageOut,extrinsicOut);
    parityIn += codeStructure_.paritySize();
    extrinsicIn += codeStructure_.messageSize();
    messageOut += codeStructure_.messageSize();
    extrinsicOut += codeStructure_.messageSize();
  }
}

void MapDecoder::parityAppDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator parityOut, std::vector<LlrType>::iterator extrinsicOut, size_t n)
{
  for (size_t i = 0; i < n; i++) {
    parityAppDecodeBloc(parityIn,extrinsicIn,parityOut,extrinsicOut);
    parityIn += codeStructure_.paritySize();
    extrinsicIn += codeStructure_.messageSize();
    parityOut += codeStructure_.paritySize();
    extrinsicOut += codeStructure_.messageSize();
  }
}

void MapDecoder::softOutDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut)
{
  branchMetrics(parityIn);
  forwardMetrics();
  backwardMetrics();
  
  messageAPosteriori(messageOut);
}

void MapDecoder::appDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut)
{
  branchMetrics(parityIn, extrinsicIn);
  forwardMetrics();
  backwardMetrics();
  
  messageAPosteriori(messageOut);
  
  std::copy(messageOut, messageOut+codeStructure().messageSize(), extrinsicOut);
  for (size_t i = 0; i < codeStructure().messageSize(); ++i) {
    extrinsicOut[i] -= extrinsicIn[i];
  }
}

void MapDecoder::parityAppDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator parityOut, std::vector<LlrType>::iterator extrinsicOut)
{
  branchMetrics(parityIn, extrinsicIn);
  forwardMetrics();
  backwardMetrics();
  
  parityAPosteriori(parityOut);
  
  std::copy(parityOut, parityOut+codeStructure().messageSize(), extrinsicOut);
  for (size_t i = 0; i < codeStructure().messageSize(); ++i) {
    extrinsicOut[i] -= extrinsicIn[i];
  }
}

MapDecoder::MapDecoder(const ConvolutionalCodeStructure& codeStructure) : codeStructure_(codeStructure)
{
  branchMetrics_.resize((codeStructure.blocSize()+codeStructure.tailSize())*codeStructure.trellis().inputCount()*codeStructure.trellis().stateCount());
  forwardMetrics_.resize((codeStructure.blocSize()+codeStructure.tailSize())*codeStructure.trellis().stateCount());
  backwardMetrics_.resize((codeStructure.blocSize()+codeStructure.tailSize())*codeStructure.trellis().stateCount());
  
  branchOutputMetrics_.resize(codeStructure.trellis().outputCount());
  branchInputMetrics_.resize(codeStructure.trellis().inputCount());
}
