/*******************************************************************************
 *  \file TurboCode.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-06-17
 *  \version Last update : 2015-06-17
 *
 *  Definition of MapCode class
 ******************************************************************************/

#include "TurboCode.h"

BOOST_CLASS_EXPORT_IMPLEMENT(TurboCode);

const char * TurboCode::get_key() const {
  return boost::serialization::type_info_implementation<TurboCode>::type::get_const_instance().get_key();
}

/*******************************************************************************
 *  TurboCode constructor
 *  \param  codeStructure Cpde structure used for encoding and decoding
 *  \param  type  Map decoder algorithm used for decoding
 *  \param  workGroupSize Number of thread used for decoding
 ******************************************************************************/
TurboCode::TurboCode(const TurboCodeStructure& codeStructure, int workGroupdSize) :
  ErrorCorrectingCode(workGroupdSize),
  codeStructure_(codeStructure),
  code1_(codeStructure.structure1(), workGroupdSize),
  code2_(codeStructure.structure2(), workGroupdSize)
{
}

void TurboCode::encodeBloc(std::vector<uint8_t>::const_iterator messageIt, std::vector<uint8_t>::iterator parityIt) const
{
  std::vector<uint8_t> messageInter(codeStructure_.msgSize());
  codeStructure_.interleaver().interleaveBloc<uint8_t>(messageIt, messageInter.begin());
  std::vector<uint8_t> parity1(codeStructure_.structure1().paritySize());
  std::vector<uint8_t> parity2(codeStructure_.structure2().paritySize());
  
  code1_.encodeBloc(messageIt, parity1.begin());
  code2_.encodeBloc(messageInter.begin(), parity2.begin());
  
  packParityBloc<uint8_t>(parity1.begin(), parity2.begin(), parityIt);
}

void TurboCode::parityAppDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const
{
  appDecodeNBloc(parityIn, extrinsicIn, messageOut, extrinsicOut, n);
}

void TurboCode::appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const
{
  std::vector<LlrType> parity1(n * codeStructure_.structure1().paritySize());
  std::vector<LlrType> parity2(n * codeStructure_.structure2().paritySize());
  unpackParityNBloc(parityIn, parity1.begin(), parity2.begin(), n);
  
  std::vector<LlrType> msgEx1(n * codeStructure_.structure2().msgSize(), 0);
  std::vector<LlrType> msgEx2(n * codeStructure_.structure2().msgSize(), 0);
  
  code1_.appDecodeNBloc(parity1.begin(), extrinsicIn, messageOut, msgEx1.begin(), n);
  codeStructure_.interleaver().interleaveNBloc<LlrType>(msgEx1.begin(), msgEx2.begin(), n);
  for (size_t i = 0; i < codeStructure_.iterationCount()-1; ++i) {
    code2_.appDecodeNBloc(parity2.begin(), msgEx2.begin(), messageOut, msgEx2.begin(), n);
    codeStructure_.interleaver().deInterleaveNBloc<LlrType>(msgEx2.begin(), msgEx1.begin(), n);
    
    code1_.appDecodeNBloc(parity1.begin(), msgEx1.begin(), messageOut, msgEx1.begin(), n);
    codeStructure_.interleaver().interleaveNBloc<LlrType>(msgEx1.begin(), msgEx2.begin(), n);
  }
  code2_.appDecodeNBloc(parity2.begin(), msgEx2.begin(), msgEx1.begin(), msgEx2.begin(), n);
  codeStructure_.interleaver().deInterleaveNBloc<LlrType>(msgEx1.begin(), messageOut, n);
  codeStructure_.interleaver().deInterleaveNBloc<LlrType>(msgEx2.begin(), extrinsicOut, n);
}

void TurboCode::softOutDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut, size_t n) const
{
  std::vector<LlrType> extrinsicIn(n * codeStructure_.msgSize(), 0);
  std::vector<LlrType> extrinsicOut(n * codeStructure_.msgSize());
  appDecodeNBloc(parityIn, extrinsicIn.begin(), messageOut, extrinsicOut.begin(), n);
}

void TurboCode::decodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<uint8_t>::iterator messageOut, size_t n) const
{
  std::vector<LlrType> messageAPosteriori(n * codeStructure_.msgSize());
  softOutDecodeNBloc(parityIn, messageAPosteriori.begin(), n);

  for (auto messageIt = messageAPosteriori.begin(); messageIt < messageAPosteriori.end(); ++messageIt, ++messageOut) {
    *messageOut = *messageIt > 0;
  }
}

void TurboCode::unpackParity(const std::vector<LlrType>& parityIn, std::vector<LlrType>& parity1, std::vector<LlrType>& parity2) const
{
  auto parity1It = parity1.begin();
  auto parity2It = parity2.begin();
  for (auto codeIt = parityIn.begin(); codeIt < parityIn.end(); codeIt += codeStructure_.paritySize()) {
    unpackParityBloc(codeIt, parity1It, parity2It);
    parity1It += codeStructure_.structure1().paritySize();
    parity2It += codeStructure_.structure2().paritySize();
  }
}

void TurboCode::unpackParityNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator parity1, std::vector<LlrType>::iterator parity2, size_t n) const
{
  for (size_t i = 0; i < n; ++i) {
    unpackParityBloc(parityIn, parity1, parity2);
    parity1 += codeStructure_.structure1().paritySize();
    parity2 += codeStructure_.structure2().paritySize();
    parityIn += codeStructure_.paritySize();
  }
}

void TurboCode::unpackParityBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator parity1, std::vector<LlrType>::iterator parity2) const
{
  std::vector<LlrType> paritySyst(codeStructure_.msgSize());
  
  for (auto paritySystIt = paritySyst.begin(); paritySystIt < paritySyst.end(); ++paritySystIt) {
    *paritySystIt = *parityIn;
    for (size_t k = 0; k < codeStructure_.structure1().trellis().outputSize(); ++k) {
      parity1[k] = parityIn[k];
    }
    parityIn += codeStructure_.structure1().trellis().outputSize();
    parity1 += codeStructure_.structure1().trellis().outputSize();
    
    for (size_t k = codeStructure_.structure2().trellis().inputSize(); k < codeStructure_.structure2().trellis().outputSize(); ++k) {
      parity2[k] = parityIn[k-codeStructure_.structure2().trellis().inputSize()];
    }
    parityIn += codeStructure_.structure2().trellis().outputSize() - codeStructure_.structure2().trellis().inputSize();
    parity2 += codeStructure_.structure2().trellis().outputSize();
  }
  for (size_t j = 0; j < codeStructure_.structure1().tailSize()*codeStructure_.structure1().trellis().outputSize(); ++j) {
    parity1[j] = parityIn[j];
  }
  
  paritySyst = codeStructure_.interleaver().interleave(paritySyst);
  parity2 -= codeStructure_.structure2().paritySize();
  for (auto paritySystIt = paritySyst.cbegin(); paritySystIt < paritySyst.end(); ++paritySystIt) {
    for (size_t k = 0; k < codeStructure_.structure2().trellis().inputSize(); ++k) {
      parity2[k] = paritySystIt[k];
    }
    parity2 += codeStructure_.structure2().trellis().outputSize();
  }
  
}
