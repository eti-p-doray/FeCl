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

namespace fec {

  class Modulation {
  public:
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

#endif