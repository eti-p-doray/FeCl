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
 
 Declaration of BpDecoder class
 ******************************************************************************/

#ifndef BP_DECODER_H
#define BP_DECODER_H

#include <vector>
#include <memory>

#include "../LdpcCode.h"

namespace fec {

  /**
   *  This class contains the abstract implementation of the belief propagation decoder.
   *  This algorithm is used for decoding in an LdpcCode.
   *  The reason for this class is to offer an common interface of bp decoders
   *  while allowing the compiler to inline implementation specific functions
   *  by using templates instead of polymorphism.
   */
class BpDecoder
{
public:
  static std::unique_ptr<BpDecoder> create(const LdpcCode::Structure&);
  virtual ~BpDecoder() = default;
  
  void appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n);
  void softOutDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut, size_t n);
  
protected:
  BpDecoder(const LdpcCode::Structure& codeStructure);
  
  void appDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut);
  void softOutDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut);
  
  virtual void checkUpdate() = 0;
  virtual void bitUpdate(std::vector<LlrType>::const_iterator parity) = 0;
  
  inline const LdpcCode::Structure& codeStructure() const {return codeStructure_;}
 
  std::vector<uint8_t> hardParity_;
  
  std::vector<LlrType> bitMetrics_;
  std::vector<LlrType> checkMetrics_;
  std::vector<LlrType> checkMetricsBuffer_;

private:
  
  const LdpcCode::Structure codeStructure_;
};
  
}

#endif
