/*******************************************************************************
 *  \file LdpcCode.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-12
 *  \version Last update : 2015-06-16
 *
 *  Definition of MapCode class
 ******************************************************************************/

#include "LdpcCode.h"

using namespace fec;

BOOST_CLASS_EXPORT_IMPLEMENT(LdpcCode);

const char * LdpcCode::get_key() const {
  return boost::serialization::type_info_implementation<LdpcCode>::type::get_const_instance().get_key();
}

/*******************************************************************************
 *  LdpcCode constructor
 *  \param  codeStructure Cpde structure used for encoding and decoding
 *  \param  type  Map decoder algorithm used for decoding
 *  \param  workGroupSize Number of thread used for decoding
 ******************************************************************************/
LdpcCode::LdpcCode(const LdpcCodeStructure& codeStructure, int workGroupdSize) :
  Code(workGroupdSize),
  codeStructure_(codeStructure)
{
}

void LdpcCode::syndrome(const std::vector<uint8_t>& parity, std::vector<uint8_t>& syndrome) const
{
  uint64_t blocCount = parity.size() / (codeStructure_.paritySize());
  if (parity.size() != blocCount * codeStructure_.paritySize()) {
    throw std::invalid_argument("Invalid size for message");
  }
  
  syndrome.resize(blocCount * codeStructure_.parityCheck().rows());
  
  auto syndromeIt = syndrome.begin();
  auto parityIt = parity.begin();
  
  for (size_t i = 0; i < blocCount; ++i) {
    codeStructure_.syndrome(parityIt, syndromeIt);
    parityIt += codeStructure_.paritySize();
    syndromeIt += codeStructure_.parityCheck().rows();
  }
}

void LdpcCode::encodeBloc(std::vector<uint8_t>::const_iterator messageIt, std::vector<uint8_t>::iterator parityIt) const
{
  codeStructure_.encode(messageIt, parityIt);
}

/*******************************************************************************
 *  Decodes several blocs of information bits.
 *  \param  codeIn  Vector containing extrinsic parity L-values
 *  \param  messageIn  Vector containing extrinsic information L-values
 *  \param  messageOut[out] Vector containing a posteriori information L-values
 ******************************************************************************/
void LdpcCode::parityAppDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const
{
  auto worker = BpDecoder::create(codeStructure_);
  worker->parityAppDecodeNBloc(parityIn, extrinsicIn, messageOut, extrinsicOut, n);
}

/*******************************************************************************
 *  Decodes several blocs of information bits.
 *  \param  codeIn  Vector containing extrinsic parity L-values
 *  \param  messageIn  Vector containing extrinsic information L-values
 *  \param  messageOut[out] Vector containing a posteriori information L-values
 ******************************************************************************/
void LdpcCode::appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const
{
  auto worker = BpDecoder::create(codeStructure_);
  worker->appDecodeNBloc(parityIn, extrinsicIn, messageOut, extrinsicOut, n);
}

/*******************************************************************************
 *  Decodes several blocs of information bits.
 *  \param  codeIn  Vector containing extrinsic parity L-values
 *  \param  messageIn  Vector containing extrinsic information L-values
 *  \param  messageOut[out] Vector containing a posteriori information L-values
 ******************************************************************************/
void LdpcCode::softOutDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut, size_t n) const
{
  auto worker = BpDecoder::create(codeStructure_);
  worker->softOutDecodeNBloc(parityIn, messageOut, n);
}

/*******************************************************************************
 *  Decodes several blocs of information bits.
 *  \param  codeIn  Vector containing extrinsic parity L-values
 *  \param  messageIn  Vector containing extrinsic information L-values
 *  \param  messageOut[out] Vector containing a posteriori information L-values
 ******************************************************************************/
void LdpcCode::decodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<uint8_t>::iterator messageOut, size_t n) const
{
  auto worker = BpDecoder::create(codeStructure_);
  std::vector<LlrType> messageAPosteriori(n * codeStructure_.msgSize());
  worker->softOutDecodeNBloc(parityIn, messageAPosteriori.begin(), n);
  
  for (auto messageIt = messageAPosteriori.begin(); messageIt < messageAPosteriori.end(); ++messageIt, ++messageOut) {
    *messageOut = *messageIt > 0;
  }
}
