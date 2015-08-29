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
 
 Declaration of MapDecoder class
 ******************************************************************************/

#ifndef MAP_DECODER_H
#define MAP_DECODER_H

#include <vector>
#include <memory>

#include "../ConvolutionalCode.h"

namespace fec {

  /**
   *  This class contains the abstract implementation of the map decoder.
   *  This algorithm is used for decoding with a-priori information
   *  in a ConvolutionalCode.
   *  The reason for this class is to offer an common interface of map decoders
   *  while allowing the compiler to inline implementation specific functions
   *  by using templates instead of polymorphism.
   */
class MapDecoder
{
public:
  class Structure {
  public:
    
  private:
    
  };
  
  static std::unique_ptr<MapDecoder> create(const ConvolutionalCode::Structure&);
  virtual ~MapDecoder() = default; /**< Default destructor */
  
  void softOutDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut, size_t n);
  void appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n);

  void softOutDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut);
  void appDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut);
  
protected:
  MapDecoder(const ConvolutionalCode::Structure& codeStructure);

  virtual void appBranchMetrics(std::vector<LlrType>::const_iterator parity, std::vector<LlrType>::const_iterator extrinsic) = 0;/**< Branch metric calculation with app (msg) L-values. */
  virtual void parityAppBranchMetrics(std::vector<LlrType>::const_iterator parity, std::vector<LlrType>::const_iterator extrinsic) = 0;/**< Branch metric calculation with app (parity) L-values. */
  virtual void branchMetrics(std::vector<LlrType>::const_iterator parity) = 0;/**< Branch metric calculation. */
  virtual void forwardMetrics() = 0;/**< Forward metric calculation. */
  virtual void backwardMetrics() = 0;/**< Backard metric calculation. */
  
  virtual void parityAPosteriori(std::vector<LlrType>::iterator parityOut) = 0;/**< Final (parity) L-values calculation. */
  virtual void messageAPosteriori(std::vector<LlrType>::iterator parityOut) = 0;
  virtual void messageExtrinsic(std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinscOut) = 0;/**< Final (msg) L-values calculation. */
  
  inline const ConvolutionalCode::Structure& codeStructure() const {return codeStructure_;} /**< Access the code structure */
  
  std::vector<LlrType> branchOutputMetrics_;/**< Output branch metric buffer */
  std::vector<LlrType> branchInputMetrics_;/**< Input branch metric buffer */
  
  std::vector<LlrType> branchMetrics_;/**< Branch metric buffer (gamma) */
  std::vector<LlrType> forwardMetrics_;/**< Forward metric buffer (alpha) */
  std::vector<LlrType> backwardMetrics_;/**< Backard metric buffer (beta) */

private:
  
  const ConvolutionalCode::Structure codeStructure_;
};
  
}

#endif
