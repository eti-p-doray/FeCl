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
 
 Definition of Codec class
 ******************************************************************************/

#include "Codec.h"

#include "Convolutional/Convolutional.h"
#include "Turbo/Turbo.h"
#include "Ldpc/Ldpc.h"

using namespace fec;

BOOST_CLASS_EXPORT_IMPLEMENT(Codec);
BOOST_CLASS_EXPORT_IMPLEMENT(Codec::Structure);

/**
 *  Codec creator function.
 *  Construct in a factory behavior a code object corresponding to the structure.
 *  \param codeStructure Codec object structure
 *  \param codeStructure Decoder parameters
 *  \param workGroupSize Number of thread used in encoding and decoding.
 */
std::unique_ptr<Codec> Codec::create(const Codec::Structure& structure, int workGroupSize)
{
  switch (structure.type()) {
    case Codec::Structure::Convolutional:
      return std::unique_ptr<Codec>(new Convolutional(dynamic_cast<const Convolutional::Structure&>(structure),
                                                     workGroupSize));
      break;
      
    case Codec::Structure::Turbo:
      return std::unique_ptr<Codec>(new Turbo(dynamic_cast<const Turbo::Structure&>(structure), workGroupSize));
      break;
      
    case Codec::Structure::Ldpc:
      return std::unique_ptr<Codec>(new Ldpc(dynamic_cast<const Ldpc::Structure&>(structure), workGroupSize));
      break;
      
    default:
      break;
  }
  return std::unique_ptr<Codec>();
}

Codec::Codec(Codec::Structure* structure, int workGroupSize) : structureRef_(structure)
{
  workGroupSize_ = workGroupSize;
}

bool Codec::checkBlocks(std::vector<BitField<uint8_t>>::const_iterator parity, size_t n) const
{
  for (size_t i = 0; i < n; ++i) {
    bool check = structure().check(parity);
    if (!check) {
      return false;
    }
    parity += paritySize();
  }
  return true;
}

void Codec::encodeBlocks(std::vector<BitField<bool>>::const_iterator msg, std::vector<BitField<uint8_t>>::iterator parity, size_t n) const
{
  for (size_t i = 0; i < n; ++i) {
    structure().encode(msg, parity);
    msg += msgSize();
    parity += paritySize();
  }
}
