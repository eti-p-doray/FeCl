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

#include "PuncturedLdpc.h"
#include "BpDecoder/BpDecoder.h"

using namespace fec;

BOOST_CLASS_EXPORT_IMPLEMENT(PuncturedLdpc);
BOOST_CLASS_EXPORT_IMPLEMENT(PuncturedLdpc::Structure);

const char * PuncturedLdpc::get_key() const {
  return boost::serialization::type_info_implementation<PuncturedLdpc>::type::get_const_instance().get_key();
}

const char * PuncturedLdpc::Structure::get_key() const {
  return boost::serialization::type_info_implementation<PuncturedLdpc::Structure>::type::get_const_instance().get_key();
}

PuncturedLdpc::PuncturedLdpc(const Options& options,  int workGroupSize) :
Ldpc(std::unique_ptr<Structure>(new Structure(options)), workGroupSize)
{
}
/**
 *  Ldpc constructor
 *  \param  codeStructure Codec structure used for encoding and decoding
 *  \param  workGroupSize Number of thread used for decoding
 */
PuncturedLdpc::PuncturedLdpc(const Structure& structure,  int workGroupSize) :
Ldpc(std::unique_ptr<Structure>(new Structure(structure)), workGroupSize)
{
}
PuncturedLdpc::PuncturedLdpc(const EncoderOptions& encoder, const PunctureOptions& puncture, const DecoderOptions& decoder, int workGroupSize) :
Ldpc(std::unique_ptr<Structure>(new Structure(encoder, puncture, decoder)), workGroupSize)
{
}
PuncturedLdpc::PuncturedLdpc(const EncoderOptions& encoder, const PunctureOptions& puncture, int workGroupSize) :
Ldpc(std::unique_ptr<Structure>(new Structure(encoder, puncture)), workGroupSize)
{
}

void PuncturedLdpc::decodeBlocks(std::vector<LlrType>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg, size_t n) const
{
  std::vector<LlrType> parityTmp(structure().innerParitySize()*n, 0.0);
  structure().permutation().dePermuteBlocks<LlrType>(parity, parityTmp.begin(), n);
  auto worker = BpDecoder::create(structure());
  worker->decodeBlocks(parityTmp.begin(), msg, n);
}

void PuncturedLdpc::soDecodeBlocks(InputIterator input, OutputIterator output, size_t n) const
{
  std::vector<LlrType> parityTmp(structure().innerParitySize()*n, 0.0);
  structure().permutation().dePermuteBlocks<LlrType>(input.parity(), parityTmp.begin(), n);
  input.parity(parityTmp.begin());
  auto outputTmp = output;
  if (outputTmp.hasParity()) {
    outputTmp.parity(parityTmp.begin());
  }
  auto worker = BpDecoder::create(structure());
  Ldpc::Structure struc = structure();
  input.setStructureRef(&struc);
  outputTmp.setStructureRef(&struc);
  worker->soDecodeBlocks(input, outputTmp, n);
  if (output.hasParity()) {
    structure().permutation().permuteBlocks<LlrType>(outputTmp.parity(), output.parity(), n);
  }
}

PuncturedLdpc::Structure::Structure(const Options& options)
{
  setEncoderOptions(options);
  setPunctureOptions(options);
  setDecoderOptions(options);
}

PuncturedLdpc::Structure::Structure(const EncoderOptions& encoder, const PunctureOptions& puncture, const DecoderOptions& decoder)
{
  setEncoderOptions(encoder);
  setPunctureOptions(puncture);
  setDecoderOptions(decoder);
}
PuncturedLdpc::Structure::Structure(const EncoderOptions& encoder, const PunctureOptions& puncture)
{
  setEncoderOptions(encoder);
  setPunctureOptions(puncture);
  setDecoderOptions(DecoderOptions());
}

void PuncturedLdpc::Structure::setPunctureOptions(const fec::Ldpc::PunctureOptions& puncture)
{
  permutation_ = puncturing(puncture);
}

void PuncturedLdpc::Structure::encode(std::vector<BitField<size_t>>::const_iterator msg, std::vector<BitField<size_t>>::iterator parity) const
{
  std::vector<BitField<size_t>> parityTmp(innerParitySize(), 0);
  Ldpc::Structure::encode(msg, parityTmp.begin());
  permutation().permuteBlock<BitField<size_t>>(parityTmp.begin(), parity);
}

bool PuncturedLdpc::Structure::check(std::vector<BitField<size_t>>::const_iterator parity) const
{
  std::vector<BitField<size_t>> parityTmp(innerParitySize(), 0);
  permutation().dePermuteBlock<BitField<size_t>>(parity, parityTmp.begin());
  return Ldpc::Structure::check(parityTmp.begin());
}
