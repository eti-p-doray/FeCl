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

#include "Turbo.h"

using namespace fec;

BOOST_CLASS_EXPORT_IMPLEMENT(Turbo);


Turbo::Turbo(const detail::Turbo::Structure& structure,  int workGroupSize) :
Codec(std::unique_ptr<detail::Turbo::Structure>(new detail::Turbo::Structure(structure)), workGroupSize)
{
}

Turbo::Turbo(const EncoderOptions& encoder, const DecoderOptions& decoder, int workGroupSize) :
Codec(std::unique_ptr<detail::Turbo::Structure>(new detail::Turbo::Structure(encoder, decoder)), workGroupSize)
{
}

Turbo::Turbo(const EncoderOptions& encoder, int workGroupSize) :
Codec(std::unique_ptr<detail::Turbo::Structure>(new detail::Turbo::Structure(encoder)), workGroupSize)
{
}

const char * Turbo::get_key() const
{
  return boost::serialization::type_info_implementation<Turbo>::type::get_const_instance().get_key();
}
