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

#ifndef FEC_DETAIL_MODULATION_FUNCTION_H
#define FEC_DETAIL_MODULATION_FUNCTION_H

#include <vector>

#include "Codec.h"
#include "Demodulator.h"

namespace fec {
  
  /**
   *  detail namespace. You shouln't be much interested.
   */
  namespace detail {
    
    namespace Modulation {

      template <class InputIterator, class OutputIterator>
      class ModulateFunction
      {
      public:
        static std::unique_ptr<ModulateFunction> create(const Modulation::Structure& structure) {return std::unique_ptr<ModulateFunction>(new ModulateFunction(structure));}
        virtual ~ModulateFunction() = default;
        
        void operator() (iterator<InputIterator> wordf, iterator<InputIterator> wordl, iterator<OutputIterator> symbol);
        void operator() (iterator<InputIterator> word, iterator<OutputIterator> symbol) {structure_.modulate(word.at(Modulation::Word), symbol.at(Modulation::Symbol));}
        
      private:
        ModulateFunction(const Structure& structure) : structure_(structure) {}
        Structure structure_;
      };
      
      template <class InputIterator, class OutputIterator>
      void ModulateFunction<InputIterator, OutputIterator>::operator() (iterator<InputIterator> wordf, iterator<InputIterator> wordl, iterator<OutputIterator> symbol)
      {
        while (wordf != wordl) {
          (*this)(wordf++, symbol++);
        }
      }
      
      template <class InputIterator, class OutputIterator>
      class DemodulateFunction
      {
      public:
        static std::unique_ptr<DemodulateFunction> create(const Modulation::Structure& structure) {return std::unique_ptr<DemodulateFunction>(new DemodulateFunction(structure));}
        virtual ~DemodulateFunction() = default;
        
        void operator() (iterator<InputIterator> symbolf, iterator<InputIterator> symboll, iterator<OutputIterator> word);
        void operator() (iterator<InputIterator> symbol, iterator<OutputIterator> word) {structure_.demodulate(symbol, word.at(Modulation::Word));}
        
      private:
        DemodulateFunction(const Structure& structure) : structure_(structure) {}
        Structure structure_;
      };
      
      template <class InputIterator, class OutputIterator>
      void DemodulateFunction<InputIterator, OutputIterator>::operator() (iterator<InputIterator> symbolf, iterator<InputIterator> symboll, iterator<OutputIterator> word)
      {
        while (symbolf != symboll) {
          (*this)(symbolf++, word++);
        }
      }
      
      template <class InputIterator, class OutputIterator>
      class SoDemodulateFunction
      {
      public:
        static std::unique_ptr<SoDemodulateFunction> create(const Modulation::Structure&);
        virtual ~SoDemodulateFunction() = default;
        
        void operator() (iterator<InputIterator> inf, iterator<InputIterator> inl, typename InputIterator::value_type k, iterator<OutputIterator> out);
        virtual void operator() (iterator<InputIterator> in, typename InputIterator::value_type k, iterator<OutputIterator> out) = 0;
      };
      
      template <class InputIterator, class OutputIterator, DecoderAlgorithm algorithm>
      class SoDemodulateFunctionImpl : public SoDemodulateFunction<InputIterator, OutputIterator>
      {
      public:
        static std::unique_ptr<SoDemodulateFunctionImpl> create(const Modulation::Structure& structure) {return std::unique_ptr<SoDemodulateFunctionImpl>(new SoDemodulateFunctionImpl(structure));}
        virtual ~SoDemodulateFunctionImpl() = default;
        
        void operator() (iterator<InputIterator> in, typename InputIterator::value_type k, iterator<OutputIterator> out) override {demodulator_.soDemodulate(in, k, out.at(Modulation::Word));}
        
      private:
        SoDemodulateFunctionImpl(const Structure& structure) : demodulator_(structure) {}
        Demodulator<algorithm, typename InputIterator::value_type> demodulator_;
      };
      
      template <class InputIterator, class OutputIterator>
      std::unique_ptr<SoDemodulateFunction<InputIterator,OutputIterator>> SoDemodulateFunction<InputIterator,OutputIterator>::create(const Modulation::Structure& structure)
      {
        switch (structure.decoderAlgorithm()) {
          default:
          case Exact:
            return SoDemodulateFunctionImpl<InputIterator,OutputIterator,Exact>::create(structure);
            
          case Linear:
            return SoDemodulateFunctionImpl<InputIterator,OutputIterator,Linear>::create(structure);
            
          case Approximate:
            return SoDemodulateFunctionImpl<InputIterator,OutputIterator,Approximate>::create(structure);
        }
      }
      
      template <class InputIterator, class OutputIterator>
      void SoDemodulateFunction<InputIterator, OutputIterator>::operator() (iterator<InputIterator> inf, iterator<InputIterator> inl, typename InputIterator::value_type k, iterator<OutputIterator> out)
      {
        while (inf != inl) {
          (*this)(inf++, k, out++);
        }
      }
      
    }
    
  }
  
}

#endif