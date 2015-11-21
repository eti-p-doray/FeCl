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

#include "TurboDecoderImpl.h"

using namespace fec::detail;

TurboDecoderImpl::TurboDecoderImpl(const Turbo::Structure& structure) : TurboDecoder(structure)
{
}

void TurboDecoderImpl::decodeBlock(std::vector<double>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg)
{
  std::copy(parity, parity + structure().paritySize(), parityIn_.begin());
  std::fill(extrinsic_.begin(), extrinsic_.end(), 0);
  for (size_t i = 0; i < structure().iterations(); ++i) {
    if (structure().schedulingType() == Parallel) {
      parallelTransferUpdate();
    }
    
    if (structure().schedulingType() == Custom) {
      for (size_t j = 0; j < structure().scheduling().size(); ++j) {
        extrinsicBuffer_ = extrinsic_;
        for (size_t k = 0; k < structure().scheduling()[j].transfer.size(); ++k) {
          customTransferUpdate(j, k);
        }
        std::swap(extrinsicBuffer_, extrinsic_);
        customActivationUpdate(i, j);
      }
    } else {
      auto parityIt = parityIn_.begin() + structure().systSize();
      auto extrinsic = extrinsic_.begin();
      for (size_t j = 0; j < structure().constituentCount(); ++j) {
        if (structure().schedulingType() == Serial) {
          serialTransferUpdate(j);
        }
        code_[j]->setScalingFactor(structure().scalingFactor(i, j));
        auto inputInfo = Codec::InputIterator().parity(parityIt).syst(extrinsic);
        auto outputInfo = Codec::OutputIterator().syst(extrinsic);
        code_[j]->soDecodeBlock(inputInfo, outputInfo);
        
        extrinsic += structure().constituent(j).systSize();
        parityIt += structure().constituent(j).paritySize();
      }
    }
  }
  std::copy(parityIn_.begin(), parityIn_.begin()+structure().msgSize(), parityOut_.begin());
  aPosterioriUpdate();
  
  for (size_t i = 0; i < structure().msgSize(); ++i) {
    msg[i] = parityOut_[i] > 0;
  }
}


void TurboDecoderImpl::soDecodeBlock(Codec::InputIterator input, Codec::OutputIterator output)
{
  std::copy(input.parity(), input.parity() + structure().paritySize(), parityIn_.begin());
  if (input.hasSyst()) {
    for (size_t i = 0; i < structure().systSize(); ++i) {
      parityIn_[i] += input.syst()[i];
    }
  }
  
  if (input.hasState()) {
    std::copy(input.state(), input.state()+structure().stateSize(), extrinsic_.begin());
  }
  else {
    std::fill(extrinsic_.begin(), extrinsic_.end(), 0);
  }
  
  if (structure().iterations() == 0) {
    if (output.hasParity()) {
      std::fill(output.parity()+structure().systSize(), output.parity()+structure().paritySize(), 0);
    }
  }
  
  for (size_t i = 0; i < structure().iterations(); ++i) {
    if (structure().schedulingType() == Parallel) {
      parallelTransferUpdate();
    }
    
    if (structure().schedulingType() == Custom) {
      for (size_t j = 0; j < structure().scheduling().size(); ++j) {
        extrinsicBuffer_ = extrinsic_;
        for (size_t k = 0; k < structure().scheduling()[j].transfer.size(); ++k) {
          customTransferUpdate(j, k);
        }
        std::swap(extrinsicBuffer_, extrinsic_);
        customActivationUpdate(i, j);
      }
    } else {
      auto parityIn = parityIn_.begin() + structure().systSize();
      auto parityOut = parityOut_.begin() + structure().systSize();
      auto extrinsic = extrinsic_.begin();
      for (size_t j = 0; j < structure().constituentCount(); ++j) {
        if (structure().schedulingType() == Serial) {
          serialTransferUpdate(j);
        }
        
        auto inputInfo = Codec::InputIterator({}).parity(parityIn).syst(extrinsic);
        auto outputInfo = Codec::OutputIterator({}).syst(extrinsic);
        if (i == structure().iterations()-1 && output.hasParity()) {
          outputInfo.parity(parityOut);
        }
        code_[j]->soDecodeBlock(inputInfo, outputInfo);
        
        extrinsic += structure().constituent(j).systSize();
        parityIn += structure().constituent(j).paritySize();
        parityOut += structure().constituent(j).paritySize();
      }
    }
  }
  std::fill(parityOut_.begin(), parityOut_.begin() + structure().systSize(), 0);
  aPosterioriUpdate();

  if (output.hasSyst()) {
    std::copy(parityOut_.begin(), parityOut_.begin()+structure().systSize(), output.syst());
  }
  if (output.hasParity()) {
    std::copy(parityOut_.begin(), parityOut_.end(), output.parity());
  }
  if (output.hasState()) {
    std::copy(extrinsic_.begin(), extrinsic_.end(), output.state());
  }
  if (output.hasMsg()) {
    for (size_t i = 0; i < structure().msgSize(); ++i) {
      output.msg()[i] = parityIn_[i] + parityOut_[i];
    }
  }
}

void TurboDecoderImpl::aPosterioriUpdate()
{
  auto extrinsic = extrinsic_.begin();
  auto systTail = parityOut_.begin() + structure().msgSize();
  auto syst = parityOut_.begin();
  for (size_t j = 0; j < structure().constituentCount(); ++j) {
    for (size_t k = 0; k < structure().constituent(j).msgSize(); ++k) {
      syst[structure().interleaver(j)[k]] += extrinsic[k];
    }
    extrinsic += structure().constituent(j).msgSize();
    for (size_t k = 0; k < structure().constituent(j).systTailSize(); ++k) {
      systTail[k] += extrinsic[k];
    }
    extrinsic += structure().constituent(j).systTailSize();
    systTail += structure().constituent(j).systTailSize();
  }
}
  
void TurboDecoderImpl::customActivationUpdate(size_t i, size_t stage)
{
  auto parityIt = parityIn_.begin() + structure().systSize();
  auto extrinsic = extrinsic_.begin();
  auto activation = structure().scheduling()[stage].activation.begin();
  for (size_t j = 0; j < structure().constituentCount(); ++j) {
    while (activation != structure().scheduling()[stage].activation.end() && *activation < j) {++activation;}
    if (activation != structure().scheduling()[stage].activation.end() && *activation == j) {
      code_[j]->setScalingFactor(structure().scalingFactor(i, j));
      auto inputInfo = Codec::InputIterator().parity(parityIt).syst(extrinsic);
      auto outputInfo = Codec::OutputIterator().syst(extrinsic);
      code_[j]->soDecodeBlock(inputInfo, outputInfo);
    } else if (activation == structure().scheduling()[stage].activation.end()) {
      break;
    }
    
    extrinsic += structure().constituent(j).systSize();
    parityIt += structure().constituent(j).paritySize();
  }
}

void TurboDecoderImpl::parallelTransferUpdate()
{
  auto extrinsic = extrinsic_.begin();
  auto extrinsicTmp = extrinsicBuffer_.begin();
  
  auto systTail = parityIn_.begin() + structure().msgSize();
  auto syst = parityOut_.begin();
  std::fill(parityOut_.begin(), parityOut_.begin() + structure().msgSize(), 0);
  for (size_t j = 0; j < structure().constituentCount(); ++j) {
    for (size_t k = 0; k < structure().constituent(j).msgSize(); ++k) {
      extrinsicTmp[k] = extrinsic[k];
      extrinsic[k] = syst[structure().interleaver(j)[k]];
      syst[structure().interleaver(j)[k]] += extrinsicTmp[k];
    }
    extrinsic += structure().constituent(j).msgSize();
    extrinsicTmp += structure().constituent(j).msgSize();
    
    std::copy(systTail, systTail + structure().constituent(j).systTailSize(), extrinsic);
    
    extrinsic += structure().constituent(j).systTailSize();
    systTail += structure().constituent(j).systTailSize();
  }
  std::copy(parityIn_.begin(), parityIn_.begin() + structure().msgSize(), parityOut_.begin());
  
  for (int64_t j = structure().constituentCount()-1; j >= 0; --j) {
    extrinsic -= structure().constituent(j).systSize();
    extrinsicTmp -= structure().constituent(j).msgSize();
    for (size_t k = 0; k < structure().constituent(j).msgSize(); ++k) {
      extrinsic[k] += syst[structure().interleaver(j)[k]];
      syst[structure().interleaver(j)[k]] += extrinsicTmp[k];
    }
  }
}

void TurboDecoderImpl::serialTransferUpdate(size_t i)
{
  auto extrinsic = extrinsic_.begin();
  auto systTail = parityIn_.begin() + structure().msgSize();
  auto syst = parityOut_.begin();
  std::copy(parityIn_.begin(), parityIn_.begin() + structure().msgSize(), parityOut_.begin());
  for (size_t j = 0; j < i; ++j) {
    for (size_t k = 0; k < structure().constituent(j).msgSize(); ++k) {
      syst[structure().interleaver(j)[k]] += extrinsic[k];
    }
    extrinsic += structure().constituent(j).systSize();
    systTail += structure().constituent(j).systTailSize();
  }
  auto extrinsicTmp = extrinsic;
  extrinsic += structure().constituent(i).msgSize();
  std::copy(systTail, systTail + structure().constituent(i).systTailSize(), extrinsic);
  extrinsic += structure().constituent(i).systTailSize();
  for (size_t j = i+1; j < structure().constituentCount(); ++j) {
    for (size_t k = 0; k < structure().constituent(j).msgSize(); ++k) {
      syst[structure().interleaver(j)[k]] += extrinsic[k];
    }
    extrinsic += structure().constituent(j).systSize();
  }
  
  structure().interleaver(i).template permuteBlock<double>(syst, extrinsicTmp);
}

void TurboDecoderImpl::customTransferUpdate(size_t stage, size_t src)
{
  size_t i = structure().scheduling()[stage].activation[src];
  auto extrinsic = extrinsic_.begin();
  auto extrinsicTmp = extrinsicBuffer_.begin();
  auto systTail = parityIn_.begin() + structure().msgSize();
  auto syst = parityOut_.begin();
  auto transfer = structure().scheduling()[stage].transfer[src].begin();
  std::copy(parityIn_.begin(), parityIn_.begin() + structure().msgSize(), parityOut_.begin());
  for (size_t j = 0; j < i; ++j) {
    while (transfer != structure().scheduling()[stage].transfer[src].end() && *transfer < j) {++transfer;}
    if (transfer != structure().scheduling()[stage].transfer[src].end() && *transfer == j) {
      for (size_t k = 0; k < structure().constituent(j).msgSize(); ++k) {
        syst[structure().interleaver(j)[k]] += extrinsic[k];
      }
    }
    extrinsic += structure().constituent(j).systSize();
    extrinsicTmp += structure().constituent(j).systSize();
    systTail += structure().constituent(j).systTailSize();
  }
  auto extrinsicConst = extrinsicTmp;
  extrinsic += structure().constituent(i).msgSize();
  std::copy(systTail, systTail + structure().constituent(i).systTailSize(), extrinsic);
  extrinsic += structure().constituent(i).systTailSize();
  for (size_t j = i+1; j < structure().constituentCount(); ++j) {
    while (transfer != structure().scheduling()[stage].transfer[src].end() && *transfer < j) {++transfer;}
    if (transfer != structure().scheduling()[stage].transfer[src].end() && *transfer == j) {
      for (size_t k = 0; k < structure().constituent(j).msgSize(); ++k) {
        syst[structure().interleaver(j)[k]] += extrinsic[k];
      }
    } else if (transfer == structure().scheduling()[stage].transfer[src].end()) {
      break;
    }
    extrinsic += structure().constituent(j).systSize();
  }
  
  structure().interleaver(i).template permuteBlock<double>(syst, extrinsicConst);
}

