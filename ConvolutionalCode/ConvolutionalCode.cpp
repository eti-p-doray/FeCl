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

#include "ConvolutionalCode.h"

using namespace fec;

BOOST_CLASS_EXPORT_IMPLEMENT(ConvolutionalCode);

const char * ConvolutionalCode::get_key() const {
  return boost::serialization::type_info_implementation<ConvolutionalCode>::type::get_const_instance().get_key();
}

/**
 *  ConvolutionalCode constructor
 *  \param  codeStructure Code structure used for encoding and decoding
 *  \param  workGroupSize Number of thread used for decoding
 */
ConvolutionalCode::ConvolutionalCode(const ConvolutionalCodeStructure& codeStructure, int workGroupSize) :
  Code(workGroupSize),
  codeStructure_(codeStructure)
{
}

void ConvolutionalCode::encodeBloc(std::vector<uint8_t>::const_iterator messageIt, std::vector<uint8_t>::iterator parityIt) const
{
  size_t state = 0;
  
  for (int j = 0; j < codeStructure_.blocSize(); j++) {
    BitField<size_t> input = 0;
    for (int k = 0; k < codeStructure_.trellis().inputSize(); k++) {
      input[k] = messageIt[k];
    }
    messageIt += codeStructure_.trellis().inputSize();
    
    BitField<size_t> output = codeStructure_.trellis().getOutput(state, input);
    state = codeStructure_.trellis().getNextState(state, input);
    
    for (int k = 0; k < codeStructure_.trellis().outputSize(); k++) {
      parityIt[k] = output.test(k);
    }
    parityIt += codeStructure_.trellis().outputSize();
  }
  
  switch (codeStructure_.endType()) {
    case ConvolutionalCodeStructure::ZeroTail:
      for (int j = 0; j < codeStructure_.tailSize(); j++) {
        for (BitField<size_t> input = 0; input < codeStructure_.trellis().inputSize(); input++) {
          BitField<size_t> nextState = codeStructure_.trellis().getNextState(state, input);
          if (nextState.test(codeStructure_.trellis().stateSize()-1) == 0) {
            BitField<size_t> output = codeStructure_.trellis().getOutput(state, 0);
            for (int k = 0; k < codeStructure_.trellis().outputSize(); k++) {
              parityIt[k] = output.test(k);
            }
            parityIt += codeStructure_.trellis().outputSize();
            state = nextState;
            continue;
          }
        }
      }
      break;
      
    default:
    case ConvolutionalCodeStructure::Truncation:
      state = 0;
      break;
  }
}

void ConvolutionalCode::appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const
{
  auto worker = MapDecoder::create(codeStructure_);
  worker->appDecodeNBloc(parityIn, extrinsicIn, messageOut, extrinsicOut, n);
}

void ConvolutionalCode::softOutDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut, size_t n) const
{
  auto worker = MapDecoder::create(codeStructure_);
  worker->softOutDecodeNBloc(parityIn, messageOut, n);
}

void ConvolutionalCode::decodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<uint8_t>::iterator messageOut, size_t n) const
{
  auto worker = std::unique_ptr<ViterbiDecoder>(new ViterbiDecoder(codeStructure_));
  worker->decodeNBloc(parityIn, messageOut, n);
}
