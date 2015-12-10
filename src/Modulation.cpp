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

#include "Modulation.h"
#include "detail/Demodulator/Demodulator.h"

using namespace fec;

BOOST_CLASS_EXPORT_IMPLEMENT(Modulation);

Modulation::Modulation(const detail::Modulation::Structure& structure)
{
  structure_ = std::unique_ptr<detail::Modulation::Structure>(new detail::Modulation::Structure(structure));
}

Modulation::Modulation(const ModOptions& mod, const DemodOptions& demod)
{
  structure_ = std::unique_ptr<detail::Modulation::Structure>(new detail::Modulation::Structure(mod, demod));
}

Modulation::Modulation(const ModOptions& mod)
{
  structure_ = std::unique_ptr<detail::Modulation::Structure>(new detail::Modulation::Structure(mod));
}

const char * Modulation::get_key() const {
  return boost::serialization::type_info_implementation<Modulation>::type::get_const_instance().get_key();
}

void Modulation::modulateBlocks(detail::Modulation::const_iterator<BitField<size_t>> first, detail::Modulation::const_iterator<BitField<size_t>> last, detail::Modulation::iterator<double> output) const
{
  while (first != last) {
    structure().modulate(first.at(detail::Modulation::Word), output.at(detail::Modulation::Symbol));
    ++first; ++output;
  }
}

void Modulation::demodulateBlocks(detail::Modulation::const_iterator<double> first, detail::Modulation::const_iterator<double> last, detail::Modulation::iterator<BitField<size_t>> output) const
{
  while (first != last) {
    structure().demodulate(first.at(detail::Modulation::Symbol), output.at(detail::Modulation::Word));
    ++first; ++output;
  }
}

void Modulation::soDemodulateBlocks(detail::Modulation::const_iterator<double> first, detail::Modulation::const_iterator<double> last, double variance, detail::Modulation::iterator<double> output) const
{
  auto worker = detail::Demodulator::create(structure());
  worker->soDemodulateBlocks(first, last, 1/(2*variance), output.at(fec::detail::Modulation::Word));
}