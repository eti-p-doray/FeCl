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

#include "Demodulator.h"
#include "DemodulatorImpl.h"

using namespace fec;
using namespace fec::detail;

/**
 *  MapDecoder creator function.
 *  Construct in a factory behavior a MapCodec object corresponding to the algorithm
 *  version in use.
 *  \param  codeStructure Convolutional code structure describing the code
 *  \return MacDecoder specialization suitable for the algorithm in use
 */
std::unique_ptr<Demodulator> Demodulator::create(const Modulation::Structure& structure)
{
  switch (structure.decoderAlgorithm()) {
    default:
    case Exact:
      return std::unique_ptr<Demodulator>(new DemodulatorImpl<FloatLlrMetrics, LogSum>(structure));

    case Linear:
      return std::unique_ptr<Demodulator>(new DemodulatorImpl<FloatLlrMetrics, LinearLogSum>(structure));

    case Approximate:
      return std::unique_ptr<Demodulator>(new DemodulatorImpl<FloatLlrMetrics, MaxLogSum>(structure));
  }
}

/**
 *  Constructor.
 *  Allocates metric buffers based on the given code structure.
 *  \param  codeStructure Convolutional code structure describing the code
 */
Demodulator::Demodulator(const Modulation::Structure& structure) :
structure_(structure)
{
}
