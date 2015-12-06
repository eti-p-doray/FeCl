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
#include <unordered_map>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/serialization/extended_type_info_no_rtti.hpp>

#include "detail/Codec.h"
#include "detail/MultiIterator.h"
#include "detail/WorkGroup.h"

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
    using Input = detail::Codec::Arguments<const std::vector<T,A<T>>>;
    template <typename T, template <typename> class A = std::allocator>
    using Output = detail::Codec::Arguments<std::vector<T,A<T>>>;
    
    template <typename T, template <typename> class A = std::allocator>
    static Input<T,A> input(const std::vector<T,A<T>>& parity) {return Input<T,A>{}.parity(parity);}
    template <typename T, template <typename> class A = std::allocator>
    static Output<T,A> output(std::vector<T,A<T>>& msg) {return Output<T,A>{}.msg(msg);}
    
    virtual ~Codec() = default;
    
    virtual const char * get_key() const = 0; /**< Access the type info key. */
    
    inline size_t msgSize() const {return structure().msgSize();} /**< Access the size of the msg in each code bloc. */
    inline size_t systSize() const {return structure().systSize();} /**< Access the size of the msg in each code bloc. */
    inline size_t paritySize() const {return structure().paritySize();} /**< Access the size of the parity in each code bloc. */
    inline size_t stateSize() const {return structure().stateSize();} /**< Access the size of the extrinsic in each code bloc. */
    
    //int getWorkGroupSize() const {return workGroupSize_;}
    //void setWorkGroupSize(int size) {workGroupSize_ = size;}
    
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
    Codec& operator=(const Codec& other) {workGroup_ = other.workGroup_; return *this;}
    
    inline const detail::Codec::Structure& structure() const {return *structure_;}
    inline detail::Codec::Structure& structure() {return *structure_;}
    
    virtual bool checkBlocks(detail::Codec::const_iterator<BitField<size_t>> first, detail::Codec::const_iterator<BitField<size_t>> last) const;
    virtual void encodeBlocks(detail::Codec::const_iterator<BitField<size_t>> first, detail::Codec::const_iterator<BitField<size_t>> last, detail::Codec::iterator<BitField<size_t>> output) const;
    
    virtual void decodeBlocks(detail::Codec::const_iterator<double> first, detail::Codec::const_iterator<double> last, detail::Codec::iterator<BitField<size_t>> output) const = 0;
    virtual void soDecodeBlocks(detail::Codec::const_iterator<double> first, detail::Codec::const_iterator<double> last, detail::Codec::iterator<double> output) const = 0;
    
    std::unique_ptr<detail::Codec::Structure> structure_;
    
  private:
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version);
    
    std::vector<std::thread> createWorkGroup() const;
    size_t taskSize(size_t blockCount) const;
    
    detail::WorkGroup workGroup_;
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

/**
 *  Encodes several blocks of information bits.
 *  Chunks of blocs are encded in parallel.
 *  \param  message  Vector containing information bits
 *  \return Vector containing parity bits
 *  \tparam A Container allocator. The reason for different allocator is to allow
 *    the matlab API to use a custom mex allocator
 */
template <template <typename> class A>
std::vector<fec::BitField<size_t>,A<fec::BitField<size_t>>> fec::Codec::encode(const std::vector<fec::BitField<size_t>,A<fec::BitField<size_t>>>& message) const
{
  std::vector<fec::BitField<size_t>,A<fec::BitField<size_t>>> parity;
  encode(message, parity);
  return parity;
}

/**
 *  Encodes several blocks of information bits.
 *  Chunks of blocs are encded in parallel.
 *  \param  msg  Vector containing information bits
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
  
  detail::Codec::const_iterator<BitField<size_t>> begin{{detail::Codec::Msg, msg.begin(), msgSize()}};
  detail::Codec::const_iterator<BitField<size_t>> end{{detail::Codec::Msg, msg.end(), msgSize()}};
  detail::Codec::iterator<BitField<size_t>> output{{detail::Codec::Parity, parity.begin(), paritySize()}};
  
  workGroup_.execute(begin, end, output, std::bind(&Codec::encodeBlocks, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

/**
 *  Decodes several blocks of information bits.
 *  Chunks of blocs are encded in parallel.
 *  \param  parity  Vector containing parity L-values
 *    Given a signal y and a parity bit x, we define the correspondig L-value as
 *    L = ln[ p(x = 1 | y) / p(x = 0 | y) ] = ln[ p(y | x = 1) / p(y | x = 0) ]
 *  \return Vector containing message bits
 *  \tparam A Container allocator. The reason for different allocator is to allow
 *    the matlab API to use a custom mex allocator
 */
template <template <typename> class A>
std::vector<fec::BitField<size_t>,A<fec::BitField<size_t>>> fec::Codec::decode(const std::vector<double,A<double>>& parity) const
{
  std::vector<fec::BitField<size_t>,A<fec::BitField<size_t>>> message;
  decode(parity, message);
  return message;
}

/**
 *  Decodes several blocks of information bits.
 *  Chunks of blocs are encded in parallel.
 *  \param  parityVector containing parity L-values
 *    Given a signal y and a parity bit x, we define the correspondig L-value as
 *    L = ln[ p(x = 1 | y) / p(x = 0 | y) ] = ln[ p(y | x = 1) / p(y | x = 0) ]
 *  \param  msg[out] Vector containing message bits
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
  
  detail::Codec::const_iterator<double> begin{{detail::Codec::Parity, parity.begin(), paritySize()}};
  detail::Codec::const_iterator<double> end{{detail::Codec::Parity, parity.end(), paritySize()}};
  detail::Codec::iterator<BitField<size_t>> output{{detail::Codec::Msg, msg.begin(), msgSize()}};
  
  workGroup_.execute(begin, end, output, std::bind(&Codec::decodeBlocks, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

/**
 *  Decodes several blocks of information bits.
 *  A posteriori information about the msg is output instead of the decoded bit sequence.
 *  Chunks of blocs are encded in parallel.
 *  \param  input  Map containing input vectors
 *  \param  output Map containing output vectors
 *  \tparam A Container allocator. The reason for different allocator is to allow
 *    the matlab API to use a custom mex allocator
 */
template <template <typename> class A>
void fec::Codec::soDecode(Input<double,A> input, Output<double,A> output) const
{
  if (!input.count(detail::Codec::Parity)) {
    throw std::invalid_argument("Input must contains parity");
  }
  if (input.count(detail::Codec::Msg)) {
    throw std::invalid_argument("Input should not contain msg");
  }
  size_t blockCount = input.at(detail::Codec::Parity).size() / paritySize();

  detail::Codec::const_iterator<double> begin{{detail::Codec::Parity, input.at(detail::Codec::Parity).begin(), paritySize()}};
  detail::Codec::const_iterator<double> end{{detail::Codec::Parity, input.at(detail::Codec::Parity).end(), paritySize()}};
  detail::Codec::iterator<double> outputIt;
  
  if (input.at(detail::Codec::Parity).size() != blockCount * paritySize()) {
    throw std::invalid_argument("Invalid size for parity");
  }
  if (input.count(detail::Codec::Syst)) {
    begin.insert(detail::Codec::Syst, input.at(detail::Codec::Syst).begin(), systSize());
    end.insert(detail::Codec::Syst, input.at(detail::Codec::Syst).end(), systSize());
    if (input.at(detail::Codec::Syst).size() != blockCount * systSize()) {
      throw std::invalid_argument("Invalid size for msg");
    }
  }
  if (input.count(detail::Codec::State)) {
    begin.insert(detail::Codec::State, input.at(detail::Codec::State).begin(), systSize());
    end.insert(detail::Codec::State, input.at(detail::Codec::State).end(), systSize());
    if (input.at(detail::Codec::State).size() != blockCount * stateSize()) {
      throw std::invalid_argument("Invalid size for state");
    }
  }
  
  if (output.count(detail::Codec::Parity)) {
    output.at(detail::Codec::Parity).resize(blockCount * paritySize());
    outputIt.insert(detail::Codec::Parity, output.at(detail::Codec::Parity).begin(), paritySize());
  }
  if (output.count(detail::Codec::Syst)) {
    output.at(detail::Codec::Syst).resize(blockCount * systSize());
    outputIt.insert(detail::Codec::Syst, output.at(detail::Codec::Syst).begin(), systSize());
  }
  if (output.count(detail::Codec::State)) {
    output.at(detail::Codec::State).resize(blockCount * stateSize());
    outputIt.insert(detail::Codec::State, output.at(detail::Codec::State).begin(), stateSize());
  }
  if (output.count(detail::Codec::Msg)) {
    output.at(detail::Codec::Msg).resize(blockCount * msgSize());
    outputIt.insert(detail::Codec::Msg, output.at(detail::Codec::Msg).begin(), msgSize());
  }
  
  workGroup_.execute(begin, end, outputIt, std::bind(&Codec::soDecodeBlocks, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

template <typename Archive>
void fec::Codec::serialize(Archive & ar, const unsigned int version) {
  using namespace boost::serialization;
  ar & ::BOOST_SERIALIZATION_NVP(workGroup_);
  ar & ::BOOST_SERIALIZATION_NVP(structure_);
}

#endif