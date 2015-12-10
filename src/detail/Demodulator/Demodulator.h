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

#ifndef FEC_DEMODULATOR_H
#define FEC_DEMODULATOR_H

#include <vector>
#include <memory>

#include "../Modulation.h"

namespace fec {
  
  namespace detail {
    
    /**
     *  This class contains the abstract implementation of the demodulator.
     *  The reason for this class is to offer an common interface of demodulators
     *  while allowing the compiler to inline implementation specific functions
     *  by using metaprogramming instead of polymorphism.
     */
    class Demodulator
    {
    public:
      static std::unique_ptr<Demodulator> create(const Modulation::Structure&); /**< Creating function */
      virtual ~Demodulator() = default; /**< Default destructor */
      
      virtual void soDemodulateBlocks(Modulation::const_iterator<double> inputf, Modulation::const_iterator<double> inputl, double k, std::vector<double>::iterator word) = 0;
      virtual void soDemodulateBlock(Modulation::const_iterator<double> input, double k, std::vector<double>::iterator word) = 0;
      
    protected:
      Demodulator(const Modulation::Structure&); /**< Constructor */
      
      inline const Modulation::Structure& structure() const {return structure_;} /**< Access the code structure */
      
    private:
      Modulation::Structure structure_;
    };
    
  }
  
}

#endif
