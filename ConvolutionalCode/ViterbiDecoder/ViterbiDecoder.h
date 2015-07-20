/*******************************************************************************
 *  \file ViterbiDecoder.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-06-11
 *  \version Last update : 2015-06-11
 *
 *  Declaration of ViterbiDecoder class
 ******************************************************************************/

#ifndef VITERBI_DECODER_H
#define VITERBI_DECODER_H

#include <vector>
#include <memory>

#include "../../CodeStructure/ConvolutionalCodeStructure.h"

namespace fec {

/*******************************************************************************
 *  This class represents a map decoder
 *  It offers methods to decode data giving a posteriori informations
 *  using a trellis.
 ******************************************************************************/
class ViterbiDecoder
{
public:
  ViterbiDecoder(const ConvolutionalCodeStructure& codeStructure);
  ~ViterbiDecoder() = default;
  
  void decodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<uint8_t>::iterator messageOut, size_t n);
  void decodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<uint8_t>::iterator messageOut);
  
protected:
  
  inline const ConvolutionalCodeStructure& codeStructure() const {return codeStructure_;}
  
  std::vector<LlrType> previousPathMetrics;
  std::vector<LlrType> nextPathMetrics;
  std::vector<LlrType> branchMetrics;
  std::vector<BitField<uint16_t>> stateTraceBack;
  std::vector<BitField<uint16_t>> inputTraceBack;

private:
  
  const ConvolutionalCodeStructure codeStructure_;
};
  
}

#endif
