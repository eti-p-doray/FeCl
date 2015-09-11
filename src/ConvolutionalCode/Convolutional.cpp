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
#include "MapDecoder/MapDecoder.h"
#include "ViterbiDecoder/ViterbiDecoder.h"

using namespace fec;

BOOST_CLASS_EXPORT_IMPLEMENT(ConvolutionalCode);
BOOST_CLASS_EXPORT_IMPLEMENT(ConvolutionalCode::Structure);

const char * ConvolutionalCode::get_key() const {
  return boost::serialization::type_info_implementation<ConvolutionalCode>::type::get_const_instance().get_key();
}

const char * ConvolutionalCode::Structure::get_key() const {
  return boost::serialization::type_info_implementation<ConvolutionalCode::Structure>::type::get_const_instance().get_key();
}

/**
 *  Constructor.
 *  \snippet Convolutional.cpp Creating a Convolutional code
 *  \param  structure Code structure used for encoding and decoding
 *  \param  workGroupSize Number of thread used for decoding
 */
ConvolutionalCode::ConvolutionalCode(const ConvolutionalCode::Structure& structure, int workGroupSize) :
structure_(structure),
Code(&structure_, workGroupSize)
{
}

void ConvolutionalCode::encodeBloc(std::vector<uint8_t>::const_iterator messageIt, std::vector<uint8_t>::iterator parityIt) const
{
  structure_.encode(messageIt, parityIt);
}

void ConvolutionalCode::appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const
{
  auto worker = MapDecoder::create(structure_);
  worker->appDecodeNBloc(parityIn, extrinsicIn, messageOut, extrinsicOut, n);
}

void ConvolutionalCode::softOutDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut, size_t n) const
{
  auto worker = MapDecoder::create(structure_);
  worker->softOutDecodeNBloc(parityIn, messageOut, n);
}

void ConvolutionalCode::decodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<uint8_t>::iterator messageOut, size_t n) const
{
  auto worker = std::unique_ptr<ViterbiDecoder>(new ViterbiDecoder(structure_));
  worker->decodeNBloc(parityIn, messageOut, n);
}


/**
 *  Constructor.
 *  Constructs an ldpc code structure following a trellis structure.
 *  \snippet Convolutional.cpp Creating a Convolutional code structure
 *  \param  trellis Trellis structure to follow
 *  \param  blocSize  Number of branch in one bloc before being terminated
 *  \param  endType Trellis termination type
 *  \param  type  Algorithm use in app decoding
 */
ConvolutionalCode::Structure::Structure(Trellis trellis, size_t blocSize, TerminationType terminationType, DecoderType type) : Code::Structure(blocSize * trellis.inputSize(), blocSize * trellis.outputSize(), blocSize * trellis.inputSize()), trellis_(trellis)
{
  blocSize_ = blocSize;
  terminationType_ = terminationType;
  decoderType_ = type;
  
  switch (terminationType_) {
    case Tail:
      paritySize_ += trellis_.stateSize() * trellis.outputSize();
      tailSize_ = trellis_.stateSize();
      break;
      
    default:
    case Truncation:
      tailSize_ = 0;
      break;
  }
}

BitField<uint64_t> ConvolutionalCode::Structure::encode(std::vector<uint8_t>::const_iterator msg, std::vector<uint8_t>::iterator parity) const
{
  size_t state = 0;
  
  for (int j = 0; j < blocSize(); ++j) {
    BitField<size_t> input = 0;
    for (int k = 0; k < trellis().inputSize(); k++) {
      input.set(k, msg[k]);
    }
    msg += trellis().inputSize();
    
    BitField<size_t> output = trellis().getOutput(state, input);
    state = trellis().getNextState(state, input);
    
    for (int k = 0; k < trellis().outputSize(); k++) {
      parity[k] = output.test(k);
    }
    parity  += trellis().outputSize();
  }
  
  BitField<uint64_t> tail = 0;
  switch (terminationType()) {
    case ConvolutionalCode::Tail:
      for (int j = 0; j < tailSize(); ++j) {
        for (BitField<size_t> input = 0; input < trellis().inputCount(); ++input) {
          BitField<size_t> nextState = trellis().getNextState(state, input);
          if (nextState.test(trellis().stateSize()-1) == 0) {
            BitField<size_t> output = trellis().getOutput(state, input);
            for (int k = 0; k < trellis().inputSize(); ++k) {
              tail.set(j*trellis().inputSize()+k, input.test(k));
            }
            for (int k = 0; k < trellis().outputSize(); ++k) {
              parity[k] = output.test(k);
            }
            parity += trellis().outputSize();
            state = nextState;
            break;
          }
        }
      }
      break;
      
    default:
    case ConvolutionalCode::Truncation:
      state = 0;
      break;
  }
  return tail;
}
