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

#include "../PuncturedLdpc.h"
#include "BpDecoder/BpDecoder.h"

using namespace fec;

BOOST_CLASS_EXPORT_IMPLEMENT(PuncturedLdpc);
BOOST_CLASS_EXPORT_IMPLEMENT(PuncturedLdpc::detail::Structure);

const char * PuncturedLdpc::get_key() const {
  return boost::serialization::type_info_implementation<PuncturedLdpc>::type::get_const_instance().get_key();
}

const char * PuncturedLdpc::detail::Structure::get_key() const {
  return boost::serialization::type_info_implementation<PuncturedLdpc::detail::Structure>::type::get_const_instance().get_key();
}

PuncturedLdpc::PuncturedLdpc(const Options& options,  int workGroupSize) :
Ldpc(std::unique_ptr<detail::Structure>(new detail::Structure(options)), workGroupSize)
{
}
/**
 *  Ldpc constructor
 *  \param  codeStructure Codec structure used for encoding and decoding
 *  \param  workGroupSize Number of thread used for decoding
 */
PuncturedLdpc::PuncturedLdpc(const detail::Structure& structure,  int workGroupSize) :
Ldpc(std::unique_ptr<detail::Structure>(new detail::Structure(structure)), workGroupSize)
{
}
PuncturedLdpc::PuncturedLdpc(const EncoderOptions& encoder, const PunctureOptions& puncture, const DecoderOptions& decoder, int workGroupSize) :
Ldpc(std::unique_ptr<detail::Structure>(new detail::Structure(encoder, puncture, decoder)), workGroupSize)
{
}
PuncturedLdpc::PuncturedLdpc(const EncoderOptions& encoder, const PunctureOptions& puncture, int workGroupSize) :
Ldpc(std::unique_ptr<detail::Structure>(new detail::Structure(encoder, puncture)), workGroupSize)
{
}

void PuncturedLdpc::decodeBlocks(std::vector<LlrType>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg, size_t n) const
{
  std::vector<LlrType> parityTmp(structure().innerParitySize()*n, 0.0);
  structure().permutation().dePermuteBlocks<LlrType>(parity, parityTmp.begin(), n);
  auto worker = BpDecoder::create(structure());
  worker->decodeBlocks(parityTmp.begin(), msg, n);
}

void PuncturedLdpc::soDecodeBlocks(Codec::detail::InputIterator input, Codec::detail::OutputIterator output, size_t n) const
{
  std::vector<LlrType> parityTmp(structure().innerParitySize()*n, 0.0);
  structure().permutation().dePermuteBlocks<LlrType>(input.parity(), parityTmp.begin(), n);
  input.parity(parityTmp.begin());
  auto outputTmp = output;
  if (outputTmp.hasParity()) {
    outputTmp.parity(parityTmp.begin());
  }
  auto worker = BpDecoder::create(structure());
  Ldpc::detail::Structure struc = structure();
  input.setStructureRef(&struc);
  outputTmp.setStructureRef(&struc);
  worker->soDecodeBlocks(input, outputTmp, n);
  if (output.hasParity()) {
    structure().permutation().permuteBlocks<LlrType>(outputTmp.parity(), output.parity(), n);
  }
}

PuncturedLdpc::detail::Structure::Structure(const Options& options)
{
  setEncoderOptions(options);
  setPunctureOptions(options);
  setDecoderOptions(options);
}

PuncturedLdpc::detail::Structure::Structure(const EncoderOptions& encoder, const PunctureOptions& puncture, const DecoderOptions& decoder)
{
  setEncoderOptions(encoder);
  setPunctureOptions(puncture);
  setDecoderOptions(decoder);
}
PuncturedLdpc::detail::Structure::Structure(const EncoderOptions& encoder, const PunctureOptions& puncture)
{
  setEncoderOptions(encoder);
  setPunctureOptions(puncture);
  setDecoderOptions(DecoderOptions());
}

void PuncturedLdpc::detail::Structure::setPunctureOptions(const fec::Ldpc::PunctureOptions& puncture)
{
  permutation_ = puncturing(puncture);
}

void PuncturedLdpc::detail::Structure::encode(std::vector<BitField<size_t>>::const_iterator msg, std::vector<BitField<size_t>>::iterator parity) const
{
  std::vector<BitField<size_t>> parityTmp(innerParitySize(), 0);
  Ldpc::detail::Structure::encode(msg, parityTmp.begin());
  permutation().permuteBlock<BitField<size_t>>(parityTmp.begin(), parity);
}

bool PuncturedLdpc::detail::Structure::check(std::vector<BitField<size_t>>::const_iterator parity) const
{
  std::vector<BitField<size_t>> parityTmp(innerParitySize(), 0);
  permutation().dePermuteBlock<BitField<size_t>>(parity, parityTmp.begin());
  return Ldpc::detail::Structure::check(parityTmp.begin());
}
