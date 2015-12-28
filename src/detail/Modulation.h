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

#ifndef FEC_DETAIL_MODULATION_H
#define FEC_DETAIL_MODULATION_H

#include <vector>
#include <cmath>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/serialization/extended_type_info_no_rtti.hpp>

#include "MultiIterator.h"

namespace fec {
  
  namespace detail {
    
    namespace Modulation {
      
      enum Field {
        Word,
        Symbol,
      };
      
      struct ModOptions {
        friend class Structure;
      public:
        ModOptions(const std::vector<std::vector<double>>& constellation) {constellation_ = constellation;}
        ModOptions& length(size_t length) {length_ = length; return *this;}
        
        inline const std::vector<std::vector<double>>& constellation() const {return constellation_;}
        inline const size_t length() const {return length_;}
        
      private:
        std::vector<std::vector<double>> constellation_;
        size_t length_ = 1;
      };
      
      struct DemodOptions {
        friend class Structure;
      public:
        DemodOptions() = default;
        DemodOptions& algorithm(DecoderAlgorithm algorithm) {algorithm_ = algorithm; return *this;}
        DemodOptions& scalingFactor(double factor) {scalingFactor_ = factor; return *this;}
        
        DecoderAlgorithm algorithm() const {return algorithm_;}
        double scalingFactor() const {return scalingFactor_;}
        
      private:
        DecoderAlgorithm algorithm_ = Linear;
        double scalingFactor_ = 1.0;
      };
      
      class Structure {
        friend class ::boost::serialization::access;
      public:
        Structure() = default;
        Structure(const ModOptions&);
        Structure(const ModOptions&, const DemodOptions&);
        ~Structure() = default;
        
        const char * get_key() const;
        
        size_t wordWidth() const {return 1;} /**< Access the width of msg in each code bloc. */
        size_t symbolWidth() const {return dimension();} /**< Access the width of systematics in each code bloc. */
        
        size_t wordSize() const {return length_*size_;} /**< Access the width of msg in each code bloc. */
        size_t symbolSize() const {return length_;} /**< Access the width of systematics in each code bloc. */
        
        void setDemodOptions(const DemodOptions& demod);
        DemodOptions getDemodOptions() const;
        
        const std::vector<double>& constellation() const {return constellation_;}
        size_t dimension() const {return dimension_;}
        size_t size() const {return size_;}
        size_t length() const {return length_;}
        double avgPower() const {return avgPower_;}
        double peakPower() const {return peakPower_;}
        double minDistance() const {return minDistance_;}
        DecoderAlgorithm decoderAlgorithm() const {return algorithm_;} /**< Access the algorithm used in decoder. */
        double scalingFactor() const {return scalingFactor_;}
        
        template <class InputIterator, class OutputIterator>
        void modulate(InputIterator word, OutputIterator symbol) const;
        template <class InputIterator, class OutputIterator>
        void demodulate(InputIterator symbol, OutputIterator word) const;

      private:
        template <typename Archive>
        void serialize(Archive & ar, const unsigned int version);
        
        void setModOptions(const ModOptions& encoder);
        
        std::vector<double> constellation_;
        size_t dimension_;
        size_t length_;
        size_t size_;
        double avgPower_;
        double peakPower_;
        double minDistance_;
        double scalingFactor_;
        DecoderAlgorithm algorithm_;
      };
      
      template <class T>
      class Arguments {
        struct Value {
          T* arg;
          bool has = false;
        };
      public:
        Arguments() = default;
        
        Arguments& word(T& arg) {insert(Word, &arg); return *this;}
        Arguments& symbol(T& arg) {insert(Symbol, &arg); return *this;}
        
        bool count(Field key) {return map_[key].has;}
        T& at(Field key) {return *map_[key].arg;}
        T& at(Field key) const {return *map_[key].arg;}
        void insert(Field key, T* arg) {map_[key].arg = arg; map_[key].has = true;}
        
      private:
        std::array<Value, 2> map_;
      };
      
      template <class T>
      using ConstArguments = Arguments<typename std::add_const<T>::type>;
      
      template <class It>
      using iterator = MultiIterator<It, Field, Word, Symbol>;
      
    }
    
  }
  
}

BOOST_CLASS_TYPE_INFO(fec::detail::Modulation::Structure,extended_type_info_no_rtti<fec::detail::Modulation::Structure>);
BOOST_CLASS_EXPORT_KEY(fec::detail::Modulation::Structure);

template <class InputIterator, class OutputIterator>
void fec::detail::Modulation::Structure::modulate(InputIterator word, OutputIterator symbol) const
{
  for (size_t i = 0; i < length(); ++i) {
    BitField<size_t> input = 0;
    for (int j = 0; j < size(); j++) {
      input.set(j, word[j]);
    }
    input *= dimension();
    for (int j = 0; j < dimension(); ++j) {
      symbol[j] = constellation_[input+j];
    }
    word += size();
    symbol += symbolWidth();
  }
}

template <class InputIterator, class OutputIterator>
void fec::detail::Modulation::Structure::demodulate(InputIterator symbol, OutputIterator word) const
{
  for (size_t i = 0; i < length(); ++i) {
    typename InputIterator::value_type max = 0;
    BitField<size_t> maxInput = 0;
    for (size_t j = 0; j < 1<<size(); ++j) {
      auto tmp = -sqDistance(symbol, constellation().begin() + j*dimension(), dimension());
      if (tmp > max) {
        maxInput = j;
      }
    }
    for (int j = 0; j < size(); j++) {
      word[j] = maxInput.test(j);
    }
    word += size();
    symbol += symbolWidth();
  }
}

template <typename Archive>
void fec::detail::Modulation::Structure::serialize(Archive & ar, const unsigned int version) {
  using namespace boost::serialization;
  ar & BOOST_SERIALIZATION_NVP(constellation_);
  ar & BOOST_SERIALIZATION_NVP(dimension_);
  ar & BOOST_SERIALIZATION_NVP(avgPower_);
  ar & BOOST_SERIALIZATION_NVP(peakPower_);
  ar & BOOST_SERIALIZATION_NVP(minDistance_);
  ar & BOOST_SERIALIZATION_NVP(scalingFactor_);
  ar & BOOST_SERIALIZATION_NVP(algorithm_);
}

#endif