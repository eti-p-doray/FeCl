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
#include "LlrMetrics.h"

using namespace fec;
using namespace fec::detail;

Modulation::Structure::Structure(const ModOptions& mod, const DemodOptions& demod)
{
  setModOptions(mod);
  setDemodOptions(demod);
}

Modulation::Structure::Structure(const ModOptions& mod)
{
  setModOptions(mod);
  setDemodOptions(DemodOptions());
}

void Modulation::Structure::setModOptions(const ModOptions& mod)
{
  width_ = mod.wordWidth();
  size_ = std::log2(mod.constellation_.size());
  length_ = mod.length();
  if (mod.constellation_.size() != 1<<size()) {
    throw std::invalid_argument("invalid size for constellation");
  }
  if (size_ % width_ != 0) {
    throw std::invalid_argument("invalid width");
  }
  size_ /= width_;
  
  dimension_ = mod.constellation_[0].size();
  constellation_.resize(dimension()*mod.constellation_.size());
  for (size_t i = 0; i < mod.constellation_.size(); ++i) {
    if (mod.constellation_[i].size() != dimension_) {
      throw std::invalid_argument("constellation dimension is not consistent");
    }
    for (size_t j = 0; j < mod.constellation_[i].size(); ++j) {
      constellation_[i*dimension()+j] = mod.constellation_[i][j];
    }
  }
}

void Modulation::Structure::setDemodOptions(const DemodOptions& demod)
{
  algorithm_ = demod.algorithm_;
  scalingFactor_ = demod.scalingFactor_;
}
