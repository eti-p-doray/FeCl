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

#ifndef FEC_MAP_DECODER_IMPL_H
#define FEC_MAP_DECODER_IMPL_H

#include <vector>
#include <memory>

#include "MapDecoder.h"

namespace fec {
  
  /**
   *  This class contains the abstract implementation of the map decoder.
   *  This algorithm is used for decoding with a-priori information
   *  in a ConvolutionalCodec.
   *  The reason for this class is to offer an common interface of map decoders
   *  while allowing the compiler to inline implementation specific functions
   *  by using templates instead of polymorphism.
   */
  template <class LlrMetrics, template <class> class LogSumAlg>
  class MapDecoderImpl : public MapDecoder
  {
  public:
    MapDecoderImpl(const Convolutional::detail::Structure&); /**< Constructor */
    virtual ~MapDecoderImpl() = default; /**< Default destructor */
    
    virtual void soDecodeBlock(Codec::detail::InputIterator input, Codec::detail::OutputIterator output);
    template <class T> void soDecodeBlockImpl(Codec::detail::InfoIterator<typename std::vector<T>::const_iterator> input, Codec::detail::InfoIterator<typename std::vector<T>::iterator> output);
    
  protected:
    template <class T> void branchUpdate(Codec::detail::InfoIterator<typename std::vector<T>::const_iterator> input);/**< Branch metric calculation. */
    void forwardUpdate();/**< Forward metric calculation. */
    void backwardUpdate();/**< Backard metric calculation. */
    template <class T> void aPosterioriUpdate(Codec::detail::InfoIterator<typename std::vector<T>::const_iterator> input, Codec::detail::InfoIterator<typename std::vector<T>::iterator> output);/**< Final (msg) L-values calculation. */
    
  private:
    template <class U = typename LogSumAlg<LlrMetrics>::isRecursive, typename std::enable_if<U::value>::type* = nullptr>
    void forwardUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator forwardMetric, typename std::vector<typename LlrMetrics::Type>::const_iterator branchMetric);/**< Forward metric calculation. */
    template <class U = typename LogSumAlg<LlrMetrics>::isRecursive, typename std::enable_if<!U::value>::type* = nullptr>
    void forwardUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator forwardMetric, typename std::vector<typename LlrMetrics::Type>::const_iterator branchMetric);/**< Forward metric calculation. */
    
    template <class U = typename LogSumAlg<LlrMetrics>::isRecursive, typename std::enable_if<U::value>::type* = nullptr>
    void backwardUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator backwardMetric, typename std::vector<typename LlrMetrics::Type>::const_iterator branchMetric);/**< Forward metric calculation. */
    template <class U = typename LogSumAlg<LlrMetrics>::isRecursive, typename std::enable_if<!U::value>::type* = nullptr>
    void backwardUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator backwardMetric, typename std::vector<typename LlrMetrics::Type>::const_iterator branchMetric);/**< Forward metric calculation. */
    
    template <class U = typename LogSumAlg<LlrMetrics>::isRecursive, typename std::enable_if<U::value>::type* = nullptr>
    typename LlrMetrics::Type msgUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator branchMetric, size_t j);/**< Forward metric calculation. */
    template <class U = typename LogSumAlg<LlrMetrics>::isRecursive, typename std::enable_if<!U::value>::type* = nullptr>
    typename LlrMetrics::Type msgUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator branchMetric, size_t j);/**< Forward metric calculation. */
    
    template <class U = typename LogSumAlg<LlrMetrics>::isRecursive, typename std::enable_if<U::value>::type* = nullptr>
    typename LlrMetrics::Type parityUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator branchMetric, size_t j);/**< Forward metric calculation. */
    template <class U = typename LogSumAlg<LlrMetrics>::isRecursive, typename std::enable_if<!U::value>::type* = nullptr>
    typename LlrMetrics::Type parityUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator branchMetric, size_t j);/**< Forward metric calculation. */
    
    std::vector<typename LlrMetrics::Type> bufferMetrics_;
    
    std::vector<typename LlrMetrics::Type> branchMetrics_;/**< Branch metric buffer (gamma) */
    std::vector<typename LlrMetrics::Type> forwardMetrics_;/**< Forward metric buffer (alpha) */
    std::vector<typename LlrMetrics::Type> backwardMetrics_;/**< Backard metric buffer (beta) */
    
    LlrMetrics llrMetrics_;
    LogSumAlg<LlrMetrics> logSum_;
  };
  
}

#endif
