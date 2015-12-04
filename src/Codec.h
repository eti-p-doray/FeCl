/*******************************************************************************
 This file is part of FeCl.
 
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 Copyright (c) 2015, Leszek Szczecinski, INRS
 All rights reserved.
 
 FeCl is free software: you can redistribute it and/or modify
 it under the terms of the Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 FeCl is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the Lesser General Public License
 along with FeCl.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef FEC_CODEC_H
#define FEC_CODEC_H

#include <memory>
#include <thread>
#include <vector>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/serialization/extended_type_info_no_rtti.hpp>

#include "detail/Codec.h"

namespace fec {
  
  /**
   *  This class represents a general encoder / decoder.
   *  It offers methods to encode and to decode data given a codec structure.
   *  Several specialisations of this class define different codec types.
   */
  class Codec
  {
    friend class boost::serialization::access;
  public:
    
    template <typename T, template <typename> class A = std::allocator>
    using Input = detail::Codec::Info<const std::vector<T,A<T>>>;
    template <typename T, template <typename> class A = std::allocator>
    using Output = detail::Codec::Info<std::vector<T,A<T>>>;
    
    virtual ~Codec() = default;
    
    virtual const char * get_key() const = 0; /**< Access the type info key. */
    
    size_t msgSize() const {return structure().msgSize();} /**< Access the size of the msg in each code bloc. */
    size_t systSize() const {return structure().systSize();} /**< Access the size of the msg in each code bloc. */
    size_t paritySize() const {return structure().paritySize();} /**< Access the size of the parity in each code bloc. */
    size_t stateSize() const {return structure().stateSize();} /**< Access the size of the extrinsic in each code bloc. */
    
    int getWorkGroupSize() const {return workGroupSize_;}
    void setWorkGroupSize(int size) {workGroupSize_ = size;}
    
    template <template <typename> class A>
    bool check(const std::vector<BitField<size_t>,A<BitField<size_t>>>& parity) const;
    
    template <template <typename> class A>
    void encode(const std::vector<BitField<size_t>,A<BitField<size_t>>>& message, std::vector<BitField<size_t>,A<BitField<size_t>>>& parity) const;
    template <template <typename> class A>
    std::vector<BitField<size_t>,A<BitField<size_t>>> encode(const std::vector<BitField<size_t>,A<BitField<size_t>>>& message) const;
    
    template <template <typename> class A>
    void decode(const std::vector<double,A<double>>& parity, std::vector<BitField<size_t>,A<BitField<size_t>>>& msg) const;
    template <template <typename> class A>
    std::vector<BitField<size_t>,A<BitField<size_t>>> decode(const std::vector<double,A<double>>& parity) const;
    
    template <template <typename> class A>
    void soDecode(Input<double, A> input, Output<double, A> output) const;
    
  protected:
    Codec() = default;
    Codec(std::unique_ptr<detail::Codec::Structure>&&, int workGroupSize = 8);
    
    Codec(const Codec& other) {*this = other;}
    Codec& operator=(const Codec& other) {workGroupSize_ = other.getWorkGroupSize(); return *this;}
    
    inline const detail::Codec::Structure& structure() const {return *structure_;}
    inline detail::Codec::Structure& structure() {return *structure_;}
    
    virtual bool checkBlocks(std::vector<BitField<size_t>>::const_iterator parity, size_t n) const;
    virtual void encodeBlocks(std::vector<BitField<size_t>>::const_iterator msg, std::vector<BitField<size_t>>::iterator parity, size_t n) const;
    
    /**
     *  Decodes several blocks of information bits.
     *  \param  parityIn  Input iterator pointing to the first element
     *    in the parity L-value sequence
     *  \param  messageOut[out] Output iterator pointing to the first element
     *    in the decoded msg sequence.
     *    Output needs to be pre-allocated.
     */
    virtual void decodeBlocks(std::vector<double>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg, size_t n) const = 0;
    /**
     *  Decodes several blocks of information bits.
     *  A posteriori information about the msg is output instead of the decoded bit sequence.
     *  \param  parityIn  Input iterator pointing to the first element
     *    in the parity L-value sequence
     *  \param  messageOut[out] Output iterator pointing to the first element
     *    in the a posteriori information L-value sequence.
     *    Output needs to be pre-allocated.
     */
    virtual void soDecodeBlocks(detail::Codec::InputIterator input, detail::Codec::OutputIterator output, size_t n) const = 0;
    
    std::unique_ptr<detail::Codec::Structure> structure_;
    
  private:
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version);
    
    std::vector<std::thread> createWorkGroup() const;
    size_t taskSize(size_t blockCount) const;
    
    int workGroupSize_;
  };
  
}



BOOST_SERIALIZATION_ASSUME_ABSTRACT(fec::Codec);
BOOST_CLASS_TYPE_INFO(fec::Codec,extended_type_info_no_rtti<fec::Codec>);
BOOST_CLASS_EXPORT_KEY(fec::Codec);

template <template <typename> class A>
bool fec::Codec::check(const std::vector<BitField<size_t>,A<BitField<size_t>>>& parity) const
{
  uint64_t blockCount = parity.size() / (paritySize());
  if (parity.size() != blockCount * paritySize()) {
    throw std::invalid_argument("Invalid size for parity");
  }
  return checkBlocks(parity.begin(), blockCount);
}

template <template <typename> class A>
std::vector<fec::BitField<size_t>,A<fec::BitField<size_t>>> fec::Codec::encode(const std::vector<fec::BitField<size_t>,A<fec::BitField<size_t>>>& message) const
{
  std::vector<fec::BitField<size_t>,A<fec::BitField<size_t>>> parity;
  encode(message, parity);
  return parity;
}

template <template <typename> class A>
std::vector<fec::BitField<size_t>,A<fec::BitField<size_t>>> fec::Codec::decode(const std::vector<double,A<double>>& parity) const
{
  std::vector<fec::BitField<size_t>,A<fec::BitField<size_t>>> message;
  decode(parity, message);
  return message;
}

/**
 *  Encodes several blocks of information bits.
 *  Chunks of blocs are encded in parallel.
 *  \param  message  Vector containing information bits
 *  \param  parity[out] Vector containing parity bits
 *  \tparam A Container allocator. The reason for different allocator is to allow
 *    the matlab API to use a custom mex allocator
 */
template <template <typename> class A>
void fec::Codec::encode(const std::vector<BitField<size_t>,A<BitField<size_t>>>& msg, std::vector<BitField<size_t>,A<BitField<size_t>>>& parity) const
{
  uint64_t blockCount = msg.size() / (msgSize());
  if (msg.size() != blockCount * msgSize()) {
    throw std::invalid_argument("Invalid size for message");
  }
  
  parity.resize(blockCount * paritySize(), 0);
  auto msgIt = msg.begin(); auto parityIt = parity.begin();
  
  auto threadGroup = createWorkGroup();
  auto thread = threadGroup.begin();
  size_t step = taskSize(blockCount);
  for (int i = 0; i + step <= blockCount; i += step) {
    threadGroup.push_back( std::thread(&Codec::encodeBlocks, this, msgIt, parityIt, step) );
    msgIt += msgSize() * step;
    parityIt += paritySize() * step;
    
    ++thread;
  }
  if (msgIt != msg.end()) {
    encodeBlocks(msgIt, parityIt, blockCount % step);
  }
  for (auto & thread : threadGroup) {
    thread.join();
  }
}

/**
 *  Decodes several blocks of information bits.
 *  Chunks of blocs are encded in parallel.
 *  \param  parityIn  Vector containing parity L-values
 *    Given a signal y and a parity bit x, we define the correspondig L-value as
 *    L = ln[ p(x = 1 | y) / p(x = 0 | y) ] = ln[ p(y | x = 1) / p(y | x = 0) ]
 *  \param  messageOut[out] Vector containing message bits
 *  \tparam A Container allocator. The reason for different allocator is to allow
 *    the matlab API to use a custom mex allocator
 */
template <template <typename> class A>
void fec::Codec::decode(const std::vector<double,A<double>>& parity, std::vector<BitField<size_t>,A<BitField<size_t>>>& msg) const
{
  size_t blockCount = parity.size() / paritySize();
  if (parity.size() != blockCount * paritySize()) {
    throw std::invalid_argument("Invalid size for parity");
  }
  
  msg.resize(blockCount * msgSize());
  auto parityInIt = parity.begin(); auto msgOutIt = msg.begin();
  
  auto threadGroup = createWorkGroup();
  auto thread = threadGroup.begin();
  size_t step = taskSize(blockCount);
  for (int i = 0; i + step <= blockCount; i += step) {
    threadGroup.push_back( std::thread(&Codec::decodeBlocks, this,
                                       parityInIt, msgOutIt, step) );
    parityInIt += paritySize() * step;
    msgOutIt += msgSize() * step;
    
    ++thread;
  }
  if (msgOutIt != msg.end()) {
    decodeBlocks(parityInIt, msgOutIt, blockCount % step);
  }
  for (auto & thread : threadGroup) {
    thread.join();
  }
}

/**
 *  Decodes several blocks of information bits.
 *  A posteriori information about the msg is output instead of the decoded bit sequence.
 *  Chunks of blocs are encded in parallel.
 *  \param  parityIn  Vector containing parity L-values
 *  \param  messageOut[out] Vector containing a posteriori information L-values
 *  \tparam A Container allocator. The reason for different allocator is to allow
 *    the matlab API to use a custom mex allocator
 */
template <template <typename> class A>
void fec::Codec::soDecode(Input<A> input, Output<A> output) const
{
  if (!input.hasParity()) {
    throw std::invalid_argument("Input must contains parity");
  }
  if (input.hasMsg()) {
    throw std::invalid_argument("Input should not contain msg");
  }
  size_t blockCount = input.parity().size() / paritySize();
  if (input.parity().size() != blockCount * paritySize()) {
    throw std::invalid_argument("Invalid size for parity");
  }
  if (input.hasSyst()) {
    if (input.syst().size() != blockCount * systSize()) {
      throw std::invalid_argument("Invalid size for msg");
    }
  }
  if (input.hasState()) {
    if (input.state().size() != blockCount * stateSize()) {
      throw std::invalid_argument("Invalid size for state");
    }
  }
  
  if (output.hasParity()) {
    output.parity().resize(blockCount * paritySize());
  }
  if (output.hasSyst()) {
    output.syst().resize(blockCount * systSize());
  }
  if (output.hasState()) {
    output.state().resize(blockCount * stateSize());
  }
  if (output.hasMsg()) {
    output.msg().resize(blockCount * msgSize());
  }
  auto inputIt = input.begin(structure());
  auto outputIt = output.begin(structure());
  
  auto threadGroup = createWorkGroup();
  auto thread = threadGroup.begin();
  size_t step = taskSize(blockCount);
  for (int i = 0; i + step <= blockCount; i += step) {
    threadGroup.push_back( std::thread(&Codec::soDecodeBlocks, this,
                                       inputIt,
                                       outputIt, step
                                       ) );
    inputIt += step;
    outputIt += step;
    
    thread++;
  }
  if (outputIt != output.end(structure())) {
    soDecodeBlocks(inputIt, outputIt, blockCount % step);
  }
  for (auto & thread : threadGroup) {
    thread.join();
  }
}

template <typename Archive>
void fec::Codec::serialize(Archive & ar, const unsigned int version) {
  using namespace boost::serialization;
  ar & ::BOOST_SERIALIZATION_NVP(workGroupSize_);
  ar & ::BOOST_SERIALIZATION_NVP(structure_);
}

#endif