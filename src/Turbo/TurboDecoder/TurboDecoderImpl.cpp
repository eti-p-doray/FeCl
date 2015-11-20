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

using namespace fec;

TurboDecoderImpl::TurboDecoderImpl(const Turbo::detail::Structure& structure) : TurboDecoder(structure)
{
}

void TurboDecoderImpl::decodeBlock(std::vector<double>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg)
{
  std::copy(parity, parity + structure().innerParitySize(), parityIn_.begin());
  std::fill(extrinsic_.begin(), extrinsic_.end(), 0);
  for (size_t i = 0; i < structure().iterations(); ++i) {
    if (structure().schedulingType() == Turbo::Parallel) {
      parallelTransferUpdate();
    }
    
    if (structure().schedulingType() == Turbo::Custom) {
      for (size_t j = 0; j < structure().scheduling().size(); ++j) {
        extrinsicBuffer_ = extrinsic_;
        for (size_t k = 0; k < structure().scheduling()[j].transfer.size(); ++k) {
          customTransferUpdate(j, k);
        }
        std::swap(extrinsicBuffer_, extrinsic_);
        customActivationUpdate(i, j);
      }
    } else {
      auto parityIt = parityIn_.begin() + structure().innerSystSize();
      auto extrinsic = extrinsic_.begin();
      for (size_t j = 0; j < structure().constituentCount(); ++j) {
        if (structure().schedulingType() == Turbo::Serial) {
          serialTransferUpdate(j);
        }
        code_[j]->setScalingFactor(structure().scalingFactor(i, j));
        auto inputInfo = Codec::detail::InputIterator().parity(parityIt).syst(extrinsic);
        auto outputInfo = Codec::detail::OutputIterator().syst(extrinsic);
        code_[j]->soDecodeBlock(inputInfo, outputInfo);
        
        extrinsic += structure().constituent(j).innerSystSize();
        parityIt += structure().constituent(j).innerParitySize();
      }
    }
  }
  std::copy(parityIn_.begin(), parityIn_.begin()+structure().innerMsgSize(), parityOut_.begin());
  aPosterioriUpdate();
  
  for (size_t i = 0; i < structure().innerMsgSize(); ++i) {
    msg[i] = parityOut_[i] > 0;
  }
}


void TurboDecoderImpl::soDecodeBlock(Codec::detail::InputIterator input, Codec::detail::OutputIterator output)
{
  std::copy(input.parity(), input.parity() + structure().innerParitySize(), parityIn_.begin());
  if (input.hasSyst()) {
    for (size_t i = 0; i < structure().innerSystSize(); ++i) {
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
      std::fill(output.parity()+structure().innerSystSize(), output.parity()+structure().innerParitySize(), 0);
    }
  }
  
  for (size_t i = 0; i < structure().iterations(); ++i) {
    if (structure().schedulingType() == Turbo::Parallel) {
      parallelTransferUpdate();
    }
    
    if (structure().schedulingType() == Turbo::Custom) {
      for (size_t j = 0; j < structure().scheduling().size(); ++j) {
        extrinsicBuffer_ = extrinsic_;
        for (size_t k = 0; k < structure().scheduling()[j].transfer.size(); ++k) {
          customTransferUpdate(j, k);
        }
        std::swap(extrinsicBuffer_, extrinsic_);
        customActivationUpdate(i, j);
      }
    } else {
      auto parityIn = parityIn_.begin() + structure().innerSystSize();
      auto parityOut = parityOut_.begin() + structure().innerSystSize();
      auto extrinsic = extrinsic_.begin();
      for (size_t j = 0; j < structure().constituentCount(); ++j) {
        if (structure().schedulingType() == Turbo::Serial) {
          serialTransferUpdate(j);
        }
        
        auto inputInfo = Codec::detail::InputIterator({}).parity(parityIn).syst(extrinsic);
        auto outputInfo = Codec::detail::OutputIterator({}).syst(extrinsic);
        if (i == structure().iterations()-1 && output.hasParity()) {
          outputInfo.parity(parityOut);
        }
        code_[j]->soDecodeBlock(inputInfo, outputInfo);
        
        extrinsic += structure().constituent(j).innerSystSize();
        parityIn += structure().constituent(j).innerParitySize();
        parityOut += structure().constituent(j).innerParitySize();
      }
    }
  }
  std::fill(parityOut_.begin(), parityOut_.begin() + structure().innerSystSize(), 0);
  aPosterioriUpdate();

  if (output.hasSyst()) {
    std::copy(parityOut_.begin(), parityOut_.begin()+structure().innerSystSize(), output.syst());
  }
  if (output.hasParity()) {
    std::copy(parityOut_.begin(), parityOut_.end(), output.parity());
  }
  if (output.hasState()) {
    std::copy(extrinsic_.begin(), extrinsic_.end(), output.state());
  }
  if (output.hasMsg()) {
    for (size_t i = 0; i < structure().innerMsgSize(); ++i) {
      output.msg()[i] = parityIn_[i] + parityOut_[i];
    }
  }
}

void TurboDecoderImpl::aPosterioriUpdate()
{
  auto extrinsic = extrinsic_.begin();
  auto systTail = parityOut_.begin() + structure().innerMsgSize();
  auto syst = parityOut_.begin();
  for (size_t j = 0; j < structure().constituentCount(); ++j) {
    for (size_t k = 0; k < structure().constituent(j).innerMsgSize(); ++k) {
      syst[structure().interleaver(j)[k]] += extrinsic[k];
    }
    extrinsic += structure().constituent(j).innerMsgSize();
    for (size_t k = 0; k < structure().constituent(j).systTailSize(); ++k) {
      systTail[k] += extrinsic[k];
    }
    extrinsic += structure().constituent(j).systTailSize();
    systTail += structure().constituent(j).systTailSize();
  }
}
  
void TurboDecoderImpl::customActivationUpdate(size_t i, size_t stage)
{
  auto parityIt = parityIn_.begin() + structure().innerSystSize();
  auto extrinsic = extrinsic_.begin();
  auto activation = structure().scheduling()[stage].activation.begin();
  for (size_t j = 0; j < structure().constituentCount(); ++j) {
    while (activation != structure().scheduling()[stage].activation.end() && *activation < j) {++activation;}
    if (activation != structure().scheduling()[stage].activation.end() && *activation == j) {
      code_[j]->setScalingFactor(structure().scalingFactor(i, j));
      auto inputInfo = Codec::detail::InputIterator().parity(parityIt).syst(extrinsic);
      auto outputInfo = Codec::detail::OutputIterator().syst(extrinsic);
      code_[j]->soDecodeBlock(inputInfo, outputInfo);
    } else if (activation == structure().scheduling()[stage].activation.end()) {
      break;
    }
    
    extrinsic += structure().constituent(j).innerSystSize();
    parityIt += structure().constituent(j).innerParitySize();
  }
}

void TurboDecoderImpl::parallelTransferUpdate()
{
  auto extrinsic = extrinsic_.begin();
  auto extrinsicTmp = extrinsicBuffer_.begin();
  
  auto systTail = parityIn_.begin() + structure().innerMsgSize();
  auto syst = parityOut_.begin();
  std::fill(parityOut_.begin(), parityOut_.begin() + structure().innerMsgSize(), 0);
  for (size_t j = 0; j < structure().constituentCount(); ++j) {
    for (size_t k = 0; k < structure().constituent(j).innerMsgSize(); ++k) {
      extrinsicTmp[k] = extrinsic[k];
      extrinsic[k] = syst[structure().interleaver(j)[k]];
      syst[structure().interleaver(j)[k]] += extrinsicTmp[k];
    }
    extrinsic += structure().constituent(j).innerMsgSize();
    extrinsicTmp += structure().constituent(j).innerMsgSize();
    
    std::copy(systTail, systTail + structure().constituent(j).systTailSize(), extrinsic);
    
    extrinsic += structure().constituent(j).systTailSize();
    systTail += structure().constituent(j).systTailSize();
  }
  std::copy(parityIn_.begin(), parityIn_.begin() + structure().innerMsgSize(), parityOut_.begin());
  
  for (int64_t j = structure().constituentCount()-1; j >= 0; --j) {
    extrinsic -= structure().constituent(j).innerSystSize();
    extrinsicTmp -= structure().constituent(j).innerMsgSize();
    for (size_t k = 0; k < structure().constituent(j).innerMsgSize(); ++k) {
      extrinsic[k] += syst[structure().interleaver(j)[k]];
      syst[structure().interleaver(j)[k]] += extrinsicTmp[k];
    }
  }
}

void TurboDecoderImpl::serialTransferUpdate(size_t i)
{
  auto extrinsic = extrinsic_.begin();
  auto systTail = parityIn_.begin() + structure().innerMsgSize();
  auto syst = parityOut_.begin();
  std::copy(parityIn_.begin(), parityIn_.begin() + structure().innerMsgSize(), parityOut_.begin());
  for (size_t j = 0; j < i; ++j) {
    for (size_t k = 0; k < structure().constituent(j).innerMsgSize(); ++k) {
      syst[structure().interleaver(j)[k]] += extrinsic[k];
    }
    extrinsic += structure().constituent(j).innerSystSize();
    systTail += structure().constituent(j).systTailSize();
  }
  auto extrinsicTmp = extrinsic;
  extrinsic += structure().constituent(i).innerMsgSize();
  std::copy(systTail, systTail + structure().constituent(i).systTailSize(), extrinsic);
  extrinsic += structure().constituent(i).systTailSize();
  for (size_t j = i+1; j < structure().constituentCount(); ++j) {
    for (size_t k = 0; k < structure().constituent(j).innerMsgSize(); ++k) {
      syst[structure().interleaver(j)[k]] += extrinsic[k];
    }
    extrinsic += structure().constituent(j).innerSystSize();
  }
  
  structure().interleaver(i).template permuteBlock<double>(syst, extrinsicTmp);
}

void TurboDecoderImpl::customTransferUpdate(size_t stage, size_t src)
{
  size_t i = structure().scheduling()[stage].activation[src];
  auto extrinsic = extrinsic_.begin();
  auto extrinsicTmp = extrinsicBuffer_.begin();
  auto systTail = parityIn_.begin() + structure().innerMsgSize();
  auto syst = parityOut_.begin();
  auto transfer = structure().scheduling()[stage].transfer[src].begin();
  std::copy(parityIn_.begin(), parityIn_.begin() + structure().innerMsgSize(), parityOut_.begin());
  for (size_t j = 0; j < i; ++j) {
    while (transfer != structure().scheduling()[stage].transfer[src].end() && *transfer < j) {++transfer;}
    if (transfer != structure().scheduling()[stage].transfer[src].end() && *transfer == j) {
      for (size_t k = 0; k < structure().constituent(j).innerMsgSize(); ++k) {
        syst[structure().interleaver(j)[k]] += extrinsic[k];
      }
    }
    extrinsic += structure().constituent(j).innerSystSize();
    extrinsicTmp += structure().constituent(j).innerSystSize();
    systTail += structure().constituent(j).systTailSize();
  }
  auto extrinsicConst = extrinsicTmp;
  extrinsic += structure().constituent(i).innerMsgSize();
  std::copy(systTail, systTail + structure().constituent(i).systTailSize(), extrinsic);
  extrinsic += structure().constituent(i).systTailSize();
  for (size_t j = i+1; j < structure().constituentCount(); ++j) {
    while (transfer != structure().scheduling()[stage].transfer[src].end() && *transfer < j) {++transfer;}
    if (transfer != structure().scheduling()[stage].transfer[src].end() && *transfer == j) {
      for (size_t k = 0; k < structure().constituent(j).innerMsgSize(); ++k) {
        syst[structure().interleaver(j)[k]] += extrinsic[k];
      }
    } else if (transfer == structure().scheduling()[stage].transfer[src].end()) {
      break;
    }
    extrinsic += structure().constituent(j).innerSystSize();
  }
  
  structure().interleaver(i).template permuteBlock<double>(syst, extrinsicConst);
}

