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

#ifndef FEC_DETAIL_CONVOLUTIONAL_H
#define FEC_DETAIL_CONVOLUTIONAL_H

#include "Codec.h"
#include "../BitField.h"
#include "../Trellis.h"
#include "../Permutation.h"

namespace fec {
  
  namespace detail {
    
    namespace Convolutional {
      
      struct EncoderOptions {
        friend class Structure;
      public:
        EncoderOptions(const Trellis& trellis, size_t length) {trellis_ = trellis; length_ = length;}
        EncoderOptions& termination(Trellis::Termination type) {termination_ = type; return *this;}
        EncoderOptions& msgWidth(size_t width) {msgWidth_ = width; return *this;}
        EncoderOptions& parityWidth(size_t width) {parityWidth_ = width; return *this;}
        
      private:
        Trellis trellis_;
        size_t length_;
        Trellis::Termination termination_ = Trellis::Truncate;
        size_t msgWidth_ = 0;
        size_t parityWidth_ = 0;
      };
      
      struct DecoderOptions {
        friend class Structure;
      public:
        DecoderOptions() = default;
        
        DecoderOptions& algorithm(DecoderAlgorithm algorithm) {algorithm_ = algorithm; return *this;}
        DecoderOptions& scalingFactor(double scalingFactor) {scalingFactor_ = scalingFactor; return *this;}
        
        DecoderAlgorithm algorithm() const {return algorithm_;}
        double scalingFactor() const {return scalingFactor_;}
        
      private:
        DecoderAlgorithm algorithm_ = Approximate;
        double scalingFactor_ = 1.0;
      };
      
      struct PunctureOptions {
        friend class Structure;
      public:
        PunctureOptions() = default;
        
        PunctureOptions& mask(std::vector<bool> mask) {mask_ = mask; return *this;}
        PunctureOptions& tailMask(std::vector<bool> mask) {tailMask_ = mask; return *this;}
        
      private:
        std::vector<bool> mask_;
        std::vector<bool> tailMask_;
      };
      
      /**
       *  This class represents a convolutional codec structure.
       *  It provides a usefull interface to store and acces the structure information.
       */
      class Structure : public Codec::Structure {
        friend class ::boost::serialization::access;
      public:
        Structure() = default;
        Structure(const EncoderOptions& encoder, const DecoderOptions& decoder);
        Structure(const EncoderOptions& encoder);
        ~Structure() = default;
        
        const char * get_key() const override;
        
        void setDecoderOptions(const DecoderOptions& decoder);
        DecoderOptions getDecoderOptions() const;
        Permutation puncturing(const PunctureOptions& options) const;
        
        size_t msgWidth() const override {return 1;} /**< Access the width of msg in each code bloc. */
        size_t systWidth() const override {return 1;} /**< Access the width of systematics in each code bloc. */
        size_t parityWidth() const override {return 1;} /**< Access the width of parities in each code bloc. */
        size_t stateWidth() const override {return 1;} /**< Access the width of state information in each code bloc. */
        
        size_t msgSize() const override {return trellis().inputWidth()*length();} /**< Access the size of msg in each code bloc. */
        size_t systSize() const override {return trellis().inputWidth()*(length()+tailLength());} /**< Access the size of systematics in each code bloc. */
        size_t paritySize() const override {return trellis().outputWidth()*(length()+tailLength());} /**< Access the size of parities in each code bloc. */
        size_t stateSize() const override {return 0;} /**< Access the size of state information in each code bloc. */
        
        inline size_t length() const {return length_;}
        inline size_t tailLength() const {return tailLength_;}
        inline size_t tailSize() const {return tailLength_ * trellis().inputWidth();}
        inline Trellis::Termination termination() const {return termination_;}
        inline const Trellis& trellis() const {return trellis_;}
        
        double scalingFactor() const {return scalingFactor_;} /**< Access the scalingFactor value used in decoder. */
        void setScalingFactor(double factor) {scalingFactor_ = factor;} /**< Modify the scalingFactor value used in decoder. */
        
        template <class InputIterator>
        bool check(InputIterator parity) const;
        template <class InputIterator, class OutputIterator>
        void encode(InputIterator msg, OutputIterator parity) const;
        template <class InputIterator, class OutputIterator1, class OutputIterator2>
        void encode(InputIterator msg, OutputIterator1 parity, OutputIterator2 tail) const;
        
      protected:
        void setEncoderOptions(const EncoderOptions& encoder);
        
      private:
        template <typename Archive>
        void serialize(Archive & ar, const unsigned int version);
        
        Trellis trellis_;
        size_t length_;
        size_t tailLength_;
        Trellis::Termination termination_;
        double scalingFactor_;
      };
      
    }
    
  }
  
}


BOOST_CLASS_TYPE_INFO(fec::detail::Convolutional::Structure,extended_type_info_no_rtti<fec::detail::Convolutional::Structure>);
BOOST_CLASS_EXPORT_KEY(fec::detail::Convolutional::Structure);

template <class InputIterator>
bool fec::detail::Convolutional::Structure::check(InputIterator parity) const
{
  size_t state = 0;
  for (int j = 0; j < length()+tailLength(); ++j) {
    bool found = false;
    for (BitField<size_t> input = 0; input < trellis().inputCount(); ++input) {
      BitField<size_t> output = trellis().getOutput(state, input);
      bool equal = true;
      for (int k = 0; k < trellis().outputWidth(); ++k) {
        if (output[k] != parity[k]) {
          equal = false;
          break;
        }
      }
      if (equal == true) {
        found = true;
        state = trellis().getNextState(state, input);
        break;
      }
    }
    if (found == false) {
      return false;
    }
    parity += trellis().outputWidth();
  }
  switch (termination()) {
    case Trellis::Tail:
      return (state == 0);
      
    default:
    case Trellis::Truncate:
      return true;
  }
}

template <class InputIterator, class OutputIterator>
void fec::detail::Convolutional::Structure::encode(InputIterator msg, OutputIterator parity) const
{
  std::vector<BitField<size_t>> tail(tailLength()*trellis().inputWidth());
  encode(msg, parity, tail.begin());
}

template <class InputIterator, class OutputIterator1, class OutputIterator2>
void fec::detail::Convolutional::Structure::encode(InputIterator msg, OutputIterator1 parity, OutputIterator2 tail) const
{
  size_t state = 0;
  
  for (int j = 0; j < length(); ++j) {
    BitField<size_t> input = 0;
    for (int k = 0; k < trellis().inputWidth(); k++) {
      input.set(k, msg[k]);
    }
    msg += trellis().inputWidth();
    
    BitField<size_t> output = trellis().getOutput(state, input);
    state = trellis().getNextState(state, input);
    
    for (int k = 0; k < trellis().outputWidth(); k++) {
      parity[k] = output.test(k);
    }
    parity += trellis().outputWidth();
  }
  
  switch (termination()) {
    case Trellis::Tail:
      for (int j = 0; j < tailLength(); ++j) {
        int maxCount = -1;
        BitField<size_t> bestInput = 0;
        for (BitField<size_t> input = 0; input < trellis().inputCount(); ++input) {
          BitField<size_t> nextState = trellis().getNextState(state, input);
          int count = weigth(BitField<size_t>(state)) - weigth(nextState);
          if (count > maxCount) {
            maxCount = count;
            bestInput = input;
          }
        }
        BitField<size_t> nextState = trellis().getNextState(state, bestInput);
        BitField<size_t> output = trellis().getOutput(state, bestInput);
        for (int k = 0; k < trellis().outputWidth(); ++k) {
          parity[k] = output.test(k);
        }
        for (int k = 0; k < trellis().inputWidth(); ++k) {
          tail[k] = bestInput.test(k);
        }
        parity += trellis().outputWidth();
        tail += trellis().inputWidth();
        state = nextState;
      }
      break;
      
    default:
    case Trellis::Truncate:
      state = 0;
      break;
  }
}

template <typename Archive>
void fec::detail::Convolutional::Structure::serialize(Archive & ar, const unsigned int version) {
  using namespace boost::serialization;
  ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(Codec::Structure);
  ar & ::BOOST_SERIALIZATION_NVP(trellis_);
  ar & ::BOOST_SERIALIZATION_NVP(termination_);
  ar & ::BOOST_SERIALIZATION_NVP(tailLength_);
  ar & ::BOOST_SERIALIZATION_NVP(length_);
  ar & ::BOOST_SERIALIZATION_NVP(scalingFactor_);
}

#endif
