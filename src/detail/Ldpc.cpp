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

#include "Ldpc.h"

using namespace fec;
using namespace fec::detail;

BOOST_CLASS_EXPORT_IMPLEMENT(Ldpc::Structure);


const char * Ldpc::Structure::get_key() const {
  return boost::serialization::type_info_implementation<Ldpc::Structure>::type::get_const_instance().get_key();
}

Ldpc::Structure::Structure(const EncoderOptions& encoder, const DecoderOptions& decoder)
{
  setEncoderOptions(encoder);
  setDecoderOptions(decoder);
}

Ldpc::Structure::Structure(const EncoderOptions& encoder)
{
  setEncoderOptions(encoder);
  setDecoderOptions(DecoderOptions());
}

void Ldpc::Structure::setEncoderOptions(const EncoderOptions& encoder)
{
  msgSize_ = encoder.checkMatrix_.cols()-encoder.checkMatrix_.rows();
  paritySize_ = encoder.checkMatrix_.cols();
  stateSize_ = encoder.checkMatrix_.size();
  
  computeGeneratorMatrix(encoder.checkMatrix_);
  
  systSize_ = msgSize_;
}

void Ldpc::Structure::setDecoderOptions(const DecoderOptions& decoder)
{
  decoderAlgorithm_ = decoder.algorithm_;
  iterations_ = decoder.iterations_;
  scalingFactor_ = scalingMapToVector(decoder.scalingFactor_);
}

std::vector<std::vector<double>> Ldpc::Structure::scalingMapToVector(const std::unordered_map<size_t,std::vector<double>>& map) const
{
  auto degree = checks().rowSizes();
  size_t maxDegree = *std::max_element(degree.begin(), degree.end());
  std::vector<std::vector<double>> vec;
  auto def = map.find(0);
  if (def != map.end()) {
    if (def->second.size() != 1 && def->second.size() != iterations()) {
      throw std::invalid_argument("Wrong size for scaling factor");
    }
    vec.resize(def->second.size());
    for (size_t i = 0; i < vec.size(); ++i) {
      vec[i].assign(maxDegree-1, def->second[i]);
    }
    for (auto i = map.begin(); i != map.end(); ++i) {
      if (i->first-2 < maxDegree-1 && i->first >= 2) {
        if (i->second.size() != vec.size()) {
          throw std::invalid_argument("Wrong size for scaling factor");
        }
        for (size_t j = 0; j < vec.size(); ++j) {
          vec[j][i->first-2] = i->second[j];
        }
      }
    }
  } else {
    size_t length = 0;
    for (auto i = map.begin(); i != map.end(); ++i) {
      length = std::max(i->second.size(), length);
    }
    vec.resize(length, std::vector<double>(maxDegree-1));
    if (length != 1 && length != iterations()) {
      throw std::invalid_argument("Wrong size for scaling factor");
    }
    for (auto i = degree.begin(); i < degree.end(); ++i) {
      auto it = map.find(*i);
      if (it != map.end()) {
        if (it->second.size() != length) {
          throw std::invalid_argument("Wrong size for scaling factor");
        }
        for (size_t j = 0; j < vec.size(); ++j) {
          vec[j][it->first-2] = it->second[j];
        }
      } else {
        throw std::invalid_argument("Scaling factor not defined and no default");
      }
    }
  }
  return vec;
}

std::unordered_map<size_t,std::vector<double>> Ldpc::Structure::scalingVectorToMap(const std::vector<std::vector<double>>& vec) const
{
  std::unordered_map<size_t,std::vector<double>> scaling;
  for (size_t i = 0; i < vec.size(); ++i) {
    for (size_t j = 0; j < vec[i].size(); ++j) {
      auto it = scaling.find(j+2);
      if (it == scaling.end()) {
        scaling.insert(std::make_pair(j+2, std::vector<double>(vec.size())));
      }
      scaling[j+2][i] = vec[i][j];
    }
  }
  return scaling;
}

Ldpc::DecoderOptions Ldpc::Structure::getDecoderOptions() const
{
  return DecoderOptions().iterations(iterations()).algorithm(decoderAlgorithm()).scalingFactor(scalingVectorToMap(scalingFactor_));
}

double Ldpc::Structure::scalingFactor(size_t i, size_t j) const
{
  i %= scalingFactor_.size();
  return scalingFactor_[i][j-2];
}

/**
 *  Computes the syndrome given a sequence of parity bits.
 *  \param  parity  Input iterator pointing to the first element of the parity sequence.
 *  \param  syndrome[out] Output iterator pointing to the first
 *    element of the computed syndrome. The output needs to be allocated.
 */
void Ldpc::Structure::syndrome(std::vector<uint8_t>::const_iterator parity, std::vector<uint8_t>::iterator syndrome) const
{
  for (auto parityEq = checks().begin(); parityEq < checks().end(); ++parityEq, ++syndrome) {
    for (auto parityBit = parityEq->begin(); parityBit < parityEq->end(); ++parityBit) {
      *syndrome ^= parity[*parityBit];
    }
  }
}

/**
 *  Checks for the parity sequence consistency using its syndrome.
 *  \param  parity  Input iterator pointing to the first element of the parity sequence.
 *  \return True if the parity sequence is consistent. False otherwise.
 */
bool Ldpc::Structure::check(std::vector<BitField<size_t>>::const_iterator parity) const
{
  for (auto parityEq = checks().begin(); parityEq < checks().end(); ++parityEq) {
    bool syndrome = false;
    for (auto parityBit = parityEq->begin(); parityBit < parityEq->end(); ++parityBit) {
      syndrome ^= bool(parity[*parityBit]);
    }
    if (syndrome != false) {
      return false;
    }
  }
  return true;
}

/**
 *  Encodes a sequence of msg bits using the transformed ldpc matrix.
 *  \param  msg Input iterator pointing to the first element of the msg bit sequence.
 *  \param  parity[out] Output iterator pointing to the first
 *    element of the computed parity sequence. The output needs to be allocated.
 */
void Ldpc::Structure::encode(std::vector<BitField<size_t>>::const_iterator msg, std::vector<BitField<size_t>>::iterator parity) const
{
  std::copy(msg, msg + msgSize(), parity);
  std::fill(parity+msgSize(), parity+checks().cols(), 0);
  parity += msgSize();
  auto parityIt = parity;
  for (auto row = DC_.begin(); row < DC_.end(); ++row, ++parityIt) {
    for (auto elem = row->begin(); elem != row->end(); ++elem) {
      *parityIt ^= msg[*elem];
    }
  }
  for (auto row = B_.begin(); row < B_.end(); ++row, ++parityIt) {
    for (auto elem = row->begin(); elem < row->end(); ++elem) {
      *parityIt ^= parity[*elem];
    }
  }
  parity += DC_.rows();
  parityIt = parity;
  for (auto row = A_.begin(); row < A_.end(); ++row, ++parityIt) {
    for (auto elem = row->begin(); elem < row->end(); ++elem) {
      *parityIt ^= msg[*elem];
    }
  }
  parityIt = parity;
  for (auto row = T_.begin(); row < T_.end(); ++row, ++parityIt) {
    for (auto elem = row->begin()+1; elem < row->end(); ++elem) {
      parity[*elem] ^= *parityIt;
    }
  }
}

/**
 *  Transforms an ldpc matrix to allow in-place encoding.
 *  The matrix is transformed in a partial triangular shape.
 *  \param  H The original ldpc matrix
 */
void Ldpc::Structure::computeGeneratorMatrix(SparseBitMatrix H)
{
  std::vector<size_t> colSizes;
  size_t maxRow = H.rows();
  size_t tSize = 0;
  
  H.colSizes(colSizes);
  auto colSize = H.end()-1;
  for (size_t i = H.cols(); i > 0; --i) {
    for (; colSize >= H.begin() + maxRow; --colSize) {
      for (auto elem = colSize->begin(); elem < colSize->end(); ++elem) {
        colSizes[*elem]--;
      }
    }
    size_t minValue = -1;
    size_t minIdx = 0;
    for (int64_t j = i-1; j >= 0; j--) {
      if (colSizes[j] == 1) {
        minIdx = j;
        minValue = colSizes[j];
        break;
      }
      else if (colSizes[j] < minValue && colSizes[j] >= 1) {
        minIdx = j;
        minValue = colSizes[j];
      }
    }
    H.swapCols(minIdx, i-1);
    std::swap(colSizes[minIdx], colSizes[i-1]);
    size_t j = maxRow;
    for (auto row = H.begin(); row < H.begin()+j; ++row) {
      if (row->test(i-1)) {
        --j;
        std::swap(H[j], *row);
        --row;
      }
    }
    maxRow -= std::max(minValue, size_t(1));
    if (maxRow == 0) {
      tSize = H.cols()-i+1;
      break;
    }
  }
  
  for (size_t i = 0; i < tSize; ++i) {
    auto row = H.begin()+i;
    if (!row->test(i+H.cols()-tSize)) {
      ++row;
      for (; row < H.end(); ++row) {
        if (row->test(i+H.cols()-tSize)) {
          std::swap(H[i], *row);
          break;
        }
      }
    }
  }
  
  BitMatrix CDE = H({tSize, H.rows()}, {0, H.cols()});
  
  for (int64_t i = tSize-1; i >= 0; --i) {
    for (auto row = CDE.begin(); row < CDE.end(); ++row) {
      if (row->test(i+CDE.cols()-tSize)) {
        *row += H[i];
      }
    }
  }
  
  for (size_t i = 0; i < CDE.cols()-tSize-msgSize(); ++i) {
    uint8_t found = false;
    for (auto row = CDE.begin()+i; row < CDE.end(); ++row) {
      if (row->test(i+msgSize())) {
        std::swap(*row, CDE[i]);
        found = true;
        break;
      }
    }
    if (!found) {
      for (auto row = CDE.begin()+i; row < CDE.end(); ++row) {
        size_t k = row->first();
        if (k != -1) {
          H.swapCols(k, i+msgSize());
          CDE.swapCols(k, i+msgSize());
          std::swap(*row, CDE[i]);
          found = true;
          break;
        }
      }
    }
    if (!found) {
      H.moveCol(i+msgSize(), msgSize());
      CDE.moveCol(i+msgSize(), msgSize());
      ++msgSize_;
      --i;
      continue;
    }
    for (auto row = CDE.begin()+i+1; row < CDE.end(); ++row) {
      if (row->test(i+msgSize())) {
        *row += CDE[i];
      }
    }
  }
  
  for (int64_t i = CDE.rows()-1; i >= 0; --i) {
    for (auto row = CDE.begin()+i-1; row >= CDE.begin(); --row) {
      if (row->test(i+msgSize())) {
        *row += CDE[i];
      }
    }
  }
  //std::cout << H << std::endl;
  H_ = H;
  DC_ = CDE({0, CDE.cols()-msgSize()-tSize}, {0, msgSize()});
  A_ = H_({0, tSize}, {0, msgSize()});
  B_ = H_({0, tSize}, {msgSize(), msgSize()+DC_.rows()});
  T_ = H_({0, tSize}, {H.cols()-tSize, H.cols()}).transpose();
}

fec::Permutation Ldpc::Structure::puncturing(const PunctureOptions& options) const
{
  std::vector<size_t> perms;
  size_t idx = 0;
  for (size_t i = 0; i < systSize(); ++i) {
    if ((options.systMask_.size() == 0 && (options.mask_.size() == 0 || options.mask_[i % options.mask_.size()])) ||
        (options.systMask_.size() != 0 && (options.systMask_[idx % options.systMask_.size()]))) {
      perms.push_back(idx);
      ++idx;
    }
  }
  for (size_t i = 0; i < paritySize() - systSize(); ++i) {
    if ((options.systMask_.size() == 0 && (options.mask_.size() == 0 || options.mask_[i % options.mask_.size()])) ||
        (options.systMask_.size() != 0 && (options.mask_.size() == 0 || options.mask_[idx % options.mask_.size()]))) {
      perms.push_back(idx);
    }
    ++idx;
  }
  
  return Permutation(perms, paritySize());
}
