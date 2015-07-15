/*******************************************************************************
 *  \file TurboCode.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-12
 *  \version Last update : 2015-05-27
 *
 *  Declaration of ConvolutionalCode class
 ******************************************************************************/

#ifndef TURBO_CODE_H
#define TURBO_CODE_H

#include <boost/serialization/export.hpp>
#include "../Archive.h"

#include "../ErrorCorrectingCode.h"
#include "../CodeStructure/TurboCodeStructure.h"
#include "../ConvolutionalCode/ConvolutionalCode.h"

/*******************************************************************************
 *  This class represents a map encode / decoder
 *  It offers methods encode and to decode data giving a posteriori informations 
 *  using a trellis.
 ******************************************************************************/
class TurboCode : public ErrorCorrectingCode
{
  friend class boost::serialization::access;
public:
  TurboCode(const TurboCodeStructure& codeStructure, int workGroupdSize = 4);
  virtual ~TurboCode() = default;
  
  virtual const char * get_key() const;
  
  virtual size_t msgSize() const {return codeStructure_.msgSize();}
  virtual size_t paritySize() const {return codeStructure_.paritySize();}
  virtual size_t extrinsicMsgSize() const {return codeStructure_.msgSize();}
  virtual size_t extrinsicParitySize() const {return codeStructure_.msgSize();}
  virtual const CodeStructure& structure() const {return codeStructure_;}
  
protected:
  TurboCode() = default;
  
  virtual void encodeBloc(std::vector<uint8_t>::const_iterator messageIt, std::vector<uint8_t>::iterator parityIt) const;
  
  virtual void parityAppDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const;
  virtual void appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const;
  virtual void softOutDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut, size_t n) const;
  virtual void decodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<uint8_t>::iterator messageOut, size_t n) const;

private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ErrorCorrectingCode);
    ar & BOOST_SERIALIZATION_NVP(code1_);
    ar & BOOST_SERIALIZATION_NVP(code2_);
    ar & BOOST_SERIALIZATION_NVP(codeStructure_);
  }
  
  template <typename T> void packParity(const std::vector<T>& parity1, const std::vector<T>& parity2, std::vector<T>& codeOut) const;
  template <typename T> void packParityNBloc(typename std::vector<T>::const_iterator parity1, typename std::vector<T>::const_iterator parity2, typename std::vector<T>::iterator parityOut, size_t n) const;
  template <typename T> void packParityBloc(typename std::vector<T>::const_iterator parity1, typename std::vector<T>::const_iterator parity2, typename std::vector<T>::iterator parityOut) const;
  
  void unpackParity(const std::vector<LlrType>& parityIn, std::vector<LlrType>& parity1, std::vector<LlrType>& parity2) const;
  void unpackParityNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator parity1, std::vector<LlrType>::iterator parity2, size_t n) const;
  void unpackParityBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator parity1, std::vector<LlrType>::iterator parity2) const;
  
  ConvolutionalCode code1_;
  ConvolutionalCode code2_;
  TurboCodeStructure codeStructure_;
};

template <typename T>
void TurboCode::packParity(const std::vector<T>& parity1, const std::vector<T>& parity2, std::vector<T>& parityOut) const
{
  auto parity1It = parity1.begin();
  auto parity2It = parity2.begin();
  for (auto parity = parityOut.begin(); parity < parityOut.end(); parity += codeStructure_.paritySize()) {
    packParityBloc(parity1It, parity2It, parity);
    parity1It += codeStructure_.structure1().paritySize();
    parity2It += codeStructure_.structure2().paritySize();
  }
}

template <typename T>
void TurboCode::packParityNBloc(typename std::vector<T>::const_iterator parity1, typename std::vector<T>::const_iterator parity2, typename std::vector<T>::iterator parityOut, size_t n) const
{
  for (size_t i = 0; i < n; ++i) {
    packParityBloc<T>(parity1, parity2, parityOut);
    parity1 += codeStructure_.structure1().paritySize();
    parity2 += codeStructure_.structure2().paritySize();
    parityOut += codeStructure_.paritySize();
  }
}

template <typename T>
void TurboCode::packParityBloc(typename std::vector<T>::const_iterator parity1, typename std::vector<T>::const_iterator parity2, typename std::vector<T>::iterator codeOut) const
{
  for (size_t j = 0; j < codeStructure_.structure2().blocSize(); ++j) {
    for (size_t k = 0; k < codeStructure_.structure1().trellis().outputSize(); ++k) {
      codeOut[k] = parity1[k];
    }
    codeOut += codeStructure_.structure1().trellis().outputSize();
    parity1 += codeStructure_.structure1().trellis().outputSize();
    for (size_t k = codeStructure_.structure2().trellis().inputSize(); k < codeStructure_.structure2().trellis().outputSize(); ++k) {
      codeOut[k-codeStructure_.structure2().trellis().inputSize()] = parity2[k];
    }
    codeOut += codeStructure_.structure2().trellis().outputSize() - codeStructure_.structure2().trellis().inputSize();
    parity2 += codeStructure_.structure2().trellis().outputSize();
  }
  for (size_t j = 0; j < codeStructure_.structure1().tailSize()*codeStructure_.structure1().trellis().outputSize(); ++j) {
    codeOut[j] = parity1[j];
  }
}

template <typename T>
void operator -=(std::vector<T>& a, const std::vector<T>& b) {
  if (a.size() != b.size()) {
    return;
  }
  auto bIt = b.begin();
  for (auto aIt = a.begin(); aIt < a.end(); ++aIt, ++bIt) {
    *aIt -= *bIt;
  }
}

BOOST_CLASS_EXPORT_KEY(TurboCode);
BOOST_CLASS_TYPE_INFO(TurboCode,extended_type_info_no_rtti<TurboCode>);

#endif
