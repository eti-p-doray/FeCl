/*******************************************************************************
 *  \file LdpcCodeStructure.cpp
 *  \author Etienne Pierre-Doray
 *  \since 2015-06-16
 *  \version Last update : 2015-06-16
 *
 *  Declaration of the ConvolutionalCodeStructure class
 ******************************************************************************/

#include "LdpcCodeStructure.h"

using namespace fec;

SparseBitMatrix LdpcCodeStructure::gallagerConstruction(size_t n, size_t wc, size_t wr)
{
  SparseBitMatrix H(n/wr * wc, n, wr);
  
  size_t elem = 0;
  auto row = H.begin();
  for (size_t i = 0; i < n/wr; i++) {
    for (size_t j = 0; j < wr; j++) {
      for (size_t k = 0; k < H.rows(); k+=n/wr) {
        row[k].set(elem);
      }
      elem++;
    }
    ++row;
  }
  
  uint64_t seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::mt19937 generator((int)seed);
  for (size_t j = 0; j < H.cols(); j++) {
    std::uniform_int_distribution<int> distribution(int(j),int(H.cols()-1));
    for (size_t i = n/wr; i < H.rows(); i+=n/wr) {
      H.swapCols(j, distribution(generator), {i, i+n/wr});
    }
  }
  
  return H;
}

LdpcCodeStructure::LdpcCodeStructure(const SparseBitMatrix& H, size_t iterationCount, DecoderType type) : CodeStructure(H.cols()-H.rows(), H.cols())
{
  computeGeneratorMatrix(SparseBitMatrix(H));
  
  decoderType_ = type;
  iterationCount_ = iterationCount;
}

LdpcCodeStructure::LdpcCodeStructure(SparseBitMatrix&& H, size_t iterationCount, DecoderType type) : CodeStructure(H.cols()-H.rows(), H.cols())
{
  computeGeneratorMatrix(std::move(H));
  
  decoderType_ = type;
  iterationCount_ = iterationCount;
}

void LdpcCodeStructure::syndrome(boost::container::vector<uint8_t>::const_iterator parity, boost::container::vector<uint8_t>::iterator syndrome) const
{
  for (auto parityEq = parityCheck().begin(); parityEq < parityCheck().end(); ++parityEq, ++syndrome) {
    for (auto parityBit = parityEq->begin(); parityBit < parityEq->end(); ++parityBit) {
      *syndrome ^= parity[*parityBit];
    }
  }
}

bool LdpcCodeStructure::syndromeCheck(boost::container::vector<uint8_t>::const_iterator parity) const
{
  for (auto parityEq = parityCheck().begin(); parityEq < parityCheck().end(); ++parityEq) {
    bool syndrome;
    for (auto parityBit = parityEq->begin(); parityBit < parityEq->end(); ++parityBit) {
      syndrome ^= parity[*parityBit];
    }
    if (syndrome != 0) {
      return false;
    }
  }
  return true;
}

void LdpcCodeStructure::encode(boost::container::vector<uint8_t>::const_iterator msg, boost::container::vector<uint8_t>::iterator parity) const
{
  std::copy(msg, msg + msgSize(), parity);
  std::fill(parity+msgSize(), parity+paritySize(), 0);
  parity += msgSize();
  auto parityIt = parity;
  for (auto row = DC_.begin(); row < DC_.end(); ++row, ++parityIt) {
    //auto msgIt = msg;
    for (auto elem = row->begin(); elem != row->end(); ++elem) {
      /*if (*elem) {
        *parityIt ^= *msgIt;
      }*/
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

void LdpcCodeStructure::computeGeneratorMatrix(SparseBitMatrix&& H)
{
  boost::container::vector<size_t> colSizes;
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
  /****/
  
  BitMatrix CDE = H({tSize, H.rows()}, {0, H.cols()});
  
  for (int64_t i = tSize; i >= 0; --i) {
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
      ++messageSize_;
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
  
  H_ = H;
  DC_ = CDE({0, CDE.cols()-msgSize()-tSize}, {0, msgSize()});
  A_ = H_({0, tSize}, {0, msgSize()});
  B_ = H_({0, tSize}, {msgSize(), msgSize()+DC_.rows()});
  T_ = H_({0, tSize}, {H.cols()-tSize, H.cols()}).transpose();
}
