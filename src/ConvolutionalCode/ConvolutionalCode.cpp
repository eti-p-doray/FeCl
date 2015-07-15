/*******************************************************************************
 *  \file MapCode.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-12
 *  \version Last update : 2015-05-27
 *
 *  Definition of MapCode class
 ******************************************************************************/

#include "ConvolutionalCode.h"

BOOST_CLASS_EXPORT_IMPLEMENT(ConvolutionalCode);

const char * ConvolutionalCode::get_key() const {
  return boost::serialization::type_info_implementation<ConvolutionalCode>::type::get_const_instance().get_key();
}

/*******************************************************************************
 *  ConvolutionalCode constructor
 *  \param  codeStructure Cpde structure used for encoding and decoding
 *  \param  type  Map decoder algorithm used for decoding
 *  \param  workGroupSize Number of thread used for decoding
 ******************************************************************************/
ConvolutionalCode::ConvolutionalCode(const ConvolutionalCodeStructure& codeStructure, int workGroupdSize) :
  ErrorCorrectingCode(workGroupdSize),
  codeStructure_(codeStructure)
{
}

void ConvolutionalCode::encodeBloc(std::vector<uint8_t>::const_iterator messageIt, std::vector<uint8_t>::iterator parityIt) const
{
  uint16_t state = 0;
  
  for (int j = 0; j < codeStructure_.blocSize(); j++) {
    BitField<uint16_t> input = 0;
    for (int k = 0; k < codeStructure_.trellis().inputSize(); k++) {
      input[k] = messageIt[k];
    }
    messageIt += codeStructure_.trellis().inputSize();
    
    BitField<uint16_t> output = codeStructure_.trellis().getOutput(state, input);
    state = codeStructure_.trellis().getNextState(state, input);
    
    for (int k = 0; k < codeStructure_.trellis().outputSize(); k++) {
      parityIt[k] = output.test(k);
    }
    parityIt += codeStructure_.trellis().outputSize();
  }
  
  switch (codeStructure_.endType()) {
    case ConvolutionalCodeStructure::ZeroTail:
      for (int j = 0; j < codeStructure_.tailSize(); j++) {
        for (BitField<uint16_t> input = 0; input < codeStructure_.trellis().inputSize(); input++) {
          BitField<uint16_t> nextState = codeStructure_.trellis().getNextState(state, input);
          if (nextState.test(codeStructure_.trellis().stateSize()-1) == 0) {
            BitField<uint16_t> output = codeStructure_.trellis().getOutput(state, 0);
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

void ConvolutionalCode::parityAppDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const
{
  auto worker = MapDecoder::create(codeStructure_);
  worker->parityAppDecodeNBloc(parityIn, extrinsicIn, messageOut, extrinsicOut, n);
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
