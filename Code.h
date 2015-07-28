/*******************************************************************************
 *  \file Code.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-06-11
 *  \version Last update : 2015-06-11
 *
 *  Declaration of Code class
 ******************************************************************************/

#ifndef CODE_H
#define CODE_H

#include <memory>
#include <thread>

#include "Archive.h"
#include <vector>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/serialization/extended_type_info_no_rtti.hpp>

#include "CodeStructure/CodeStructure.h"

namespace fec {

/*******************************************************************************
 *  This class represents a map encode / decoder
 *  It offers methods encode and to decode data giving a posteriori informations
 *  using a trellis.
 ******************************************************************************/
class Code
{
  friend class boost::serialization::access;
public:
  static std::unique_ptr<Code> create(const CodeStructure& codeStructure, int workGroupdSize = 4);
  virtual ~Code() = default;
  
  virtual const char * get_key() const = 0;
  
  virtual size_t msgSize() const = 0;
  virtual size_t paritySize() const = 0;
  virtual size_t extrinsicSize() const = 0;
  virtual const CodeStructure& structure() const = 0;
  
  template <typename T> void encode(const std::vector<uint8_t,T>& message, std::vector<uint8_t,T>& parity) const;
  
  template <typename T1,typename T2> void decode(const std::vector<LlrType,T1>& parityIn, std::vector<uint8_t,T2>& msgOut) const;
  template <typename T> void softOutDecode(const std::vector<LlrType,T>& parityIn, std::vector<LlrType,T>& msgOut) const;
  template <typename T> void appDecode(const std::vector<LlrType,T>& parityIn, const std::vector<LlrType,T>& extrinsicIn, std::vector<LlrType,T>& msgOut, std::vector<LlrType,T>& extrinsicOut) const;

protected:
  Code(int workGroupdSize = 4);
  
  inline int workGroupSize() const {return workGroupSize_;}
  
  virtual void encodeNBloc(std::vector<uint8_t>::const_iterator messageIt, std::vector<uint8_t>::iterator parityIt, size_t n) const;
  virtual void encodeBloc(std::vector<uint8_t>::const_iterator messageIt, std::vector<uint8_t>::iterator parityIt) const = 0;
  
  virtual void appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const = 0;
  virtual void softOutDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut, size_t n) const = 0;
  virtual void decodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<uint8_t>::iterator messageOut, size_t n) const = 0;
  
private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    using namespace boost::serialization;
    ar & ::BOOST_SERIALIZATION_NVP(workGroupSize_);
  }
  
  int workGroupSize_;
};
  
}

BOOST_SERIALIZATION_ASSUME_ABSTRACT(fec::Code);
BOOST_CLASS_TYPE_INFO(fec::Code,extended_type_info_no_rtti<fec::Code>);
BOOST_CLASS_EXPORT_KEY(fec::Code);

/*******************************************************************************
 *  Encode several blocs of information bits.
 *  \param  message  Vector containing information bits
 *  \param  parity[out] Vector containing parity bits
 ******************************************************************************/
template <typename T>
void fec::Code::encode(const std::vector<uint8_t,T>& message, std::vector<uint8_t,T>& parity) const
{
  uint64_t blocCount = message.size() / (msgSize());
  if (message.size() != blocCount * msgSize()) {
    throw std::invalid_argument("Invalid size for message");
  }
  
  parity.resize(blocCount * paritySize());
  std::fill(parity.begin(), parity.end(), 0);
  
  std::vector<uint8_t>::const_iterator messageIt = message.begin();
  std::vector<uint8_t>::iterator parityIt = parity.begin();
  
  std::vector<std::thread> threadGroup;
  threadGroup.reserve(workGroupSize());
  auto thread = threadGroup.begin();
  size_t step = (blocCount+workGroupSize()-1)/workGroupSize();
  for (int i = 0; i + step <= blocCount; i += step) {
    threadGroup.push_back( std::thread(&Code::encodeNBloc, this,
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
 *  \param  parityIn  Vector containing parity L-values
 *  \param  extrinsicIn  Vector containing extrinsic L-values
 *  \param  messageOut[out] Vector containing a posteriori information L-values
 *  \param  extrinsicOut[out] Vector containing a extrinsic L-values
 ******************************************************************************/
template <typename T>
void fec::Code::appDecode(const std::vector<LlrType,T>& parityIn, const std::vector<LlrType,T>& extrinsicIn, std::vector<LlrType,T>& messageOut, std::vector<LlrType,T>& extrinsicOut) const
{
  size_t blocCount = parityIn.size() / paritySize();
  if (parityIn.size() != blocCount * paritySize()) {
    throw std::invalid_argument("Invalid size for parity");
  }
  if (extrinsicIn.size() != blocCount * extrinsicSize()) {
    throw std::invalid_argument("Invalid size for extrinsic");
  }
  
  std::vector<std::thread> threadGroup;
  threadGroup.reserve(workGroupSize());
  
  messageOut.resize(blocCount * msgSize());
  extrinsicOut.resize(extrinsicIn.size());
  
  std::vector<LlrType>::const_iterator extrinsicInIt = extrinsicIn.begin();
  std::vector<LlrType>::const_iterator  parityInIt = parityIn.begin();
  std::vector<LlrType>::iterator extrinsicOutIt = extrinsicOut.begin();
  std::vector<LlrType>::iterator messageOutIt = messageOut.begin();
  
  
  auto thread = threadGroup.begin();
  size_t step = (blocCount+workGroupSize()-1)/workGroupSize();
  for (int i = 0; i + step <= blocCount; i += step) {
    threadGroup.push_back( std::thread(&Code::appDecodeNBloc, this,
                                       parityInIt,extrinsicInIt,messageOutIt, extrinsicOutIt, step) );
    parityInIt += paritySize() * step;
    extrinsicInIt += extrinsicSize() * step;
    extrinsicOutIt += extrinsicSize() * step;
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

/*******************************************************************************
 *  Decodes several blocs of information bits.
 *  \param  parityIn  Vector containing parity L-values
 *  \param  messageOut[out] Vector containing a posteriori information L-values
 ******************************************************************************/
template <typename T>
void fec::Code::softOutDecode(const std::vector<LlrType,T>& parityIn, std::vector<LlrType,T>& messageOut) const
{
  size_t blocCount = parityIn.size() / paritySize();
  if (parityIn.size() != blocCount * paritySize()) {
    throw std::invalid_argument("Invalid size for parity");
  }
  
  messageOut.resize(blocCount * msgSize());
  
  std::vector<LlrType>::const_iterator parityInIt = parityIn.begin();
  std::vector<LlrType>::iterator messageOutIt = messageOut.begin();
  
  std::vector<std::thread> threadGroup;
  threadGroup.reserve(workGroupSize());
  
  auto thread = threadGroup.begin();
  size_t step = (blocCount+workGroupSize()-1)/workGroupSize();
  for (int i = 0; i + step <= blocCount; i += step) {
    threadGroup.push_back( std::thread(&Code::softOutDecodeNBloc, this,
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

/*******************************************************************************
 *  Decodes several blocs of information bits.
 *  \param  parityIn  Vector containing parity L-values
 *  \param  messageOut[out] Vector containing message bits
 ******************************************************************************/
template <typename T1, typename T2>
void fec::Code::decode(const std::vector<LlrType,T1>& parityIn, std::vector<uint8_t,T2>& messageOut) const
{
  size_t blocCount = parityIn.size() / paritySize();
  if (parityIn.size() != blocCount * paritySize()) {
    throw std::invalid_argument("Invalid size for parity");
  }
  
  std::vector<std::thread> threadGroup;
  threadGroup.reserve(workGroupSize());
  
  messageOut.resize(blocCount * msgSize());
  
  std::vector<LlrType>::const_iterator parityInIt = parityIn.begin();
  std::vector<uint8_t>::iterator messageOutIt = messageOut.begin();
  
  auto thread = threadGroup.begin();
  size_t step = (blocCount+workGroupSize()-1)/workGroupSize();
  for (int i = 0; i + step <= blocCount; i += step) {
    threadGroup.push_back( std::thread(&Code::decodeNBloc, this,
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