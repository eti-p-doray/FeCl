/*******************************************************************************
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 Declaration of MapDecoderImpl class
 ******************************************************************************/

#ifndef MAP_DECODER_IMPL_H
#define MAP_DECODER_IMPL_H

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
    MapDecoderImpl(const Convolutional::Structure&); /**< Constructor */
    virtual ~MapDecoderImpl() = default; /**< Default destructor */
    
    virtual void soDecodeBlock(Codec::InputIterator input, Codec::OutputIterator output);
    template <class T> void soDecodeBlockImpl(Codec::InfoIterator<typename std::vector<T>::const_iterator> input, Codec::InfoIterator<typename std::vector<T>::iterator> output);
    
  protected:
    template <class T> void branchUpdate(Codec::InfoIterator<typename std::vector<T>::const_iterator> input);/**< Branch metric calculation. */
    void forwardUpdate();/**< Forward metric calculation. */
    void backwardUpdate();/**< Backard metric calculation. */
    template <class T> void aPosterioriUpdate(Codec::InfoIterator<typename std::vector<T>::const_iterator> input, Codec::InfoIterator<typename std::vector<T>::iterator> output);/**< Final (msg) L-values calculation. */
    
  private:
    template <bool isRecursive>
    void forwardUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator forwardMetric, typename std::vector<typename LlrMetrics::Type>::const_iterator branchMetric, char(*)[isRecursive]=0);/**< Forward metric calculation. */
    template <bool isRecursive>
    void forwardUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator forwardMetric, typename std::vector<typename LlrMetrics::Type>::const_iterator branchMetric, char(*)[!isRecursive]=0);/**< Forward metric calculation. */
    
    template <bool isRecursive>
    void backwardUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator backwardMetric, typename std::vector<typename LlrMetrics::Type>::const_iterator branchMetric, char(*)[isRecursive]=0);/**< Forward metric calculation. */
    template <bool isRecursive>
    void backwardUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator backwardMetric, typename std::vector<typename LlrMetrics::Type>::const_iterator branchMetric, char(*)[!isRecursive]=0);/**< Forward metric calculation. */
    
    template <bool isRecursive>
    typename LlrMetrics::Type msgUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator branchMetric, size_t j, char(*)[isRecursive]=0);/**< Forward metric calculation. */
    template <bool isRecursive>
    typename LlrMetrics::Type msgUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator branchMetric, size_t j, char(*)[!isRecursive]=0);/**< Forward metric calculation. */
    
    template <bool isRecursive>
    typename LlrMetrics::Type parityUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator branchMetric, size_t j, char(*)[isRecursive]=0);/**< Forward metric calculation. */
    template <bool isRecursive>
    typename LlrMetrics::Type parityUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator branchMetric, size_t j, char(*)[!isRecursive]=0);/**< Forward metric calculation. */
    
    std::vector<typename LlrMetrics::Type> bufferMetrics_;
    
    std::vector<typename LlrMetrics::Type> branchMetrics_;/**< Branch metric buffer (gamma) */
    std::vector<typename LlrMetrics::Type> forwardMetrics_;/**< Forward metric buffer (alpha) */
    std::vector<typename LlrMetrics::Type> backwardMetrics_;/**< Backard metric buffer (beta) */
    
    LlrMetrics llrMetrics_;
    LogSumAlg<LlrMetrics> logSum_;
  };
  
}

#endif
