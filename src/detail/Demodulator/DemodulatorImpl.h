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

#ifndef FEC_DEMODULATOR_IMPL_H
#define FEC_DEMODULATOR_IMPL_H

#include <vector>
#include <memory>

#include "Demodulator.h"
#include "../LlrMetrics.h"

namespace fec {
  
  namespace detail {
    
    /**
     *  This class contains the abstract implementation of the map decoder.
     *  This algorithm is used for decoding with a-priori information
     *  in a ConvolutionalCodec.
     *  The reason for this class is to offer an common interface of map decoders
     *  while allowing the compiler to inline implementation specific functions
     *  by using templates instead of polymorphism.
     */
    template <class LlrMetrics, template <class> class LogSumAlg>
    class DemodulatorImpl : public Demodulator
    {
    public:
      DemodulatorImpl(const Modulation::Structure&); /**< Constructor */
      virtual ~DemodulatorImpl() = default; /**< Default destructor */
      
      void soDemodulateBlocks(Modulation::const_iterator<double> inputf, Modulation::const_iterator<double> inputl, double k, std::vector<double>::iterator word) override;
      void soDemodulateBlock(Modulation::const_iterator<double> input, double k, std::vector<double>::iterator word) override;
      
    protected:
      void distanceUpdate(Modulation::const_iterator<double> input);/**< distance metric calculation. */
      void aPosterioriUpdate(Modulation::const_iterator<double> input, double k, std::vector<double>::iterator output);/**< Final (msg) L-values calculation. */
      
    private:
      template <class U = typename LogSumAlg<LlrMetrics>::isRecursive, typename std::enable_if<U::value>::type* = nullptr>
      typename LlrMetrics::Type wordUpdateImpl(size_t j);/**< Forward metric calculation. */
      template <class U = typename LogSumAlg<LlrMetrics>::isRecursive, typename std::enable_if<!U::value>::type* = nullptr>
      typename LlrMetrics::Type wordUpdateImpl(size_t j);/**< Forward metric calculation. */
      
      std::vector<typename LlrMetrics::Type> distance_;
      
      LlrMetrics llrMetrics_;
      LogSumAlg<LlrMetrics> logSum_;
    };
    
  }
  
}

#endif
