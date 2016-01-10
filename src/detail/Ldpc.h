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

#ifndef FEC_DETAIL_LDPC_H
#define FEC_DETAIL_LDPC_H

#include <thread>
#include <random>
#include <chrono>
#include <algorithm>
#include <vector>
#include <unordered_map>

#include <boost/serialization/export.hpp>

#include "Codec.h"
#include "../BitMatrix.h"
#include "../Permutation.h"

namespace fec {
  
  namespace detail {
    
    namespace Ldpc {
      
      struct EncoderOptions
      {
        friend class Structure;
      public:
        EncoderOptions(const SparseBitMatrix<size_t>& checkMatrix) {checkMatrix_ = checkMatrix;}
        
        inline const SparseBitMatrix<size_t>& checkMatrix() const {return checkMatrix_;}
        
      private:
        SparseBitMatrix<size_t> checkMatrix_;
      };
      
      struct DecoderOptions {
        friend class Structure;
      public:
        DecoderOptions() = default;
        
        DecoderOptions& algorithm(DecoderAlgorithm algorithm) {algorithm_ = algorithm; return *this;}
        DecoderOptions& iterations(size_t n) {iterations_ = n; return *this;}
        DecoderOptions& scalingFactor(double factor) {scalingFactor_ = {std::make_pair(0, std::vector<double>({factor}))}; return *this;}
        DecoderOptions& scalingFactor(const std::unordered_map<size_t,std::vector<double>>& factor) {scalingFactor_ = factor; return *this;}
        
        DecoderAlgorithm algorithm() const {return algorithm_;}
        size_t iterations() const {return iterations_;}
        inline const std::unordered_map<size_t,std::vector<double>>& scalingFactor() const {return scalingFactor_;}
        
      private:
        DecoderAlgorithm algorithm_ = Approximate;
        size_t iterations_;
        std::unordered_map<size_t,std::vector<double>> scalingFactor_ = {std::make_pair(0, std::vector<double>({1.0}))};
      };
      
      struct PunctureOptions {
        friend class Structure;
      public:
        PunctureOptions() = default;
        
        PunctureOptions& mask(std::vector<bool> mask) {mask_ = mask; return *this;}
        PunctureOptions& systMask(std::vector<bool> mask) {systMask_ = mask; return *this;}
        
        std::vector<bool> systMask() const {return systMask_;}
        std::vector<bool> mask() const {return mask_;}
        
      private:
        std::vector<bool> systMask_;
        std::vector<bool> mask_;
      };
      
      /**
       *  This class represents a ldpc code structure.
       *  It provides a usefull interface to store and acces the structure information.
       */
      class Structure : public Codec::Structure {
        friend class ::boost::serialization::access;
      public:
        Structure() = default;
        Structure(const EncoderOptions&, const DecoderOptions&);
        Structure(const EncoderOptions&);
        ~Structure() = default;
        
        const char * get_key() const override;
        
        void setDecoderOptions(const DecoderOptions& decoder);
        DecoderOptions getDecoderOptions() const;
        Permutation puncturing(const PunctureOptions& options) const;
        
        size_t msgWidth() const override {return 1;} /**< Access the width of msg in each code bloc. */
        size_t systWidth() const override {return 1;} /**< Access the width of systematics in each code bloc. */
        size_t parityWidth() const override {return 1;} /**< Access the width of parities in each code bloc. */
        size_t stateWidth() const override {return 1;} /**< Access the width of state information in each code bloc. */
        
        size_t msgSize() const override {return msgSize_;} /**< Access the size of msg in each code bloc. */
        size_t systSize() const override {return H_.rows();} /**< Access the size of systematics in each code bloc. */
        size_t paritySize() const override {return H_.cols();} /**< Access the size of parities in each code bloc. */
        size_t stateSize() const override {return H_.size();} /**< Access the size of state information in each code bloc. */
        
        inline const SparseBitMatrix<size_t>& checks() const {return H_;}
        inline size_t iterations() const {return iterations_;}
        double scalingFactor(size_t i, size_t j) const; /**< Access the scalingFactor value used in decoder. */
        
        template <class InputIterator, class OutputIterator>
        void syndrome(InputIterator parity, OutputIterator syndrome) const;
        template <class InputIterator>
        bool check(InputIterator parity) const;
        template <class InputIterator, class OutputIterator>
        void encode(InputIterator msg, OutputIterator parity) const;
        
      private:
        template <typename Archive>
        void serialize(Archive & ar, const unsigned int version);
        
        void setEncoderOptions(const EncoderOptions& encoder);
        
        void computeGeneratorMatrix(SparseBitMatrix<size_t> H);
        std::vector<std::vector<double>> scalingMapToVector(const std::unordered_map<size_t,std::vector<double>>& map) const;
        std::unordered_map<size_t,std::vector<double>> scalingVectorToMap(const std::vector<std::vector<double>>& map) const;
        
        SparseBitMatrix<size_t> H_;
        SparseBitMatrix<size_t> DC_;
        SparseBitMatrix<size_t> T_;
        SparseBitMatrix<size_t> A_;
        SparseBitMatrix<size_t> B_;
        
        size_t msgSize_;
        
        size_t iterations_;
        std::vector<std::vector<double>> scalingFactor_;
      };
      
    }
  }
}



BOOST_CLASS_EXPORT_KEY(fec::detail::Ldpc::Structure);
BOOST_CLASS_TYPE_INFO(fec::detail::Ldpc::Structure,extended_type_info_no_rtti<fec::detail::Ldpc::Structure>);

/**
 *  Computes the syndrome given a sequence of parity bits.
 *  \param  parity  Input iterator pointing to the first element of the parity sequence.
 *  \param  syndrome[out] Output iterator pointing to the first
 *    element of the computed syndrome. The output needs to be allocated.
 */
template <class InputIterator, class OutputIterator>
void fec::detail::Ldpc::Structure::syndrome(InputIterator parity, OutputIterator syndrome) const
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
template <class InputIterator>
bool fec::detail::Ldpc::Structure::check(InputIterator parity) const
{
  for (auto parityEq = checks().begin(); parityEq < checks().end(); ++parityEq) {
    bool syndrome = false;
    for (auto parityBit = parityEq->begin(); parityBit < parityEq->end(); ++parityBit) {
      syndrome ^= bool(parity[parityBit->first]);
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
template <class InputIterator, class OutputIterator>
void fec::detail::Ldpc::Structure::encode(InputIterator msg, OutputIterator parity) const
{
  std::copy(msg, msg + msgSize(), parity);
  std::fill(parity+msgSize(), parity+checks().cols(), 0);
  parity += msgSize();
  auto parityIt = parity;
  for (auto row = DC_.begin(); row < DC_.end(); ++row, ++parityIt) {
    for (auto elem = row->begin(); elem != row->end(); ++elem) {
      *parityIt ^= msg[elem->first];
    }
  }
  for (auto row = B_.begin(); row < B_.end(); ++row, ++parityIt) {
    for (auto elem = row->begin(); elem < row->end(); ++elem) {
      *parityIt ^= parity[elem->first];
    }
  }
  parity += DC_.rows();
  parityIt = parity;
  for (auto row = A_.begin(); row < A_.end(); ++row, ++parityIt) {
    for (auto elem = row->begin(); elem < row->end(); ++elem) {
      *parityIt ^= msg[elem->first];
    }
  }
  parityIt = parity;
  for (auto row = T_.begin(); row < T_.end(); ++row, ++parityIt) {
    for (auto elem = row->begin()+1; elem < row->end(); ++elem) {
      parity[elem->first] ^= *parityIt;
    }
  }
}

template <typename Archive>
void fec::detail::Ldpc::Structure::serialize(Archive & ar, const unsigned int version) {
  using namespace boost::serialization;
  ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(Codec::Structure);
  ar & ::BOOST_SERIALIZATION_NVP(H_);
  ar & ::BOOST_SERIALIZATION_NVP(DC_);
  ar & ::BOOST_SERIALIZATION_NVP(T_);
  ar & ::BOOST_SERIALIZATION_NVP(A_);
  ar & ::BOOST_SERIALIZATION_NVP(B_);
  ar & ::BOOST_SERIALIZATION_NVP(msgSize_);
  ar & ::BOOST_SERIALIZATION_NVP(iterations_);
  ar & ::BOOST_SERIALIZATION_NVP(scalingFactor_);
}

#endif
