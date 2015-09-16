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
 
 Declaration of Codec class
 ******************************************************************************/

#ifndef CODEC_H
#define CODEC_H

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

#include "Structure/BitField.h"
#include "Structure/LlrMetrics.h"

namespace fec {
/**
 *  This class represents a general encoder / decoder.
 *  It offers methods to encode and to decode data given a code structure.
 */
class Codec
{
  friend class boost::serialization::access;
public:

  enum DecoderAlgorithm {
    Exact, /**< No approximation is used and the L-values are computed in logarithmic domain. */
    Table, /**< A lookup table is used  */
    Approximate,  /**< An approximation is used */
  };
  enum MetricType {
    Floating, /**< Floating point is used in decoding. */
    Fixed, /**< Fixed type */
  };
  
  /**
   *  This class represents a general code structure
   *  It provides a usefull interface to store and acces the code information.
   */
  class Structure {
    friend class boost::serialization::access;
  public:
    /**
     *  This enum lists the implemented code structures.
     */
    enum Type {
      Convolutional, /**< Convolutional code following a trellis structure */
      Turbo,  /**< Parallel concatenated convolutional codes */
      Ldpc  /**< Low-density parity check code */
    };
    
    virtual ~Structure() = default;
    
    virtual const char * get_key() const = 0; /**< Access the type info key. */
    virtual Type type() const = 0; /**< Access the code structure type as an enumerated type. */
    
    inline size_t msgSize() const {return msgSize_;} /**< Access the size of the msg in each code bloc. */
    inline size_t systSize() const {return systSize_;} /**< Access the size of the msg in each code bloc. */
    inline size_t paritySize() const {return paritySize_;} /**< Access the size of the parity in each code bloc. */
    inline size_t stateSize() const {return stateSize_;} /**< Access the size of the extrinsic in each code bloc. */
    DecoderAlgorithm decoderAlgorithm() const {return decoderAlgorithm_;}
    
    virtual void encode(std::vector<BitField<bool>>::const_iterator msg, std::vector<BitField<uint8_t>>::iterator parity) const = 0;
    virtual bool check(std::vector<BitField<uint8_t>>::const_iterator parity) const = 0;
    
  protected:
    Structure() = default;
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version) {
      using namespace boost::serialization;
      ar & BOOST_SERIALIZATION_NVP(msgSize_);
      ar & BOOST_SERIALIZATION_NVP(systSize_);
      ar & BOOST_SERIALIZATION_NVP(paritySize_);
      ar & BOOST_SERIALIZATION_NVP(stateSize_);
      ar & BOOST_SERIALIZATION_NVP(decoderAlgorithm_);
    }
    
    size_t msgSize_;/**< Size of the msg in each code bloc. */
    size_t systSize_;/**< Size of the msg in each code bloc. */
    size_t paritySize_;/**< Size of the parity in each code bloc. */
    size_t stateSize_;/**< Size of the extrinsic in each code bloc. */
    DecoderAlgorithm decoderAlgorithm_;
  };
  template <class Iterator>
  class InfoIterator {
  public:
    InfoIterator(const Structure* structureRef) : structureRef_(structureRef) {}
    
    InfoIterator& syst(Iterator syst) {syst_ = syst; hasSyst_ = true; return *this;}
    InfoIterator& parity(Iterator parity) {parity_ = parity; hasParity_ = true; return *this;}
    InfoIterator& state(Iterator state) {state_ = state; hasState_ = true; return *this;}
    
    inline void operator++() {
      syst_ += structureRef_->systSize();
      parity_ += structureRef_->paritySize();
      state_ += structureRef_->stateSize();
    }
    inline void operator+=(size_t x) {
      syst_ += structureRef_->systSize() * x;
      parity_ += structureRef_->paritySize() * x;
      state_ += structureRef_->stateSize() * x;
    }
    inline bool operator != (const InfoIterator& b) {
      if (structureRef_ != b.structureRef_) {
        return true;
      }
      if (hasSyst() && (syst_ != b.syst_)) {
        return true;
      }
      else if (hasParity() && (parity_ != b.parity_)) {
        return true;
      }
      else if (hasState() && (state_ != b.state_)) {
        return true;
      }
      return false;
    }
    
    Iterator syst() const {return syst_;}
    Iterator parity() const {return parity_;}
    Iterator state() const {return state_;}
    
    bool hasSyst() const {return hasSyst_;}
    bool hasParity() const {return hasParity_;}
    bool hasState() const {return hasState_;}
    
  private:
    Iterator syst_;
    Iterator parity_;
    Iterator state_;
    bool hasSyst_ = false;
    bool hasParity_ = false;
    bool hasState_ = false;
    const Structure* structureRef_;
  };
  using InputIterator = InfoIterator<std::vector<LlrType>::const_iterator>;
  using OutputIterator = InfoIterator<std::vector<LlrType>::iterator>;
  
  template <class Vector>
  class Info {
  public:
    using Iterator = InfoIterator<decltype(std::declval<Vector>().begin())>;
    
    Info() = default;
    
    Info& syst(Vector& syst) {syst_ = &syst; return *this;}
    Info& parity(Vector& parity) {parity_ = &parity; return *this;}
    Info& state(Vector& state) {state_ = &state; return *this;}
    
    Vector& syst() const {return *syst_;}
    Vector& parity() const {return *parity_;}
    Vector& state() const {return *state_;}
    
    bool hasSyst() const {return syst_ != nullptr;}
    bool hasParity() const {return parity_ != nullptr;}
    bool hasState() const {return state_ != nullptr;}
    
    Iterator begin(const Structure& structure) const {
      auto it = Iterator(&structure);
      if (hasSyst()) {
        it.syst(syst().begin());
      }
      if (hasParity()) {
        it.parity(parity().begin());
      }
      if (hasState()) {
        it.state(state().begin());
      }
      return it;
    }
    Iterator end(const Structure& structure) const {
      auto it = Iterator(&structure);
      if (hasSyst()) {
        it.syst(syst().end());
      }
      if (hasParity()) {
        it.parity(parity().end());
      }
      if (hasState()) {
        it.state(state().end());
      }
      return it;
    }
    
  private:
    Vector* syst_ = nullptr;
    Vector* parity_ = nullptr;
    Vector* state_ = nullptr;
  };
  
  template <template <typename> class A = std::allocator>
  using Input = Info<const std::vector<LlrType,A<LlrType>>>;
  template <template <typename> class A = std::allocator>
  using Output = Info<std::vector<LlrType,A<LlrType>>>;
  
  static std::unique_ptr<Codec> create(const Structure& Structure, int workGroupdSize = 4);
  virtual ~Codec() = default;
  
  virtual const char * get_key() const = 0; /**< Access the type info key. */
  
  inline const Structure& structure() const {return *(structureRef_);}
  
  inline size_t msgSize() const {return structure().msgSize();} /**< Access the size of the msg in each code bloc. */
  inline size_t systSize() const {return structure().systSize();} /**< Access the size of the msg in each code bloc. */
  inline size_t paritySize() const {return structure().paritySize();} /**< Access the size of the parity in each code bloc. */
  inline size_t stateSize() const {return structure().stateSize();} /**< Access the size of the extrinsic in each code bloc. */
  
  int getWorkGroupSize() const {return workGroupSize_;}
  void setWorkGroupSize(int size) {workGroupSize_ = size;}
  
  template <template <typename> class A> bool check(std::vector<BitField<uint8_t>,A<BitField<uint8_t>>>& parity) const;
  template <template <typename> class A> void encode(const std::vector<BitField<bool>,A<BitField<bool>>>& message, std::vector<BitField<uint8_t>,A<BitField<uint8_t>>>& parity) const;
  
  template <template <typename> class A>
  void decode(const std::vector<LlrType,A<LlrType>>& parityIn, std::vector<BitField<bool>,A<BitField<bool>>>& msgOut) const;
  
  template <template <typename> class A>
  void soDecode(Input<A> input, Output<A> output) const;

protected:
  Codec() = default;
  Codec(Structure* structure, int workGroupdSize = 4);
  Codec(const Codec& other) {*this = other;}
  Codec& operator=(const Codec& other) {workGroupSize_ = other.workGroupSize(); return *this;}
  
  inline int workGroupSize() const {return workGroupSize_;}
  
  virtual bool checkBlocks(std::vector<BitField<uint8_t>>::const_iterator parity, size_t n) const;
  /**
   *  Encodes several blocs of msg bits.
   *  \param  messageIt  Input iterator pointing to the first element in the msg bit sequence.
   *  \param  parityIt[out] Output iterator pointing to the first element in the parity bit sequence.
   *    The output neeeds to be pre-allocated.
   */
  virtual void encodeBlocks(std::vector<BitField<bool>>::const_iterator msg, std::vector<BitField<uint8_t>>::iterator parity, size_t n) const;
  
  /**
   *  Decodes several blocks of information bits.
   *  \param  parityIn  Input iterator pointing to the first element
   *    in the parity L-value sequence
   *  \param  messageOut[out] Output iterator pointing to the first element
   *    in the decoded msg sequence.
   *    Output needs to be pre-allocated.
   */
  virtual void decodeBlocks(std::vector<LlrType>::const_iterator parity, std::vector<BitField<bool>>::iterator msg, size_t n) const = 0;
  /**
   *  Decodes several blocks of information bits.
   *  A posteriori information about the msg is output instead of the decoded bit sequence.
   *  \param  parityIn  Input iterator pointing to the first element
   *    in the parity L-value sequence
   *  \param  messageOut[out] Output iterator pointing to the first element
   *    in the a posteriori information L-value sequence.
   *    Output needs to be pre-allocated.
   */
  virtual void soDecodeBlocks(InputIterator input, OutputIterator output, size_t n) const = 0;
  
private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    using namespace boost::serialization;
    ar & ::BOOST_SERIALIZATION_NVP(workGroupSize_);
    ar & ::BOOST_SERIALIZATION_NVP(structureRef_);
  }
  
  std::vector<std::thread> createWorkGroup() const {
    std::vector<std::thread> threadGroup;
    threadGroup.reserve(workGroupSize());
    return threadGroup;
  }
  size_t taskSize(size_t blockCount) const {
    int n = std::thread::hardware_concurrency();
    if (n > workGroupSize() || n == 0) {
      n = workGroupSize();
    }
    return (blockCount+n-1)/n;
  }
  
  int workGroupSize_;
  Structure* structureRef_;
};
  
}

BOOST_CLASS_TYPE_INFO(fec::Codec::Structure,extended_type_info_no_rtti<fec::Codec::Structure>);
BOOST_CLASS_EXPORT_KEY(fec::Codec::Structure);
BOOST_SERIALIZATION_ASSUME_ABSTRACT(fec::Codec);
BOOST_CLASS_TYPE_INFO(fec::Codec,extended_type_info_no_rtti<fec::Codec>);
BOOST_CLASS_EXPORT_KEY(fec::Codec);

template <template <typename> class A>
bool fec::Codec::check(std::vector<BitField<uint8_t>,A<BitField<uint8_t>>>& parity) const
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
 *  \param  parity[out] Vector containing parity bits
 *  \tparam A Container allocator. The reason for different allocator is to allow 
 *    the matlab API to use a custom mex allocator
 */
template <template <typename> class A>
void fec::Codec::encode(const std::vector<BitField<bool>,A<BitField<bool>>>& msg, std::vector<BitField<uint8_t>,A<BitField<uint8_t>>>& parity) const
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
void fec::Codec::decode(const std::vector<LlrType,A<LlrType>>& parity, std::vector<BitField<bool>,A<BitField<bool>>>& msg) const
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
    throw std::invalid_argument("Input must contains parities");
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

#endif