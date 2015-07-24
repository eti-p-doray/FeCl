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

using namespace fec;

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

void MapDecoder::softOutDecodeNBloc(boost::container::vector<LlrType>::const_iterator parityIn, boost::container::vector<LlrType>::iterator messageOut, size_t n)
{
  for (size_t i = 0; i < n; ++i) {
    softOutDecodeBloc(parityIn,messageOut);
    parityIn += codeStructure_.paritySize();
    messageOut += codeStructure_.msgSize();
  }
}

void MapDecoder::appDecodeNBloc(boost::container::vector<LlrType>::const_iterator parityIn, boost::container::vector<LlrType>::const_iterator extrinsicIn, boost::container::vector<LlrType>::iterator messageOut, boost::container::vector<LlrType>::iterator extrinsicOut, size_t n)
{
  for (size_t i = 0; i < n; ++i) {
    appDecodeBloc(parityIn,extrinsicIn,messageOut,extrinsicOut);
    parityIn += codeStructure_.paritySize();
    extrinsicIn += codeStructure_.msgSize();
    messageOut += codeStructure_.msgSize();
    extrinsicOut += codeStructure_.msgSize();
  }
}

/*void MapDecoder::parityAppDecodeNBloc(boost::container::vector<LlrType>::const_iterator parityIn, boost::container::vector<LlrType>::const_iterator extrinsicIn, boost::container::vector<LlrType>::iterator messageOut, boost::container::vector<LlrType>::iterator extrinsicOut, size_t n)
{
  for (size_t i = 0; i < n; ++i) {
    parityAppDecodeBloc(parityIn,extrinsicIn,messageOut,extrinsicOut);
    parityIn += codeStructure_.paritySize();
    extrinsicIn += codeStructure_.paritySize();
    messageOut += codeStructure_.msgSize();
    extrinsicOut += codeStructure_.paritySize();
  }
}*/

void MapDecoder::softOutDecodeBloc(boost::container::vector<LlrType>::const_iterator parityIn, boost::container::vector<LlrType>::iterator messageOut)
{
  branchMetrics(parityIn);
  forwardMetrics();
  backwardMetrics();
  
  messageAPosteriori(messageOut);
}

void MapDecoder::appDecodeBloc(boost::container::vector<LlrType>::const_iterator parityIn, boost::container::vector<LlrType>::const_iterator extrinsicIn, boost::container::vector<LlrType>::iterator messageOut, boost::container::vector<LlrType>::iterator extrinsicOut)
{
  appBranchMetrics(parityIn, extrinsicIn);
  forwardMetrics();
  backwardMetrics();
  
  messageAPosteriori(messageOut);
  
  for (size_t i = 0; i < codeStructure().msgSize(); ++i) {
    extrinsicOut[i] = messageOut[i] - extrinsicIn[i];
  }
}

/*void MapDecoder::parityAppDecodeBloc(boost::container::vector<LlrType>::const_iterator parityIn, boost::container::vector<LlrType>::const_iterator extrinsicIn, boost::container::vector<LlrType>::iterator messageOut, boost::container::vector<LlrType>::iterator extrinsicOut)
{  
  parityAppBranchMetrics(parityIn, extrinsicIn);
  forwardMetrics();
  backwardMetrics();
  
  parityAPosteriori(extrinsicOut);
  messageAPosteriori(messageOut);
  
  for (size_t i = 0; i < codeStructure().paritySize(); ++i) {
    extrinsicOut[i] -= extrinsicIn[i];
  }
}*/

MapDecoder::MapDecoder(const ConvolutionalCodeStructure& codeStructure) : codeStructure_(codeStructure)
{
  branchMetrics_.resize((codeStructure.blocSize()+codeStructure.tailSize())*codeStructure.trellis().inputCount()*codeStructure.trellis().stateCount());
  forwardMetrics_.resize((codeStructure.blocSize()+codeStructure.tailSize())*codeStructure.trellis().stateCount());
  backwardMetrics_.resize((codeStructure.blocSize()+codeStructure.tailSize())*codeStructure.trellis().stateCount());
  
  branchOutputMetrics_.resize(codeStructure.trellis().outputCount());
  branchInputMetrics_.resize(codeStructure.trellis().inputCount());
}