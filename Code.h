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
 
 Declaration of Code class
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

/**
 *  This class represents a general encoder / decoder.
 *  It offers methods to encode and to decode data given a code structure.
 */
class Code
{
  friend class boost::serialization::access;
public:
  static std::unique_ptr<Code> create(const CodeStructure& codeStructure, int workGroupdSize = 4);
  virtual ~Code() = default;
  
  virtual const char * get_key() const = 0;
  
  /**
   *  Access size of the message in one bloc.
   *  \return Message size
   */
  virtual size_t msgSize() const = 0;
  /**
   *  Access size of one parity bloc.
   *  \return Parity size
   */
  virtual size_t paritySize() const = 0;
  /**
   *  Access size of extrinsic information in one bloc.
   *  \return Extrinsic size
   */
  virtual size_t extrinsicSize() const = 0;
  virtual const CodeStructure& structure() const = 0;
  
  template <template <typename> class A> void encode(const std::vector<uint8_t,A<uint8_t>>& message, std::vector<uint8_t,A<uint8_t>>& parity) const;
  
  template <template <typename> class A> void decode(const std::vector<LlrType,A<LlrType>>& parityIn, std::vector<uint8_t,A<uint8_t>>& msgOut) const;
  template <template <typename> class A> void softOutDecode(const std::vector<LlrType,A<LlrType>>& parityIn, std::vector<LlrType,A<LlrType>>& msgOut) const;
  template <template <typename> class A> void appDecode(const std::vector<LlrType,A<LlrType>>& parityIn, const std::vector<LlrType,A<LlrType>>& extrinsicIn, std::vector<LlrType,A<LlrType>>& msgOut, std::vector<LlrType,A<LlrType>>& extrinsicOut) const;

protected:
  Code(int workGroupdSize = 4);
  
  inline int workGroupSize() const {return workGroupSize_;}
  
  /**
   *  Encodes several blocs of msg bits.
   *  \param  messageIt  Input iterator pointing to the first element in the msg bit sequence.
   *  \param  parityIt[out] Output iterator pointing to the first element in the parity bit sequence.
   *    The output neeeds to be pre-allocated.
   */
  virtual void encodeNBloc(std::vector<uint8_t>::const_iterator messageIt, std::vector<uint8_t>::iterator parityIt, size_t n) const;
  /**
   *  Encodes one bloc of msg bits.
   *  \param  messageIt  Input iterator pointing to the first element in the msg bit sequence.
   *  \param  parityIt[out] Output iterator pointing to the first element in the parity bit sequence.
   *    The output neeeds to be pre-allocated.
   */
  virtual void encodeBloc(std::vector<uint8_t>::const_iterator messageIt, std::vector<uint8_t>::iterator parityIt) const = 0;
  
  /**
   *  Decodes several blocs of information bits.
   *  A priori information about the decoder state is provided and extrinsic
   *  information is output, following the same structure. These informations
   *  can be transfered sequencially to multiple decoding attempts.
   *  \param  parityIn  Input iterator pointing to the first element 
   *    in the parity L-value sequence
   *  \param  extrinsicIn  Input iterator pointing to the first element 
   *    in the a-priori extrinsic L-value sequence
   *  \param  messageOut[out] Output iterator pointing to the first element 
   *    in the a posteriori information L-value sequence. 
   *    Output needs to be pre-allocated.
   *  \param  extrinsicOut[out]  Output iterator pointing to the first element
   *    in the extrinsic L-value sequence.
   *    Output needs to be pre-allocated.
   */
  virtual void appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const = 0;
  /**
   *  Decodes several blocs of information bits.
   *  A posteriori information about the msg is output instead of the decoded bit sequence.
   *  \param  parityIn  Input iterator pointing to the first element
   *    in the parity L-value sequence
   *  \param  messageOut[out] Output iterator pointing to the first element
   *    in the a posteriori information L-value sequence.
   *    Output needs to be pre-allocated.
   */
  virtual void softOutDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut, size_t n) const = 0;
  /**
   *  Decodes several blocs of information bits.
   *  \param  parityIn  Input iterator pointing to the first element
   *    in the parity L-value sequence
   *  \param  messageOut[out] Output iterator pointing to the first element
   *    in the decoded msg sequence.
   *    Output needs to be pre-allocated.
   */
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

/**
 *  Encodes several blocs of information bits.
 *  Chunks of blocs are encded in parallel.
 *  \param  message  Vector containing information bits
 *  \param  parity[out] Vector containing parity bits
 *  \tparam A Container allocator. The reason for different allocator is to allow 
 *    the matlab API to use a custom mex allocator
 */
template <template <typename> class A>
void fec::Code::encode(const std::vector<uint8_t,A<uint8_t>>& message, std::vector<uint8_t,A<uint8_t>>& parity) const
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

/**
 *  Decodes several blocs of information bits.
 *  A priori information about the decoder state is provided and extrinsic
 *  information is output, following the same structure. These informations
 *  can be transfered sequencially to multiple decoding attempts.
 *  Chunks of blocs are encded in parallel.
 *  \param  parityIn  Vector containing parity L-values
 *  \param  extrinsicIn  Vector containing a-priori extrinsic L-values
 *  \param  messageOut[out] Vector containing a posteriori information L-values
 *  \param  extrinsicOut[out] Vector containing a extrinsic L-values
 *  \tparam A Container allocator. The reason for different allocator is to allow
 *    the matlab API to use a custom mex allocator
 */
template <template <typename> class A>
void fec::Code::appDecode(const std::vector<LlrType,A<LlrType>>& parityIn, const std::vector<LlrType,A<LlrType>>& extrinsicIn, std::vector<LlrType,A<LlrType>>& messageOut, std::vector<LlrType,A<LlrType>>& extrinsicOut) const
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

/**
 *  Decodes several blocs of information bits.
 *  A posteriori information about the msg is output instead of the decoded bit sequence.
 *  Chunks of blocs are encded in parallel.
 *  \param  parityIn  Vector containing parity L-values
 *  \param  messageOut[out] Vector containing a posteriori information L-values
 *  \tparam A Container allocator. The reason for different allocator is to allow
 *    the matlab API to use a custom mex allocator
 */
template <template <typename> class A>
void fec::Code::softOutDecode(const std::vector<LlrType,A<LlrType>>& parityIn, std::vector<LlrType,A<LlrType>>& messageOut) const
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

/**
 *  Decodes several blocs of information bits.
 *  Chunks of blocs are encded in parallel.
 *  \param  parityIn  Vector containing parity L-values
 *    Given a signal y and a parity bit x, we define the correspondig L-value as
 *    L = ln[ p(x = 1 | y) / p(x = 0 | y) ] = ln[ p(y | x = 1) / p(y | x = 0) ]
 *  \param  messageOut[out] Vector containing message bits
 *  \tparam A Container allocator. The reason for different allocator is to allow
 *    the matlab API to use a custom mex allocator
 */
template <template <typename> class A>
void fec::Code::decode(const std::vector<LlrType,A<LlrType>>& parityIn, std::vector<uint8_t,A<uint8_t>>& messageOut) const
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