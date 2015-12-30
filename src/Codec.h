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
#include <boost/serialization/export.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/extended_type_info_no_rtti.hpp>

#include "detail/Codec.h"
#include "detail/CodecFunction.h"
#include "detail/MultiIterator.h"
#include "detail/WorkGroup.h"
#include "detail/rebind.h"

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
    
    struct Input {
      template <typename T> static detail::Codec::ConstArguments<T> msg(T& msg) {return detail::Codec::ConstArguments<T>{}.msg(msg);}
      template <typename T> static detail::Codec::ConstArguments<T> syst(T& syst) {return detail::Codec::ConstArguments<T>{}.syst(syst);}
      template <typename T> static detail::Codec::ConstArguments<T> parity(T& parity) {return detail::Codec::ConstArguments<T>{}.parity(parity);}
      template <typename T> static detail::Codec::ConstArguments<T> state(T& state) {return detail::Codec::ConstArguments<T>{}.state(state);}
    } Input;
    struct Output {
      template <typename T> static detail::Codec::Arguments<T> msg(T& msg) {return detail::Codec::Arguments<T>{}.msg(msg);}
      template <typename T> static detail::Codec::Arguments<T> syst(T& syst) {return detail::Codec::Arguments<T>{}.syst(syst);}
      template <typename T> static detail::Codec::Arguments<T> parity(T& parity) {return detail::Codec::Arguments<T>{}.parity(parity);}
      template <typename T> static detail::Codec::Arguments<T> state(T& state) {return detail::Codec::Arguments<T>{}.state(state);}
    } Output;
    
    virtual ~Codec() = default;
    virtual const char * get_key() const = 0; /**< Access the type info key. */
    
    inline size_t msgCount() const {return 1<<structure().msgWidth();} /**< Access the size of the msg in each code bloc. */
    inline size_t systCount() const {return 1<<structure().systWidth();} /**< Access the size of the msg in each code bloc. */
    inline size_t parityCount() const {return 1<<structure().parityWidth();} /**< Access the size of the parity in each code bloc. */
    inline size_t stateCount() const {return 1<<structure().stateWidth();} /**< Access the size of the extrinsic in each code bloc. */

    inline size_t msgWidth() const {return structure().msgWidth();} /**< Access the size of the msg in each code bloc. */
    inline size_t systWidth() const {return structure().systWidth();} /**< Access the size of the msg in each code bloc. */
    inline size_t parityWidth() const {return structure().parityWidth();} /**< Access the size of the parity in each code bloc. */
    inline size_t stateWidth() const {return structure().stateWidth();} /**< Access the size of the extrinsic in each code bloc. */

    inline size_t msgSize() const {return structure().msgSize();} /**< Access the size of the msg in each code bloc. */
    inline size_t systSize() const {return structure().systSize();} /**< Access the size of the msg in each code bloc. */
    inline size_t paritySize() const {return structure().paritySize();} /**< Access the size of the parity in each code bloc. */
    inline size_t stateSize() const {return structure().stateSize();} /**< Access the size of the extrinsic in each code bloc. */
    
    int getWorkGroupSize() const {return workGroupSize_;}
    void setWorkGroupSize(int size) {workGroupSize_ = size;}
    
    template <class Vector>
    bool check(const Vector& parity) const;
    
    template <class Vector>
    void encode(const Vector& message, Vector& parity) const;
    template <class Vector>
    Vector encode(const Vector& message) const;
    
    template <class InputVector, class OutputVector>
    void decode(const InputVector& parity, OutputVector& msg) const;
    template <class InputVector, class OutputVector = typename detail::rebind<InputVector, BitField<size_t>>::type>
    OutputVector decode(const InputVector& parity) const;
    
    template <class Vector>
    void soDecode(detail::Codec::Arguments<const Vector> input, detail::Codec::Arguments<Vector> output) const;
    
  protected:
    Codec() = default;
    Codec(std::unique_ptr<detail::Codec::Structure>&&);
    
    Codec(const Codec& other) {*this = other;}
    Codec& operator=(const Codec& other) {workGroupSize_ = other.workGroupSize_; return *this;}
    
    inline const detail::Codec::Structure& structure() const {return *structure_;}
    inline detail::Codec::Structure& structure() {return *structure_;}
    
    std::unique_ptr<detail::Codec::Structure> structure_;
    
  private:
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version);
    
    template <class InputIterator, class OutputIterator>
    void encodeBlocks(detail::Codec::iterator<InputIterator> msgf, detail::Codec::iterator<InputIterator> msgl, detail::Codec::iterator<OutputIterator> parity) const;
    template <class InputIterator, class OutputIterator>
    void decodeBlocks(detail::Codec::iterator<InputIterator> parityf, detail::Codec::iterator<InputIterator> parityl, detail::Codec::iterator<OutputIterator> msg) const;
    template <class InputIterator, class OutputIterator>
    void soDecodeBlocks(detail::Codec::iterator<InputIterator> first, detail::Codec::iterator<InputIterator> last, detail::Codec::iterator<OutputIterator> output) const;
    
    int workGroupSize_ = 0;
  };
  
}



BOOST_SERIALIZATION_ASSUME_ABSTRACT(fec::Codec);
BOOST_CLASS_TYPE_INFO(fec::Codec,extended_type_info_no_rtti<fec::Codec>);
BOOST_CLASS_EXPORT_KEY(fec::Codec);

template <class Vector>
bool fec::Codec::check(const Vector& parity) const
{
  uint64_t blockCount = parity.size() / (paritySize());
  if (parity.size() != blockCount * paritySize()) {
    throw std::invalid_argument("Invalid size for parity");
  }
  detail::Codec::iterator<typename Vector::const_iterator> first{{detail::Codec::Parity, parity.begin(), paritySize()}};
  detail::Codec::iterator<typename Vector::const_iterator> last{{detail::Codec::Parity, parity.end(), paritySize()}};
  auto check = detail::Codec::CheckFunction<typename Vector::const_iterator>::create(structure());
  return (*check)(first, last);
}

/**
 *  Encodes several blocks of information bits.
 *  Chunks of blocs are encded in parallel.
 *  \param  message  Vector containing information bits
 *  \return Vector containing parity bits
 *  \tparam A Container allocator. The reason for different allocator is to allow
 *    the matlab API to use a custom mex allocator
 */
template <class Vector>
Vector fec::Codec::encode(const Vector& message) const
{
  Vector parity;
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
template <class Vector>
void fec::Codec::encode(const Vector& msg, Vector& parity) const
{
  uint64_t blockCount = msg.size() / (msgSize());
  if (msg.size() != blockCount * msgSize()) {
    throw std::invalid_argument("Invalid size for message");
  }
  parity.resize(blockCount * paritySize(), 0);
  
  detail::Codec::iterator<typename Vector::const_iterator> begin{{detail::Codec::Msg, msg.begin(), msgSize()}};
  detail::Codec::iterator<typename Vector::const_iterator> end{{detail::Codec::Msg, msg.end(), msgSize()}};
  detail::Codec::iterator<typename Vector::iterator> output{{detail::Codec::Parity, parity.begin(), paritySize()}};
  
  detail::WorkGroup workGroup(workGroupSize_);
  workGroup.executeTask(std::bind(&Codec::encodeBlocks<typename Vector::const_iterator, typename Vector::iterator>, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), begin, end, output);
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
template <class InputVector, class OutputVector>
OutputVector fec::Codec::decode(const InputVector& parity) const
{
  OutputVector message;
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
template <class InputVector, class OutputVector>
void fec::Codec::decode(const InputVector& parity, OutputVector& msg) const
{
  size_t blockCount = parity.size() / paritySize();
  if (parity.size() != blockCount * paritySize()) {
    throw std::invalid_argument("Invalid size for parity");
  }
  
  msg.resize(blockCount * msgSize());
  
  detail::Codec::iterator<typename InputVector::const_iterator> begin{{detail::Codec::Parity, parity.begin(), paritySize()}};
  detail::Codec::iterator<typename InputVector::const_iterator> end{{detail::Codec::Parity, parity.end(), paritySize()}};
  detail::Codec::iterator<typename OutputVector::iterator> output{{detail::Codec::Msg, msg.begin(), msgSize()}};
  
  detail::WorkGroup workGroup(workGroupSize_);
  workGroup.executeTask(std::bind(&Codec::decodeBlocks<typename InputVector::const_iterator, typename OutputVector::iterator>, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), begin, end, output);
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
template <class Vector>
void fec::Codec::soDecode(detail::Codec::Arguments<const Vector> input, detail::Codec::Arguments<Vector> output) const
{
  if (!input.count(detail::Codec::Parity)) {
    throw std::invalid_argument("Input must contains parity");
  }
  if (input.count(detail::Codec::Msg)) {
    throw std::invalid_argument("Input should not contain msg");
  }
  size_t blockCount = input.at(detail::Codec::Parity).size() / paritySize();

  detail::Codec::iterator<typename Vector::const_iterator> begin{{detail::Codec::Parity, input.at(detail::Codec::Parity).begin(), paritySize()}};
  detail::Codec::iterator<typename Vector::const_iterator> end{{detail::Codec::Parity, input.at(detail::Codec::Parity).end(), paritySize()}};
  detail::Codec::iterator<typename Vector::iterator> outputIt;
  
  if (input.at(detail::Codec::Parity).size() != blockCount * paritySize()) {
    throw std::invalid_argument("Invalid size for parity");
  }
  if (input.count(detail::Codec::Syst)) {
    begin.insert(detail::Codec::Syst, input.at(detail::Codec::Syst).begin(), systSize());
    end.insert(detail::Codec::Syst, input.at(detail::Codec::Syst).end(), systSize());
    if (input.at(detail::Codec::Syst).size() != blockCount * systSize()) {
      throw std::invalid_argument("Invalid size for syst");
    }
  }
  if (input.count(detail::Codec::State)) {
    begin.insert(detail::Codec::State, input.at(detail::Codec::State).begin(), stateSize());
    end.insert(detail::Codec::State, input.at(detail::Codec::State).end(), stateSize());
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
  
  detail::WorkGroup workGroup(workGroupSize_);
  workGroup.executeTask(std::bind(&Codec::soDecodeBlocks<typename Vector::const_iterator, typename Vector::iterator>, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), begin, end, outputIt);
}

template <class InputIterator, class OutputIterator>
void fec::Codec::encodeBlocks(detail::Codec::iterator<InputIterator> msgf, detail::Codec::iterator<InputIterator> msgl, detail::Codec::iterator<OutputIterator> parity) const
{
  auto encode = detail::Codec::EncodeFunction<InputIterator, OutputIterator>::create(structure());
  (*encode)(msgf, msgl, parity);
}

template <class InputIterator, class OutputIterator>
void fec::Codec::decodeBlocks(detail::Codec::iterator<InputIterator> parityf, detail::Codec::iterator<InputIterator> parityl, detail::Codec::iterator<OutputIterator> msg) const
{
  auto decode = detail::Codec::DecodeFunction<InputIterator, OutputIterator>::create(structure());
  (*decode)(parityf, parityl, msg);
}

template <class InputIterator, class OutputIterator>
void fec::Codec::soDecodeBlocks(detail::Codec::iterator<InputIterator> parityf, detail::Codec::iterator<InputIterator> parityl, detail::Codec::iterator<OutputIterator> msg) const
{
  auto soDecode = detail::Codec::SoDecodeFunction<InputIterator, OutputIterator>::create(structure());
  (*soDecode)(parityf, parityl, msg);
}

template <typename Archive>
void fec::Codec::serialize(Archive & ar, const unsigned int version) {
  using namespace boost::serialization;
  ar & ::BOOST_SERIALIZATION_NVP(workGroupSize_);
  ar & ::BOOST_SERIALIZATION_NVP(structure_);
}

#endif