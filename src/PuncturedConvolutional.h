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

#ifndef FEC_PUNCTURED_CONVOLUTIONAL_H
#define FEC_PUNCTURED_CONVOLUTIONAL_H

#include <boost/serialization/export.hpp>

#include "Convolutional.h"

namespace fec {
  
  class PuncturedConvolutional : public Convolutional
  {
    friend class boost::serialization::access;
  public:
    struct Options : public EncoderOptions, DecoderOptions, PunctureOptions
    {
    public:
      Options(const Trellis& trellis, size_t length) : EncoderOptions(trellis, length) {}
    };
    
    struct detail {
      /**
       *  This class represents a convolutional code structure.
       *  It provides a usefull interface to store and acces the structure information.
       */
      class Structure : public Convolutional::detail::Structure {
        friend class ::boost::serialization::access;
      public:
        Structure() = default;
        Structure(const Options& options);
        Structure(const EncoderOptions&, const PunctureOptions&, const DecoderOptions&);
        Structure(const EncoderOptions&, const PunctureOptions&);
        virtual ~Structure() = default;
        
        virtual const char * get_key() const;
        
        virtual size_t paritySize() const {return permutation_.outputSize();}
        
        void setPunctureOptions(const PunctureOptions& puncture);
        
        inline Permutation permutation() const {return permutation_;}
        
        virtual bool check(std::vector<BitField<size_t>>::const_iterator parity) const;
        virtual void encode(std::vector<BitField<size_t>>::const_iterator msg, std::vector<BitField<size_t>>::iterator parity) const;
        
      private:
        template <typename Archive>
        void serialize(Archive & ar, const unsigned int version) {
          using namespace boost::serialization;
          ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(Convolutional::detail::Structure);
          ar & ::BOOST_SERIALIZATION_NVP(permutation_);
        }
        
        Permutation permutation_;
      };
    };
    
    PuncturedConvolutional(const Options& options, int workGroupSize = 8);
    PuncturedConvolutional(const detail::Structure& structure, int workGroupSize = 8);
    PuncturedConvolutional(const EncoderOptions& encoder, const PunctureOptions& puncture, const DecoderOptions& decoder, int workGroupSize = 8);
    PuncturedConvolutional(const EncoderOptions& encoder, const PunctureOptions& puncture, int workGroupSize = 8);
    PuncturedConvolutional(const PuncturedConvolutional& other) {*this = other;}
    virtual ~PuncturedConvolutional() = default;
    PuncturedConvolutional& operator=(const PuncturedConvolutional& other) {Codec::operator=(other); structure_ = std::unique_ptr<detail::Structure>(new detail::Structure(other.structure())); return *this;}
    
    virtual const char * get_key() const;
    
    void setPunctureOptions(const PunctureOptions& puncture) {structure().setPunctureOptions(puncture);}
    
  protected:
    PuncturedConvolutional() = default;
    
    inline const detail::Structure& structure() const {return dynamic_cast<const detail::Structure&>(Codec::structure());}
    inline detail::Structure& structure() {return dynamic_cast<detail::Structure&>(Codec::structure());}
    
    virtual void decodeBlocks(std::vector<LlrType>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg, size_t n) const;
    virtual void soDecodeBlocks(Codec::detail::InputIterator input, Codec::detail::OutputIterator output, size_t n) const;
    
  private:
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version) {
      using namespace boost::serialization;
      ar.template register_type<detail::Structure>();
      ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(Convolutional);
    }
  };
  
}

BOOST_CLASS_EXPORT_KEY(fec::PuncturedConvolutional);
BOOST_CLASS_TYPE_INFO(fec::PuncturedConvolutional,extended_type_info_no_rtti<fec::PuncturedConvolutional>);
BOOST_CLASS_EXPORT_KEY(fec::PuncturedConvolutional::detail::Structure);
BOOST_CLASS_TYPE_INFO(fec::PuncturedConvolutional::detail::Structure,extended_type_info_no_rtti<fec::PuncturedConvolutional::detail::Structure>);


#endif
