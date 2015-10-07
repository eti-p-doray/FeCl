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

#include "PuncturedConvolutional.h"
#include "MapDecoder/MapDecoder.h"
#include "ViterbiDecoder/ViterbiDecoder.h"

using namespace fec;

BOOST_CLASS_EXPORT_IMPLEMENT(PuncturedConvolutional);
BOOST_CLASS_EXPORT_IMPLEMENT(PuncturedConvolutional::Structure);

const char * PuncturedConvolutional::get_key() const {
  return boost::serialization::type_info_implementation<PuncturedConvolutional>::type::get_const_instance().get_key();
}

const char * PuncturedConvolutional::Structure::get_key() const {
  return boost::serialization::type_info_implementation<PuncturedConvolutional::Structure>::type::get_const_instance().get_key();
}

/**
 *  Convolutional constructor
 *  \param  codeStructure Codec structure used for encoding and decoding
 *  \param  workGroupSize Number of thread used for decoding
 */
PuncturedConvolutional::PuncturedConvolutional(const Structure& structure,  int workGroupSize) :
Convolutional(std::unique_ptr<Structure>(new Structure(structure)), workGroupSize)
{
}
PuncturedConvolutional::PuncturedConvolutional(const EncoderOptions& encoder, const PunctureOptions& puncture, const DecoderOptions& decoder, int workGroupSize) :
Convolutional(std::unique_ptr<Structure>(new Structure(encoder, puncture, decoder)), workGroupSize)
{
}
PuncturedConvolutional::PuncturedConvolutional(const EncoderOptions& encoder, const PunctureOptions& puncture, int workGroupSize) :
Convolutional(std::unique_ptr<Structure>(new Structure(encoder, puncture)), workGroupSize)
{
}

void PuncturedConvolutional::decodeBlocks(std::vector<LlrType>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg, size_t n) const
{
  std::vector<LlrType> parityTmp(structure().paritySize(), 0.0);
  structure().permutation().dePermuteBlocks<LlrType>(parity, parityTmp.begin(), n);
  auto worker = ViterbiDecoder::create(structure());
  worker->decodeBlocks(parityTmp.begin(), msg, n);
}

void PuncturedConvolutional::soDecodeBlocks(InputIterator input, OutputIterator output, size_t n) const
{
  std::vector<LlrType> parityTmp(structure().paritySize(), 0.0);
  structure().permutation().dePermuteBlocks<LlrType>(input.parity(), parityTmp.begin(), n);
  input.parity(parityTmp.begin());
  auto outputTmp = output;
  if (outputTmp.hasParity()) {
    outputTmp.parity(parityTmp.begin());
  }
  auto worker = MapDecoder::create(structure());
  worker->soDecodeBlocks(input, outputTmp, n);
  if (output.hasParity()) {
    structure().permutation().permuteBlocks<LlrType>(outputTmp.parity(), output.parity(), n);
  }
}


PuncturedConvolutional::Structure::Structure(const EncoderOptions& encoder, const PunctureOptions& puncture, const DecoderOptions& decoder)
{
  setEncoderOptions(encoder);
  setPunctureOptions(puncture);
  setDecoderOptions(decoder);
}
PuncturedConvolutional::Structure::Structure(const EncoderOptions& encoder, const PunctureOptions& puncture)
{
  setEncoderOptions(encoder);
  setPunctureOptions(puncture);
  setDecoderOptions(DecoderOptions());
}

void PuncturedConvolutional::Structure::setEncoderOptions(const fec::Convolutional::EncoderOptions& encoder)
{
  Convolutional::Structure::setEncoderOptions(encoder);
  permutation_ = createPermutation({});
}

void PuncturedConvolutional::Structure::setPunctureOptions(const fec::Convolutional::PunctureOptions& puncture)
{
  permutation_ = createPermutation(puncture);
}

void PuncturedConvolutional::Structure::encode(std::vector<BitField<size_t>>::const_iterator msg, std::vector<BitField<size_t>>::iterator parity) const
{
  std::vector<BitField<size_t>> parityTmp(paritySize(), 0);
  Convolutional::Structure::encode(msg, parityTmp.begin());
  permutation().permuteBlock<BitField<size_t>>(parityTmp.begin(), parity);
}

bool PuncturedConvolutional::Structure::check(std::vector<BitField<size_t>>::const_iterator parity) const
{
  std::vector<BitField<size_t>> parityTmp(paritySize(), 0);
  permutation().dePermuteBlock<BitField<size_t>>(parity, parityTmp.begin());
  return Convolutional::Structure::check(parityTmp.begin());
}