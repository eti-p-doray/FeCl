/*******************************************************************************
 *  \file ErrorCorrectingCode.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-06-11
 *  \version Last update : 2015-06-11
 *
 *  Declaration of ErrorCorrectingCode class
 ******************************************************************************/

#ifndef ERROR_CORRECTING_CODE_H
#define ERROR_CORRECTING_CODE_H

#include <memory>
#include <thread>

#include "Archive.h"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/serialization/extended_type_info_no_rtti.hpp>

#include "CodeStructure/CodeStructure.h"

/*******************************************************************************
 *  This class represents a map encode / decoder
 *  It offers methods encode and to decode data giving a posteriori informations
 *  using a trellis.
 ******************************************************************************/
class ErrorCorrectingCode
{
  friend class boost::serialization::access;
public:
  static std::unique_ptr<ErrorCorrectingCode> create(const CodeStructure& codeStructure, int workGroupdSize = 4);
  virtual ~ErrorCorrectingCode() = default;
  
  virtual const char * get_key() const = 0;
  
  virtual size_t msgSize() const = 0;
  virtual size_t paritySize() const = 0;
  virtual size_t extrinsicMsgSize() const = 0;
  virtual size_t extrinsicParitySize() const = 0;
  virtual const CodeStructure& structure() const = 0;
  
  template <typename T> void encode(const T& message, T& parity) const;
  
  template <typename T1, typename T2> void decode(const T1& parityIn, T2& msgOut) const;
  template <typename T> void softOutDecode(const T& parityIn, T& msgOut) const;
  template <typename T> void appDecode(const T& parityIn, const T& extrinsicIn, T& msgOut, T& extrinsicOut) const;
  template <typename T> void parityAppDecode(const T& parityIn, const T& extrinsicIn, T& msgOut, T& extrinsicOut) const;
  
protected:
  ErrorCorrectingCode(int workGroupdSize = 4);
  
  inline int workGroupSize() const {return workGroupSize_;}
  
  virtual void encodeNBloc(std::vector<uint8_t>::const_iterator messageIt, std::vector<uint8_t>::iterator parityIt, size_t n) const;
  virtual void encodeBloc(std::vector<uint8_t>::const_iterator messageIt, std::vector<uint8_t>::iterator parityIt) const = 0;
  
  virtual void parityAppDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const = 0;
  virtual void appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const = 0;
  virtual void softOutDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut, size_t n) const = 0;
  virtual void decodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<uint8_t>::iterator messageOut, size_t n) const = 0;
  
private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_NVP(workGroupSize_);
  }
  
  int workGroupSize_;
};

BOOST_SERIALIZATION_ASSUME_ABSTRACT(ErrorCorrectingCode);
BOOST_CLASS_TYPE_INFO(ErrorCorrectingCode,extended_type_info_no_rtti<ErrorCorrectingCode>);
BOOST_CLASS_EXPORT_KEY(ErrorCorrectingCode);

template <typename T>
void ErrorCorrectingCode::encode(const T& message, T& parity) const
{
  uint64_t blocCount = message.size() / (msgSize());
  if (message.size() != blocCount * msgSize()) {
    throw std::invalid_argument("Invalid size for message");
  }
  
  parity.resize(blocCount * paritySize());
  std::fill(parity.begin(), parity.end(), 0);
  
  auto messageIt = message.begin();
  auto parityIt = parity.begin();
  
  std::vector<std::thread> threadGroup;
  threadGroup.reserve(workGroupSize());
  auto thread = threadGroup.begin();
  size_t step = (blocCount+workGroupSize()-1)/workGroupSize();
  for (int i = 0; i + step <= blocCount; i += step) {
    threadGroup.push_back( std::thread(&ErrorCorrectingCode::encodeNBloc, this,
                                       messageIt, parityIt, step) );
    messageIt += msgSize() * step;
    parityIt += paritySize() * step;
    
    thread++;
  }
  if (messageIt != message.end()) {
    encodeNBloc(messageIt, parityIt, blocCount % step);
  }
  for (auto & thread : threadGroup) {
    thread.join();
  }
}

/*******************************************************************************
 *  Decodes several blocs of information bits.
 *  \param  codeIn  Vector containing extrinsic parity L-values
 *  \param  messageIn  Vector containing extrinsic information L-values
 *  \param  messageOut[out] Vector containing a posteriori information L-values
 ******************************************************************************/
template <typename T>
void ErrorCorrectingCode::parityAppDecode(const T& parityIn, const T& extrinsicIn, T& msgOut, T& extrinsicOut) const
{
  size_t blocCount = parityIn.size() / paritySize();
  if (parityIn.size() != blocCount * paritySize()) {
    throw std::invalid_argument("Invalid size for parity");
  }
  if (extrinsicIn.size() != blocCount *  extrinsicParitySize()) {
    throw std::invalid_argument("Invalid size for extrinsic");
  }
  
  std::vector<std::thread> threadGroup;
  threadGroup.reserve(workGroupSize());
  
  msgOut.resize(blocCount * msgSize());
  extrinsicOut.resize(extrinsicIn.size());
  
  auto extrinsicInIt = extrinsicIn.begin();
  auto parityInIt = parityIn.begin();
  auto extrinsicOutIt = extrinsicOut.begin();
  auto msgOutIt = msgOut.begin();
  
  
  auto thread = threadGroup.begin();
  size_t step = (blocCount+workGroupSize()-1)/workGroupSize();
  for (int i = 0; i + step <= blocCount; i += step) {
    threadGroup.push_back( std::thread(&ErrorCorrectingCode::parityAppDecodeNBloc, this,
                                       parityInIt,extrinsicInIt,msgOutIt,extrinsicOutIt, step) );
    parityInIt += paritySize() * step;
    msgOutIt += msgSize() * step;
    extrinsicInIt += extrinsicParitySize() * step;
    extrinsicOutIt += extrinsicParitySize() * step;
    
    thread++;
  }
  if (parityInIt != parityIn.end()) {
    parityAppDecodeNBloc(parityInIt, extrinsicInIt, msgOutIt, extrinsicOutIt, blocCount % step);
  }
  for (auto & thread : threadGroup) {
    thread.join();
  }
}

/*******************************************************************************
 *  Decodes several blocs of information bits.
 *  \param  codeIn  Vector containing extrinsic parity L-values
 *  \param  messageIn  Vector containing extrinsic information L-values
 *  \param  messageOut[out] Vector containing a posteriori information L-values
 ******************************************************************************/
template <typename T>
void ErrorCorrectingCode::appDecode(const T& parityIn, const T& extrinsicIn, T& messageOut, T& extrinsicOut) const
{
  size_t blocCount = parityIn.size() / paritySize();
  if (parityIn.size() != blocCount * paritySize()) {
    throw std::invalid_argument("Invalid size for parity");
  }
  if (extrinsicIn.size() != blocCount *  extrinsicMsgSize()) {
    throw std::invalid_argument("Invalid size for message extrinsic");
  }
  
  std::vector<std::thread> threadGroup;
  threadGroup.reserve(workGroupSize());
  
  messageOut.resize(blocCount * msgSize());
  extrinsicOut.resize(extrinsicIn.size());
  
  auto extrinsicInIt = extrinsicIn.begin();
  auto parityInIt = parityIn.begin();
  auto extrinsicOutIt = extrinsicOut.begin();
  auto messageOutIt = messageOut.begin();
  
  
  auto thread = threadGroup.begin();
  size_t step = (blocCount+workGroupSize()-1)/workGroupSize();
  for (int i = 0; i + step <= blocCount; i += step) {
    threadGroup.push_back( std::thread(&ErrorCorrectingCode::appDecodeNBloc, this,
                                       parityInIt,extrinsicInIt,messageOutIt, extrinsicOutIt, step) );
    parityInIt += paritySize() * step;
    extrinsicInIt += extrinsicMsgSize() * step;
    extrinsicOutIt += extrinsicMsgSize() * step;
    messageOutIt += msgSize() * step;
    
    thread++;
  }
  if (parityInIt != parityIn.end()) {
    appDecodeNBloc(parityInIt, extrinsicInIt, messageOutIt, extrinsicOutIt, blocCount % step);
  }
  for (auto & thread : threadGroup) {
    thread.join();
  }
}

template <typename T>
void ErrorCorrectingCode::softOutDecode(const T& parityIn, T& messageOut) const
{
  size_t blocCount = parityIn.size() / paritySize();
  if (parityIn.size() != blocCount * paritySize()) {
    throw std::invalid_argument("Invalid size for parity");
  }
  
  messageOut.resize(blocCount * msgSize());
  
  auto parityInIt = parityIn.begin();
  auto messageOutIt = messageOut.begin();
  
  std::vector<std::thread> threadGroup;
  threadGroup.reserve(workGroupSize());
  
  auto thread = threadGroup.begin();
  size_t step = (blocCount+workGroupSize()-1)/workGroupSize();
  for (int i = 0; i + step <= blocCount; i += step) {
    threadGroup.push_back( std::thread(&ErrorCorrectingCode::softOutDecodeNBloc, this,
                                       parityInIt,
                                       messageOutIt, step
                                       ) );
    parityInIt += paritySize() * step;
    messageOutIt += msgSize() * step;
    
    thread++;
  }
  if (messageOutIt != messageOut.end()) {
    softOutDecodeNBloc(parityInIt, messageOutIt, blocCount % step);
  }
  for (auto & thread : threadGroup) {
    thread.join();
  }
}

template <typename T1, typename T2>
void ErrorCorrectingCode::decode(const T1& parityIn, T2& messageOut) const
{
  size_t blocCount = parityIn.size() / paritySize();
  if (parityIn.size() != blocCount * paritySize()) {
    throw std::invalid_argument("Invalid size for parity");
  }
  
  std::vector<std::thread> threadGroup;
  threadGroup.reserve(workGroupSize());
  
  messageOut.resize(blocCount * msgSize());
  
  auto parityInIt = parityIn.begin();
  auto messageOutIt = messageOut.begin();
  
  auto thread = threadGroup.begin();
  size_t step = (blocCount+workGroupSize()-1)/workGroupSize();
  for (int i = 0; i + step <= blocCount; i += step) {
    threadGroup.push_back( std::thread(&ErrorCorrectingCode::decodeNBloc, this,
                                       parityInIt, messageOutIt, step) );
    parityInIt += paritySize() * step;
    messageOutIt += msgSize() * step;
    
    thread++;
  }
  if (messageOutIt != messageOut.end()) {
    decodeNBloc(parityInIt, messageOutIt, blocCount % step);
  }
  for (auto & thread : threadGroup) {
    thread.join();
  }
}

#endif