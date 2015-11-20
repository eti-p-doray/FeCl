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
  
  class Demodulator {
  public:
    static std::unique_ptr<Demodulator> create(const Turbo::detail::Structure&);
    virtual ~Demodulator() = default;
    
    void demodulateBlocks(Modulation::detail::InputIterator input, Modulation::detail::OutputIterator output, size_t n);
    
  protected:
    Demodulator(const Modulation::detail::Structure& codeStructure);
    Demodulator() = default;
    
    inline const Modulation::detail::Structure& structure() const {return structure_;}
    
    virtual void demodulateBlock(Modulation::detail::InputIterator input, Modulation::detail::OutputIterator output) = 0;

  private:
    Modulation::detail::Structure structure_;
  };
}

/*typename LlrMetrics::Type metric[1<<n] = {logSum_.prior(-llrMetrics_.max())};
for (BitField<size_t> input = 0; input < m; ++input) {
  metric[input.test(j,n)] = logSum_.sum(metric[input.test(j,m)], bank[input]);
}
return logSum_.post(metric) - logSum_.post(metric[0]);*/

#endif