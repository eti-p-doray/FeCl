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

#include "Convolutional.h"
#include "detail/MapDecoder/MapDecoder.h"
#include "detail/ViterbiDecoder/ViterbiDecoder.h"

using namespace fec;

BOOST_CLASS_EXPORT_IMPLEMENT(Convolutional);

Convolutional::Convolutional(const detail::Convolutional::Structure& structure,  int workGroupSize) :
Codec(std::unique_ptr<detail::Convolutional::Structure>(new detail::Convolutional::Structure(structure)), workGroupSize)
{
}

Convolutional::Convolutional(const EncoderOptions& encoder, const DecoderOptions& decoder, int workGroupSize) :
Codec(std::unique_ptr<detail::Convolutional::Structure>(new detail::Convolutional::Structure(encoder, decoder)), workGroupSize)
{
}

Convolutional::Convolutional(const EncoderOptions& encoder, int workGroupSize) :
Codec(std::unique_ptr<detail::Convolutional::Structure>(new detail::Convolutional::Structure(encoder)), workGroupSize)
{
}

const char * Convolutional::get_key() const
{
  return boost::serialization::type_info_implementation<Convolutional>::type::get_const_instance().get_key();
}


void Convolutional::soDecodeBlocks(detail::Codec::const_iterator<double> first, detail::Codec::const_iterator<double> last, detail::Codec::iterator<double> output) const
{
  auto worker = detail::MapDecoder::create(structure());
  worker->soDecodeBlocks(first, last, output);
}

void Convolutional::decodeBlocks(detail::Codec::const_iterator<double> first, detail::Codec::const_iterator<double> last, detail::Codec::iterator<BitField<size_t>> output) const
{
  auto worker = detail::ViterbiDecoder::create(structure());
  worker->decodeBlocks(first.at(detail::Codec::Parity), last.at(detail::Codec::Parity), output.at(detail::Codec::Msg));
}
