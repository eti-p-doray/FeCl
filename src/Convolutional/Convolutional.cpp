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
 
 Definition of Convolutional class
 ******************************************************************************/

#include "Convolutional.h"
#include "MapDecoder/MapDecoder.h"
#include "ViterbiDecoder/ViterbiDecoder.h"

using namespace fec;

BOOST_CLASS_EXPORT_IMPLEMENT(Convolutional);
BOOST_CLASS_EXPORT_IMPLEMENT(Convolutional::Structure);

const char * Convolutional::get_key() const {
  return boost::serialization::type_info_implementation<Convolutional>::type::get_const_instance().get_key();
}

const char * Convolutional::Structure::get_key() const {
  return boost::serialization::type_info_implementation<Convolutional::Structure>::type::get_const_instance().get_key();
}

/**
 *  Constructor.
 *  \snippet Convolutional.cpp Creating a Convolutional code
 *  \param  structure Codec structure used for encoding and decoding
 *  \param  workGroupSize Number of thread used for decoding
 */
Convolutional::Convolutional(const Structure& structure,  int workGroupSize) :
structure_(structure),
Codec(&structure_, workGroupSize)
{
}
Convolutional::Convolutional(const EncoderOptions& encoder, const DecoderOptions& decoder, int workGroupSize) :
structure_(encoder, decoder),
Codec(&structure_, workGroupSize)
{
}
Convolutional::Convolutional(const EncoderOptions& encoder, int workGroupSize) :
structure_(encoder),
Codec(&structure_, workGroupSize)
{
}

void Convolutional::soDecodeBlocks(InputIterator input, OutputIterator output, size_t n) const
{
  auto worker = MapDecoder::create(structure());
  worker->soDecodeBlocks(input, output, n);
}

void Convolutional::decodeBlocks(std::vector<LlrType>::const_iterator parity, std::vector<BitField<bool>>::iterator msg, size_t n) const
{
  auto worker = ViterbiDecoder::create(structure());
  worker->decodeBlocks(parity, msg, n);
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
Convolutional::Structure::Structure(const EncoderOptions& encoder, const DecoderOptions& decoder)
{
  setEncoderOptions(encoder);
  setDecoderOptions(decoder);
}
Convolutional::Structure::Structure(const EncoderOptions& encoder)
{
  setEncoderOptions(encoder);
  setDecoderOptions(DecoderOptions());
}

void Convolutional::Structure::setEncoderOptions(const EncoderOptions& encoder)
{
  trellis_ = encoder.trellis_;
  length_ = encoder.length_;
  terminationType_ = encoder.terminationType_;
  
  msgSize_ = length_ * trellis_.inputSize();
  systSize_ = length_ * trellis_.inputSize();
  paritySize_ = length_ * trellis_.outputSize();
  stateSize_ = 0;
  switch (terminationType_) {
    case Tail:
      paritySize_ += trellis_.stateSize() * trellis_.outputSize();
      systSize_ += trellis_.stateSize() * trellis_.inputSize();
      tailSize_ = trellis_.stateSize();
      break;
      
    default:
    case Truncation:
      tailSize_ = 0;
      break;
  }
}

void Convolutional::Structure::setDecoderOptions(const DecoderOptions& decoder)
{
  decoderType_ = decoder.decoderType_;
}

Convolutional::DecoderOptions Convolutional::Structure::getDecoderOptions()
{
  return DecoderOptions().decoderType(decoderType_);
}

void Convolutional::Structure::encode(std::vector<BitField<bool>>::const_iterator msg, std::vector<BitField<uint8_t>>::iterator parity) const
{
  size_t state = 0;
  
  for (int j = 0; j < length(); ++j) {
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
  
  switch (terminationType()) {
    case Convolutional::Tail:
      for (int j = 0; j < tailSize(); ++j) {
        for (BitField<size_t> input = 0; input < trellis().inputCount(); ++input) {
          BitField<size_t> nextState = trellis().getNextState(state, input);
          if (nextState.test(trellis().stateSize()-1) == 0) {
            BitField<size_t> output = trellis().getOutput(state, input);
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
    case Convolutional::Truncation:
      state = 0;
      break;
  }
}

bool Convolutional::Structure::check(std::vector<BitField<uint8_t>>::const_iterator parity) const
{
  size_t state = 0;
  for (int j = 0; j < length()+tailSize(); ++j) {
    bool found = false;
    for (BitField<size_t> input = 0; input < trellis().inputCount(); ++input) {
      BitField<size_t> output = trellis().getOutput(state, input);
      bool equal = true;
      for (int k = 0; k < trellis().outputSize(); ++k) {
        if (output[k] != parity[k]) {
          equal = false;
          break;
        }
      }
      if (equal == true) {
        found = true;
        state = trellis().getNextState(state, input);
        break;
      }
    }
    if (found == false) {
      return false;
    }
    parity += trellis().outputSize();
  }
  switch (terminationType()) {
    case Convolutional::Tail:
      return (state == 0);
      
    default:
    case Convolutional::Truncation:
      return true;
  }
}

void Convolutional::Structure::encode(std::vector<BitField<bool>>::const_iterator msg, std::vector<BitField<uint8_t>>::iterator parity, std::vector<BitField<uint8_t>>::iterator tail) const
{
  size_t state = 0;
  
  for (int j = 0; j < length(); ++j) {
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
  
  switch (terminationType()) {
    case Convolutional::Tail:
      for (int j = 0; j < tailSize(); ++j) {
        for (BitField<size_t> input = 0; input < trellis().inputCount(); ++input) {
          BitField<size_t> nextState = trellis().getNextState(state, input);
          if (nextState.test(trellis().stateSize()-1) == 0) {
            BitField<size_t> output = trellis().getOutput(state, input);
            for (int k = 0; k < trellis().inputSize(); ++k) {
              tail[k] = input.test(k);
            }
            for (int k = 0; k < trellis().outputSize(); ++k) {
              parity[k] = output.test(k);
            }
            parity += trellis().outputSize();
            tail += trellis().inputSize();
            state = nextState;
            break;
          }
        }
      }
      break;
      
    default:
    case Convolutional::Truncation:
      state = 0;
      break;
  }
}
