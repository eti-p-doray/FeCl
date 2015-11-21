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
        
      private:
        Trellis trellis_;
        size_t length_;
        Trellis::Termination termination_ = Trellis::Truncate;
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
        virtual ~Structure() = default;
        
        virtual const char * get_key() const;
        
        void setDecoderOptions(const DecoderOptions& decoder);
        DecoderOptions getDecoderOptions() const;
        
        inline size_t length() const {return length_;}
        inline size_t tailSize() const {return tailSize_;}
        inline size_t systTailSize() const {return tailSize_ * trellis().inputSize();}
        inline Trellis::Termination termination() const {return termination_;}
        inline const Trellis& trellis() const {return trellis_;}
        
        double scalingFactor() const {return scalingFactor_;} /**< Access the scalingFactor value used in decoder. */
        void setScalingFactor(double factor) {scalingFactor_ = factor;} /**< Modify the scalingFactor value used in decoder. */
        
        virtual bool check(std::vector<BitField<size_t>>::const_iterator parity) const;
        virtual void encode(std::vector<BitField<size_t>>::const_iterator msg, std::vector<BitField<size_t>>::iterator parity) const;
        void encode(std::vector<BitField<size_t>>::const_iterator msg, std::vector<BitField<size_t>>::iterator parity, std::vector<BitField<size_t>>::iterator tail) const;
        
        Permutation puncturing(const PunctureOptions& options) const;
        
      protected:
        void setEncoderOptions(const EncoderOptions& encoder);
        
      private:
        template <typename Archive>
        void serialize(Archive & ar, const unsigned int version);
        
        Trellis trellis_;
        size_t length_;
        Trellis::Termination termination_;
        size_t tailSize_;
        double scalingFactor_;
      };
      
    }
    
  }
  
}


BOOST_CLASS_TYPE_INFO(fec::detail::Convolutional::Structure,extended_type_info_no_rtti<fec::detail::Convolutional::Structure>);
BOOST_CLASS_EXPORT_KEY(fec::detail::Convolutional::Structure);


template <typename Archive>
void fec::detail::Convolutional::Structure::serialize(Archive & ar, const unsigned int version) {
  using namespace boost::serialization;
  ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(Codec::Structure);
  ar & ::BOOST_SERIALIZATION_NVP(trellis_);
  ar & ::BOOST_SERIALIZATION_NVP(termination_);
  ar & ::BOOST_SERIALIZATION_NVP(tailSize_);
  ar & ::BOOST_SERIALIZATION_NVP(length_);
  ar & ::BOOST_SERIALIZATION_NVP(scalingFactor_);
}

#endif
