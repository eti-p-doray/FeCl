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

#ifndef FEC_DETAIL_CODEC_H
#define FEC_DETAIL_CODEC_H

#include <vector>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/serialization/extended_type_info_no_rtti.hpp>

#include "LlrMetrics.h"
#include "../BitField.h"
#include "../DecoderAlgorithm.h"

namespace fec {
  
  /**
   *  detail namespace. You shouln't be much interested.
   */
  namespace detail {
    
    namespace Codec {
      
      /**
       *  This class represents a general codec structure
       *  It provides a usefull interface to store and access the codec information.
       */
      class Structure {
        friend class boost::serialization::access;
      public:
        Structure() = default; /**< Default constructor. */
        virtual ~Structure() = default; /**< Default destructor. */
        
        virtual const char * get_key() const = 0; /**< Access the type info key. */
        
        size_t msgSize() const {return msgSize_;} /**< Access the size of msg in each code bloc. */
        size_t systSize() const {return systSize_;} /**< Access the size of systematics in each code bloc. */
        size_t paritySize() const {return paritySize_;} /**< Access the size of parities in each code bloc. */
        size_t stateSize() const {return stateSize_;} /**< Access the size of state information in each code bloc. */
        
        DecoderAlgorithm decoderAlgorithm() const {return decoderAlgorithm_;} /**< Access the algorithm used in decoder. */
        
        /**
         *  Encodes one block of msg bits.
         *  \param  msg  Input iterator pointing to the first element in the msg bit sequence.
         *  \param  parity[out] Output iterator pointing to the first element in the parity bit sequence.
         *    The output neeeds to be pre-allocated.
         */
        virtual void encode(std::vector<BitField<size_t>>::const_iterator msg, std::vector<BitField<size_t>>::iterator parity) const = 0;
        
        /**
         *  Checks the consistency of a parity sequence.
         *  \param  parity  Input iterator pointing to the first element in the parity bit sequence.
         *  \return  True if the sequence is consistent, false otherwise.
         */
        virtual bool check(std::vector<BitField<size_t>>::const_iterator parity) const = 0;
        
      protected:
        size_t msgSize_ = 0;/**< Size of msg in each code bloc. */
        size_t systSize_ = 0;/**< Size of systematics in each code bloc. */
        size_t paritySize_ = 0;/**< Size of parities in each code bloc. */
        size_t stateSize_ = 0;/**< Size of state information in each code bloc. */
        DecoderAlgorithm decoderAlgorithm_; /**< Algorithm type used in decoder. */
        
      private:
        template <typename Archive>
        void serialize(Archive & ar, const unsigned int version);
      };
      
      /**
       *  This class is an iterator on the codec data flow.
       */
      template <class Iterator>
      class InfoIterator {
      public:
        /**
         *  Constructor.
         *  \param  structureRef Pointer to the codec structure associated with the info.
         */
        InfoIterator(const Structure* structureRef) : structureRef_(structureRef) {}
        InfoIterator() = default;
        void setStructureRef(const Structure* structureRef) {structureRef_ = structureRef;}
        
        InfoIterator& syst(Iterator syst) {syst_ = syst; hasSyst_ = true; return *this;} /**< Link systematic extrinsics. */
        InfoIterator& parity(Iterator parity) {parity_ = parity; hasParity_ = true; return *this;} /**< Link parity extrinsics. */
        InfoIterator& state(Iterator state) {state_ = state; hasState_ = true; return *this;} /**< Link state extrinsics. */
        InfoIterator& msg(Iterator msg) {msg_ = msg; hasMsg_ = true; return *this;} /**< Link msg data. */
        
        inline void operator ++ ();
        inline void operator += (size_t x);
        inline bool operator != (const InfoIterator& b);
        
        Iterator syst() const {return syst_;}
        Iterator parity() const {return parity_;}
        Iterator state() const {return state_;}
        Iterator msg() const {return msg_;}
        
        bool hasSyst() const {return hasSyst_;}
        bool hasParity() const {return hasParity_;}
        bool hasState() const {return hasState_;}
        bool hasMsg() const {return hasMsg_;}
        
      private:
        Iterator syst_;
        Iterator parity_;
        Iterator state_;
        Iterator msg_;
        bool hasSyst_ = false;
        bool hasParity_ = false;
        bool hasState_ = false;
        bool hasMsg_ = false;
        const Structure* structureRef_;
      };
      
      using InputIterator = InfoIterator<std::vector<double>::const_iterator>;
      using OutputIterator = InfoIterator<std::vector<double>::iterator>;
      
      template <class Vector>
      class Info {
        using Iterator = InfoIterator<decltype(std::declval<Vector>().begin())>;
      public:
        Info() = default;
        
        Info& syst(Vector& syst) {syst_ = &syst; return *this;}
        Info& parity(Vector& parity) {parity_ = &parity; return *this;}
        Info& state(Vector& state) {state_ = &state; return *this;}
        Info& msg(Vector& msg) {msg_ = &msg; return *this;}
        
        Vector& syst() const {return *syst_;}
        Vector& parity() const {return *parity_;}
        Vector& state() const {return *state_;}
        Vector& msg() const {return *msg_;}
        
        bool hasSyst() const {return syst_ != nullptr;}
        bool hasParity() const {return parity_ != nullptr;}
        bool hasState() const {return state_ != nullptr;}
        bool hasMsg() const {return msg_ != nullptr;}
        
        Iterator begin(const Structure& structure) const;
        Iterator end(const Structure& structure) const;
        
      private:
        Vector* syst_ = nullptr;
        Vector* parity_ = nullptr;
        Vector* state_ = nullptr;
        Vector* msg_ = nullptr;
      };
      
    }
    
  }
  
}



BOOST_CLASS_TYPE_INFO(fec::detail::Codec::Structure,extended_type_info_no_rtti<fec::detail::Codec::Structure>);
BOOST_CLASS_EXPORT_KEY(fec::detail::Codec::Structure);


template <typename Archive>
void fec::detail::Codec::Structure::serialize(Archive & ar, const unsigned int version) {
  using namespace boost::serialization;
  ar & BOOST_SERIALIZATION_NVP(msgSize_);
  ar & BOOST_SERIALIZATION_NVP(systSize_);
  ar & BOOST_SERIALIZATION_NVP(paritySize_);
  ar & BOOST_SERIALIZATION_NVP(stateSize_);
  ar & BOOST_SERIALIZATION_NVP(decoderAlgorithm_);
}

template <class Iterator>
void fec::detail::Codec::InfoIterator<Iterator>::operator++() {
  syst_ += structureRef_->systSize();
  parity_ += structureRef_->paritySize();
  state_ += structureRef_->stateSize();
  msg_ += structureRef_->msgSize();
}

template <class Iterator>
void fec::detail::Codec::InfoIterator<Iterator>::operator+=(size_t x) {
  syst_ += structureRef_->systSize() * x;
  parity_ += structureRef_->paritySize() * x;
  state_ += structureRef_->stateSize() * x;
  msg_ += structureRef_->msgSize() * x;
}

template <class Iterator>
bool fec::detail::Codec::InfoIterator<Iterator>::operator != (const InfoIterator& b) {
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
  else if (hasMsg() && (msg_ != b.msg_)) {
    return true;
  }
  return false;
}

template <class Vector>
typename fec::detail::Codec::Info<Vector>::Iterator fec::detail::Codec::Info<Vector>::begin(const Structure& structure) const {
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
  if (hasMsg()) {
    it.msg(msg().begin());
  }
  return it;
}

template <class Vector>
typename fec::detail::Codec::Info<Vector>::Iterator fec::detail::Codec::Info<Vector>::end(const Structure& structure) const {
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
  if (hasMsg()) {
    it.msg(msg().end());
  }
  return it;
}


#endif