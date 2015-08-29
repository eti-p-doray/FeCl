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
 
 Definition of Code class
 ******************************************************************************/

#include "Code.h"

#include "ConvolutionalCode/ConvolutionalCode.h"
#include "TurboCode/TurboCode.h"
#include "LdpcCode/LdpcCode.h"

using namespace fec;

BOOST_CLASS_EXPORT_IMPLEMENT(Code);
BOOST_CLASS_EXPORT_IMPLEMENT(Code::Structure);

/**
 *  Code creator function.
 *  Construct in a factory behavior a code object corresponding to the structure.
 *  \param codeStructure Code object structure
 *  \workGroupSize Number of thread used in encoding and decoding.
 */
std::unique_ptr<Code> Code::create(const Code::Structure& codeStructure, int workGroupdSize)
{
  switch (codeStructure.type()) {
    case Code::Structure::Convolutional:
      return std::unique_ptr<Code>(new ConvolutionalCode(dynamic_cast<const ConvolutionalCode::Structure&>(codeStructure), workGroupdSize));
      break;
      
    case Code::Structure::Turbo:
      return std::unique_ptr<Code>(new TurboCode(dynamic_cast<const TurboCode::Structure&>(codeStructure), workGroupdSize));
      break;
      
    case Code::Structure::Ldpc:
      return std::unique_ptr<Code>(new LdpcCode(dynamic_cast<const LdpcCode::Structure&>(codeStructure), workGroupdSize));
      break;
      
    default:
      break;
  }
  return std::unique_ptr<Code>();
}

Code::Code(Code::Structure* structure, int workGroupSize) : structureRef_(structure)
{
  workGroupSize_ = workGroupSize;
}

void Code::encodeNBloc(std::vector<uint8_t>::const_iterator messageIt, std::vector<uint8_t>::iterator parityIt, size_t n) const
{
  for (size_t i = 0; i < n; ++i) {
    encodeBloc(messageIt, parityIt);
    messageIt += msgSize();
    parityIt += paritySize();
  }
}

/**
 *  Code stucture constructor.
 *  \param  messageSize Size of the msg in each code bloc
 *  \param  paritySize Size of the parity code in each code bloc
 */
Code::Structure::Structure(size_t messageSize, size_t paritySize, size_t extrinsicSize)
{
  messageSize_ = messageSize;
  paritySize_ = paritySize;
  extrinsicSize_ = extrinsicSize;
}
