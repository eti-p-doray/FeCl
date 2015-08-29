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
 
 Declaration of TurboCodeImpl class
 ******************************************************************************/

#ifndef TURBO_CODE_IMPL_H
#define TURBO_CODE_IMPL_H

#include "../TurboCode.h"

namespace fec {

  /**
   *  This class contains the implementation of iterative decoder.
   *  This algorithm is used for decoding in a TurboCode.
   */
class TurboDecoder
{
public:
  class Structure {
  public:
    
  private:
    
  };
  
  TurboDecoder(const TurboCode::Structure& codeStructure) : codeStructure_(codeStructure) {
    for (size_t i = 0; i < codeStructure_.constituentCount(); ++i) {
      code_.push_back(MapDecoder::create(codeStructure_.constituent(i)));
    }
  }
  virtual ~TurboCodeImpl() = default;
  
  
  inline size_t extrinsicSize() const {return codeStructure_.extrinsicSize();}
  inline const TurboCode::Structure& structure() const {return codeStructure_;}
  
  void appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const;
  
protected:
  TurboDecoder() = default;

private:
  virtual void appDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const;
  
  //void serialDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut) const;
  //void parallelDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut) const;
  
  std::vector<std::unique_ptr<MapDecoder>> code_;
  TurboCode::Structure codeStructure_;
};
  
}

#endif
