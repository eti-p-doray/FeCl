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

#include "PuncturedTurbo.h"
#include "TurboDecoder/TurboDecoder.h"

using namespace fec;

BOOST_CLASS_EXPORT_IMPLEMENT(PuncturedTurbo);
BOOST_CLASS_EXPORT_IMPLEMENT(PuncturedTurbo::Structure);

const char * PuncturedTurbo::get_key() const {
  return boost::serialization::type_info_implementation<PuncturedTurbo>::type::get_const_instance().get_key();
}

const char * PuncturedTurbo::Structure::get_key() const {
  return boost::serialization::type_info_implementation<PuncturedTurbo::Structure>::type::get_const_instance().get_key();
}

/**
 *  Turbo constructor
 *  \param  codeStructure Codec structure used for encoding and decoding
 *  \param  workGroupSize Number of thread used for decoding
 */
PuncturedTurbo::PuncturedTurbo(const Structure& structure,  int workGroupSize) :
Turbo(std::unique_ptr<Structure>(new Structure(structure)), workGroupSize)
{
}
PuncturedTurbo::PuncturedTurbo(const EncoderOptions& encoder, const PunctureOptions& puncture, const DecoderOptions& decoder, int workGroupSize) :
Turbo(std::unique_ptr<Structure>(new Structure(encoder, puncture, decoder)), workGroupSize)
{
}
PuncturedTurbo::PuncturedTurbo(const EncoderOptions& encoder, const PunctureOptions& puncture, int workGroupSize) :
Turbo(std::unique_ptr<Structure>(new Structure(encoder, puncture)), workGroupSize)
{
}

void PuncturedTurbo::decodeBlocks(std::vector<LlrType>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg, size_t n) const
{
  std::vector<LlrType> parityTmp(structure().innerParitySize(), 0.0);
  structure().permutation().dePermuteBlocks<LlrType>(parity, parityTmp.begin(), n);
  auto worker = TurboDecoder::create(structure());
  worker->decodeBlocks(parityTmp.begin(), msg, n);
}

void PuncturedTurbo::soDecodeBlocks(InputIterator input, OutputIterator output, size_t n) const
{
  std::vector<LlrType> parityTmp(structure().innerParitySize(), 0.0);
  structure().permutation().dePermuteBlocks<LlrType>(input.parity(), parityTmp.begin(), n);
  input.parity(parityTmp.begin());
  auto outputTmp = output;
  if (outputTmp.hasParity()) {
    outputTmp.parity(parityTmp.begin());
  }
  auto worker = TurboDecoder::create(structure());
  worker->soDecodeBlocks(input, outputTmp, n);
  if (output.hasParity()) {
    structure().permutation().permuteBlocks<LlrType>(outputTmp.parity(), output.parity(), n);
  }
}


PuncturedTurbo::Structure::Structure(const EncoderOptions& encoder, const PunctureOptions& puncture, const DecoderOptions& decoder)
{
  Turbo::Structure::setEncoderOptions(encoder);
  setPunctureOptions(puncture);
  setDecoderOptions(decoder);
}
PuncturedTurbo::Structure::Structure(const EncoderOptions& encoder, const PunctureOptions& puncture)
{
  setEncoderOptions(encoder);
  setPunctureOptions(puncture);
  setDecoderOptions(DecoderOptions());
}

void PuncturedTurbo::Structure::setEncoderOptions(const fec::Turbo::EncoderOptions& encoder)
{
  Turbo::Structure::setEncoderOptions(encoder);
  permutation_ = puncturing({});
}

void PuncturedTurbo::Structure::setPunctureOptions(const fec::Turbo::PunctureOptions& puncture)
{
  permutation_ = puncturing(puncture);
}

void PuncturedTurbo::Structure::encode(std::vector<BitField<size_t>>::const_iterator msg, std::vector<BitField<size_t>>::iterator parity) const
{
  std::vector<BitField<size_t>> parityTmp(innerParitySize(), 0);
  Turbo::Structure::encode(msg, parityTmp.begin());
  permutation().permuteBlock<BitField<size_t>>(parityTmp.begin(), parity);
}

bool PuncturedTurbo::Structure::check(std::vector<BitField<size_t>>::const_iterator parity) const
{
  std::vector<BitField<size_t>> parityTmp(innerParitySize(), 0);
  permutation().dePermuteBlock<BitField<size_t>>(parity, parityTmp.begin());
  return Turbo::Structure::check(parityTmp.begin());
}
