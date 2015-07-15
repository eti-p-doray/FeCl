/*******************************************************************************
 *  \file ConvolutionalCode.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-12
 *  \version Last update : 2015-05-27
 *
 *  Declaration of ConvolutionalCode class
 ******************************************************************************/

#ifndef CONVOLUTIONAL_CODE_H
#define CONVOLUTIONAL_CODE_H

#include <thread>

#include "../Archive.h"
#include <boost/serialization/export.hpp>

#include "../ErrorCorrectingCode.h"
#include "../CodeStructure/ConvolutionalCodeStructure.h"
#include "MapDecoder/MapDecoder.h"
#include "ViterbiDecoder/ViterbiDecoder.h"

/*******************************************************************************
 *  This class represents a map encode / decoder
 *  It offers methods encode and to decode data giving a posteriori informations 
 *  using a trellis.
 ******************************************************************************/
class ConvolutionalCode : public ErrorCorrectingCode
{
  friend class TurboCode;
  friend class boost::serialization::access;
public:
  ConvolutionalCode(const ConvolutionalCodeStructure& codeStructure, int workGroupdSize = 4);
  virtual ~ConvolutionalCode() = default;
  
  virtual const char * get_key() const;
  
  virtual size_t msgSize() const {return codeStructure_.msgSize();}
  virtual size_t paritySize() const {return codeStructure_.paritySize();}
  virtual size_t extrinsicMsgSize() const {return codeStructure_.msgSize();}
  virtual size_t extrinsicParitySize() const {return codeStructure_.paritySize();}
  virtual const CodeStructure& structure() const {return codeStructure_;}

protected:
  ConvolutionalCode() = default;
  
  virtual void encodeBloc(std::vector<uint8_t>::const_iterator messageIt, std::vector<uint8_t>::iterator parityIt) const;
  
  virtual void parityAppDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const;
  virtual void appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const;
  virtual void softOutDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut, size_t n) const;
  virtual void decodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<uint8_t>::iterator messageOut, size_t n) const;
  
  template <typename T> void parityToMsgNBloc(typename std::vector<T>::const_iterator parity, typename std::vector<T>::iterator msg, size_t n) const;
  template <typename T> void msgToParityNBloc(typename std::vector<T>::const_iterator msg, typename std::vector<T>::iterator parity, size_t n) const;
  template <typename T> void parityToMsgBloc(typename std::vector<T>::const_iterator parity, typename std::vector<T>::iterator msg) const;
  template <typename T> void msgToParityBloc(typename std::vector<T>::const_iterator msg, typename std::vector<T>::iterator parity) const;
  
private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ErrorCorrectingCode);
    ar & BOOST_SERIALIZATION_NVP(codeStructure_);
  }
  ConvolutionalCodeStructure codeStructure_;
};

BOOST_CLASS_TYPE_INFO(ConvolutionalCode,extended_type_info_no_rtti<ConvolutionalCode>);
BOOST_CLASS_EXPORT_KEY(ConvolutionalCode);

template <typename T>
void ConvolutionalCode::parityToMsgNBloc(typename std::vector<T>::const_iterator parity, typename std::vector<T>::iterator msg, size_t n) const
{
  for (size_t i = 0; i < n; ++i) {
    parityToMsgBloc<T>(parity, msg);
    parity += codeStructure_.paritySize();
    msg += codeStructure_.msgSize();
  }
}

template <typename T>
void ConvolutionalCode::msgToParityNBloc(typename std::vector<T>::const_iterator parity, typename std::vector<T>::iterator msg, size_t n) const
{
  for (size_t i = 0; i < n; ++i) {
    msgToParityBloc<T>(parity, msg);
    parity += codeStructure_.paritySize();
    msg += codeStructure_.msgSize();
  }
}

template <typename T>
void ConvolutionalCode::parityToMsgBloc(typename std::vector<T>::const_iterator parity, typename std::vector<T>::iterator msg) const
{
  for (size_t i = 0; i < codeStructure_.blocSize(); ++i) {
    for (size_t j = 0; j < codeStructure_.trellis().inputSize(); ++j) {
      msg[j] = parity[j];
    }
    msg += codeStructure_.trellis().inputSize();
    parity += codeStructure_.trellis().outputSize();
  }
}

template <typename T>
void ConvolutionalCode::msgToParityBloc(typename std::vector<T>::const_iterator msg, typename std::vector<T>::iterator parity) const
{
  for (size_t i = 0; i < codeStructure_.blocSize(); ++i) {
    for (size_t j = 0; j < codeStructure_.trellis().inputSize(); ++j) {
      parity[j] = msg[j];
    }
    for (size_t j = codeStructure_.trellis().inputSize(); j < codeStructure_.trellis().outputSize(); ++j) {
      parity[j] = T(0);
    }
    msg += codeStructure_.trellis().inputSize();
    parity += codeStructure_.trellis().outputSize();
  }
}

#endif
