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

#ifndef FEC_PUNCTURED_LDPC_H
#define FEC_PUNCTURED_LDPC_H

#include <boost/serialization/export.hpp>

#include "Ldpc.h"

namespace fec {
  
  class PuncturedLdpc : public Ldpc
  {
    friend class boost::serialization::access;
  public:
    struct Options : public EncoderOptions, DecoderOptions, PunctureOptions
    {
    public:
      Options(const SparseBitMatrix& checkMatrix) : EncoderOptions(checkMatrix) {}
    };
    
    struct detail {
      /**
       *  This class represents a convolutional code structure.
       *  It provides a usefull interface to store and acces the structure information.
       */
      class Structure : public Ldpc::detail::Structure {
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
          ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(Ldpc::detail::Structure);
          ar & ::BOOST_SERIALIZATION_NVP(permutation_);
        }
        
        Permutation permutation_;
      };
    };
    
    PuncturedLdpc(const Options& options, int workGroupSize = 8);
    PuncturedLdpc(const detail::Structure& structure, int workGroupSize = 8);
    PuncturedLdpc(const EncoderOptions& encoder, const PunctureOptions& puncture, const DecoderOptions& decoder, int workGroupSize = 8);
    PuncturedLdpc(const EncoderOptions& encoder, const PunctureOptions& puncture, int workGroupSize = 8);
    PuncturedLdpc(const PuncturedLdpc& other) {*this = other;}
    virtual ~PuncturedLdpc() = default;
    PuncturedLdpc& operator=(const PuncturedLdpc& other) {Codec::operator=(other); structure_ = std::unique_ptr<detail::Structure>(new detail::Structure(other.structure())); return *this;}
    
    virtual const char * get_key() const;
    
    void setPunctureOptions(const PunctureOptions& puncture) {structure().setPunctureOptions(puncture);}
    
  protected:
    PuncturedLdpc() = default;
    
    inline const detail::Structure& structure() const {return dynamic_cast<const detail::Structure&>(Codec::structure());}
    inline detail::Structure& structure() {return dynamic_cast<detail::Structure&>(Codec::structure());}
    
    virtual void decodeBlocks(std::vector<double>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg, size_t n) const;
    virtual void soDecodeBlocks(Codec::detail::InputIterator input, Codec::detail::OutputIterator output, size_t n) const;
    
  private:
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version) {
      using namespace boost::serialization;
      ar.template register_type<detail::Structure>();
      ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(Ldpc);
    }
  };
  
}

BOOST_CLASS_EXPORT_KEY(fec::PuncturedLdpc);
BOOST_CLASS_TYPE_INFO(fec::PuncturedLdpc,extended_type_info_no_rtti<fec::PuncturedLdpc>);
BOOST_CLASS_EXPORT_KEY(fec::PuncturedLdpc::detail::Structure);
BOOST_CLASS_TYPE_INFO(fec::PuncturedLdpc::detail::Structure,extended_type_info_no_rtti<fec::PuncturedLdpc::detail::Structure>);


#endif
