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
       
        virtual size_t msgWidth() const = 0; /**< Access the size of msg in each code bloc. */
        virtual size_t systWidth() const = 0; /**< Access the size of systematics in each code bloc. */
        virtual size_t parityWidth() const = 0; /**< Access the size of parities in each code bloc. */
        virtual size_t stateWidth() const = 0; /**< Access the size of state information in each code bloc. */
        
        virtual size_t msgSize() const = 0; /**< Access the size of msg in each code bloc. */
        virtual size_t systSize() const = 0; /**< Access the size of systematics in each code bloc. */
        virtual size_t paritySize() const = 0; /**< Access the size of parities in each code bloc. */
        virtual size_t stateSize() const = 0; /**< Access the size of state information in each code bloc. */
        
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
        DecoderAlgorithm decoderAlgorithm_; /**< Algorithm type used in decoder. */
        
      private:
        template <typename Archive>
        void serialize(Archive & ar, const unsigned int version);
      };
      
    }
    
  }
  
}


BOOST_CLASS_TYPE_INFO(fec::detail::Codec::Structure,extended_type_info_no_rtti<fec::detail::Codec::Structure>);
BOOST_CLASS_EXPORT_KEY(fec::detail::Codec::Structure);


template <typename Archive>
void fec::detail::Codec::Structure::serialize(Archive & ar, const unsigned int version) {
  using namespace boost::serialization;
  ar & BOOST_SERIALIZATION_NVP(decoderAlgorithm_);
}

#endif