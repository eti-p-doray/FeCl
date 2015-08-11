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
 
 Declaration of MapDecoder abstract class
 ******************************************************************************/

#include "MapDecoder.h"
#include "MaxLogMap.h"
#include "LogMap.h"
#include "MapDecoderImpl.h"

using namespace fec;

/**
 *  MapDecoder creator function.
 *  Construct in a factory behavior a MapCode object corresponding to the algorithm
 *  version in use.
 *  \param  codeStructure Convolutional code structure describing the code
 *  \return MacDecoder specialization suitable for the algorithm in use
 */
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

/**
 *  Implementation of Code#softOutDecodeNBloc.
 */
void MapDecoder::softOutDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut, size_t n)
{
  for (size_t i = 0; i < n; ++i) {
    softOutDecodeBloc(parityIn,messageOut);
    parityIn += codeStructure_.paritySize();
    messageOut += codeStructure_.msgSize();
  }
}

/**
 *  Implementation of Code#appDecodeNBloc.
 */
void MapDecoder::appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n)
{
  for (size_t i = 0; i < n; ++i) {
    appDecodeBloc(parityIn,extrinsicIn,messageOut,extrinsicOut);
    parityIn += codeStructure_.paritySize();
    extrinsicIn += codeStructure_.msgSize();
    messageOut += codeStructure_.msgSize();
    extrinsicOut += codeStructure_.msgSize();
  }
}

/**
 *  Decodes one blocs of information bits.
 *  A posteriori information about the msg is output instead of the decoded bit sequence.
 *  \param  parityIn  Input iterator pointing to the first element
 *    in the parity L-value sequence
 *  \param  messageOut[out] Output iterator pointing to the first element
 *    in the a posteriori information L-value sequence.
 *    Output needs to be pre-allocated.
 */
void MapDecoder::softOutDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut)
{
  branchMetrics(parityIn);
  forwardMetrics();
  backwardMetrics();
  
  messageAPosteriori(messageOut);
}

/**
 *  Decodes one blocs of information bits.
 *  A priori information about the decoder state is provided and extrinsic
 *  information is output, following the same structure. These informations
 *  can be transfered sequencially to multiple decoding attempts.
 *  \param  parityIn  Input iterator pointing to the first element
 *    in the parity L-value sequence
 *  \param  extrinsicIn  Input iterator pointing to the first element
 *    in the a-priori extrinsic L-value sequence
 *  \param  messageOut[out] Output iterator pointing to the first element
 *    in the a posteriori information L-value sequence.
 *    Output needs to be pre-allocated.
 *  \param  extrinsicOut[out]  Output iterator pointing to the first element
 *    in the extrinsic L-value sequence.
 *    Output needs to be pre-allocated.
 */
void MapDecoder::appDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut)
{
  appBranchMetrics(parityIn, extrinsicIn);
  forwardMetrics();
  backwardMetrics();
  
  messageAPosteriori(messageOut);
  
  for (size_t i = 0; i < codeStructure().msgSize(); ++i) {
    extrinsicOut[i] = messageOut[i] - extrinsicIn[i];
  }
}

/**
 *  Constructor.
 *  Allocates metric buffers based on the given code structure.
 *  \param  codeStructure Convolutional code structure describing the code
 */
MapDecoder::MapDecoder(const ConvolutionalCodeStructure& codeStructure) : codeStructure_(codeStructure)
{
  branchMetrics_.resize((codeStructure.blocSize()+codeStructure.tailSize())*codeStructure.trellis().inputCount()*codeStructure.trellis().stateCount());
  forwardMetrics_.resize((codeStructure.blocSize()+codeStructure.tailSize())*codeStructure.trellis().stateCount());
  backwardMetrics_.resize((codeStructure.blocSize()+codeStructure.tailSize())*codeStructure.trellis().stateCount());
  
  branchOutputMetrics_.resize(codeStructure.trellis().outputCount());
  branchInputMetrics_.resize(codeStructure.trellis().inputCount());
}
