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
 
 Declaration of MapDecoder abstract class
 ******************************************************************************/

#include "ViterbiDecoder.h"
#include "ViterbiDecoderImpl.h"

using namespace fec;

/**
 *  MapDecoder creator function.
 *  Construct in a factory behavior a MapCode object corresponding to the algorithm
 *  version in use.
 *  \param  codeStructure Convolutional code structure describing the code
 *  \return MacDecoder specialization suitable for the algorithm in use
 */
std::unique_ptr<ViterbiDecoder> ViterbiDecoder::create(const Convolutional::Structure& structure)
{
  return std::unique_ptr<ViterbiDecoder>(new ViterbiDecoderImpl<FloatLlrMetrics>(structure));
}

/**
 *  Implementation of Code#decodeNBloc
 */
void ViterbiDecoder::decodeBlocks(std::vector<LlrType>::const_iterator parity, std::vector<BitField<bool>>::iterator msg, size_t n)
{
  for (size_t i = 0; i < n; i++) {
    decodeBlock(parity, msg);
    parity += structure().trellis().outputSize() * (structure().length() + structure().tailSize());
    msg += structure().trellis().inputSize() * structure().length();
  }
}

/**
 *  Constructor.
 *  Allocates metric buffers based on the given code structure.
 *  \param  codeStructure Convolutional code structure describing the code
 */
ViterbiDecoder::ViterbiDecoder(const Convolutional::Structure& structure) :
structure_(structure)
{
}
