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
  size_ = std::log2(mod.constellation_.size());
  length_ = mod.length();
  if (mod.constellation_.size() != 1<<size()) {
    throw std::invalid_argument("wrong sive for constellation");
  }
  
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
        
void Modulation::Structure::modulate(std::vector<BitField<size_t>>::const_iterator word, std::vector<double>::iterator symbol) const
{
  for (size_t i = 0; i < length(); ++i) {
    BitField<size_t> input = 0;
    for (int j = 0; j < size(); j++) {
      input.set(j, word[j]);
    }
    input *= dimension();
    for (int j = 0; j < dimension(); ++j) {
      symbol[j] = constellation_[input+j];
    }
    word += size();
    symbol += symbolWidth();
  }
}

void Modulation::Structure::demodulate(std::vector<double>::const_iterator symbol, std::vector<BitField<size_t>>::iterator word) const
{
  for (size_t i = 0; i < length(); ++i) {
    double max = 0;
    BitField<size_t> maxInput = 0;
    for (size_t j = 0; j < 1<<size(); ++j) {
      double tmp = -sqDistance<double>(symbol, constellation().begin() + j*dimension(), dimension());
      if (tmp > max) {
        maxInput = j;
      }
    }
    for (int j = 0; j < size(); j++) {
      word[j] = maxInput.test(j);
    }
    word += size();
    symbol += symbolWidth();
  }
}

        

