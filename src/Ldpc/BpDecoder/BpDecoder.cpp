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
 
 Declaration of BpDecoder abstract class
 ******************************************************************************/

#include "BpDecoder.h"
#include "BpDecoderImpl.h"

using namespace fec;

std::unique_ptr<BpDecoder> BpDecoder::create(const Ldpc::Structure& structure)
{
  switch (structure.decoderType()) {
    default:
    case Code::Exact:
      return std::unique_ptr<BpDecoder>(new BpDecoderImpl<FloatLlrMetrics,BoxSum>(structure));
      break;
      
    case Code::Approximate:
      return std::unique_ptr<BpDecoder>(new BpDecoderImpl<FloatLlrMetrics,MinBoxSum>(structure));
      break;
  }
}

void BpDecoder::decodeBlocks(std::vector<LlrType>::const_iterator parity, std::vector<BitField<bool>>::iterator msg, size_t n)
{
  for (size_t i = 0; i < n; ++i) {
    decodeBlock(parity, msg);
    parity += structure().paritySize();
    msg += structure().msgSize();
  }
}

void BpDecoder::soDecodeBlocks(Code::InputIterator input, Code::OutputIterator output, size_t n)
{
  for (size_t i = 0; i < n; ++i) {
    soDecodeBlock(input, output);
    ++input;
    ++output;
  }
}

BpDecoder::BpDecoder(const Ldpc::Structure& structure) : structure_(structure)
{
}
