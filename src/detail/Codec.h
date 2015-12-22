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
#include <boost/serialization/export.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/extended_type_info_no_rtti.hpp>

#include "../DecoderAlgorithm.h"
#include "../BitField.h"
#include "MultiIterator.h"

namespace fec {
  
  /**
   *  detail namespace. You shouln't be much interested.
   */
  namespace detail {
    
    namespace Codec {
      
      enum Field {
        Msg,
        Syst,
        Parity,
        State,
      };
      
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
        
      protected:
        DecoderAlgorithm decoderAlgorithm_; /**< Algorithm type used in decoder. */
        
      private:
        template <typename Archive>
        void serialize(Archive & ar, const unsigned int version);
      };
      
      template <class T>
      class Arguments {
        struct Value {
          T* arg;
          bool has = false;
        };
      public:
        Arguments() = default;
        
        Arguments& msg(T& arg) {insert(Msg, &arg); return *this;}
        Arguments& syst(T& arg) {insert(Syst, &arg); return *this;}
        Arguments& parity(T& arg) {insert(Parity, &arg); return *this;}
        Arguments& state(T& arg) {insert(State, &arg); return *this;}
        
        bool count(Field key) {return map_[key].has;}
        T& at(Field key) {return *map_[key].arg;}
        T& at(Field key) const {return *map_[key].arg;}
        void insert(Field key, T* arg) {map_[key].arg = arg; map_[key].has = true;}
        
      private:
        std::array<Value, 4> map_;
      };
      
      template <class T>
      using ConstArguments = Arguments<typename std::add_const<T>::type>;
      
      template <class it>
      using iterator = MultiIterator<it, Field, Msg, Syst, Parity, State>;
      
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