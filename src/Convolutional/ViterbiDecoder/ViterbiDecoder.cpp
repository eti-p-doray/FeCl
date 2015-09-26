/*******************************************************************************
 This file is part of C3sar.
 
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 Copyright (c) 2015, Leszek Szczecinski, INRS
 All rights reserved.
 
 C3sar is free software: you can redistribute it and/or modify
 it under the terms of the Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 C3sar is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the Lesser General Public License
 along with C3rel.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include "ViterbiDecoder.h"
#include "ViterbiDecoderImpl.h"

using namespace fec;

/**
 *  MapDecoder creator function.
 *  Construct in a factory behavior a MapCodec object corresponding to the algorithm
 *  version in use.
 *  \param  codeStructure Convolutional code structure describing the code
 *  \return MacDecoder specialization suitable for the algorithm in use
 */
std::unique_ptr<ViterbiDecoder> ViterbiDecoder::create(const Convolutional::Structure& structure)
{
  return std::unique_ptr<ViterbiDecoder>(new ViterbiDecoderImpl<FloatLlrMetrics>(structure));
}

/**
 *  Implementation of Codec#decodeNBloc
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
