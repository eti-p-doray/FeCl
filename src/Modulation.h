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

#ifndef FEC_MODULATION_H
#define FEC_MODULATION_H

#include <memory>
#include <thread>
#include <vector>

#include "LlrMetrics.h"

namespace fec {

  class Modulation {
  public:
    struct ModOptions {
      ModOptions(const std::vector<double>& constellation, size_t dimension) {constellation_ = constellation; dimension_ = dimension;}
      
      std::vector<double> constellation_;
      size_t dimension_;
      
    };
    
    struct DemodOptions {
      DemodOptions& algorithm(DecoderAlgorithm algorithm) {algorithm_ = algorithm; return *this;}
      
      DecoderAlgorithm algorithm_ = Linear;
      
    };
    
    struct detail {
      class Structure {
      public:
        Structure(const ModOptions&);
        Structure(const ModOptions&, const DemodOptions);
        
        size_t dimension() const;
        double power() const;
        
      private:
        std::vector<double> constellation_;
        size_t dimension_;
        double power_;
      };
      
      template <class Iterator>
      class InfoIterator {
      public:
        InfoIterator(const Structure* structureRef) : structureRef_(structureRef) {}
        
        InfoIterator& symbol(Iterator symbol) {symbol_ = symbol; hasSymbol_ = true; return *this;}
        InfoIterator& msg(Iterator msg) {msg_ = msg; hasMsg_ = true; return *this;}
        
        inline void operator ++ ();
        inline void operator += (size_t x);
        inline bool operator != (const InfoIterator& b);
        
        Iterator symbol() const {return symbol_;}
        Iterator msg() const {return msg_;}
        
        bool hasSymbol() const {return hasSymbol_;}
        bool hasMsg() const {return hasMsg_;}
        
      private:
        Iterator symbol_;
        Iterator msg_;
        bool hasSymbol_ = false;
        bool hasMsg_ = false;
        const Structure* structureRef_;
      };
      using InputIterator = InfoIterator<std::vector<double>::const_iterator>;
      using OutputIterator = InfoIterator<std::vector<double>::iterator>;
      
      template <class Vector>
      class Info {
      public:
        using Iterator = InfoIterator<decltype(std::declval<Vector>().begin())>;
        
        Info() = default;
        
        Info& symbol(Vector& syst) {symbol_ = &symbol; return *this;}
        Info& msg(Vector& parity) {msg_ = &msg; return *this;}
        
        Vector& symbol() const {return *symbol_;}
        Vector& msg() const {return *msg_;}
        
        bool hasSymbol() const {return symbol_ != nullptr;}
        bool hasMsg() const {return msg_ != nullptr;}
        
        Iterator begin(const Structure& structure) const;
        Iterator end(const Structure& structure) const;
        
      private:
        Vector* symbol_ = nullptr;
        Vector* msg_ = nullptr;
      };
    };
    
    template <template <typename> class A = std::allocator>
    using Input = detail::Info<const std::vector<double,A<double>>>;
    template <template <typename> class A = std::allocator>
    using Output = detail::Info<std::vector<double,A<double>>>;
    
    Modulation(const ModOptions&);
    Modulation(const ModOptions&, const DemodOptions);
    Modulation(const detail::Structure&);
    
    template <template <typename> class A>
    void modulate(const std::vector<BitField<size_t>,A<BitField<size_t>>>& msg, std::vector<double,A<double>>& symbol) const;
    template <template <typename> class A>
    std::vector<double,A<double>> modulate(const std::vector<double,A<double>>& bit) const;
    
    template <template <typename> class A>
    void soDemodulate(std::vector<double>, Input<A> input, std::vector<double,A<double>>& output) const;
    template <template <typename> class A>
    std::vector<BitField<size_t>,A<BitField<size_t>>> demodulate(std::vector<double> noise, std::vector<double> symbol) const;
    
  };
  
}

/*typename LlrMetrics::Type metric[1<<n] = {logSum_.prior(-llrMetrics_.max())};
for (BitField<size_t> input = 0; input < m; ++input) {
  metric[input.test(j,n)] = logSum_.sum(metric[input.test(j,m)], bank[input]);
}
return logSum_.post(metric) - logSum_.post(metric[0]);*/

#endif