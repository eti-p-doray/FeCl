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
        EncoderOptions(const SparseBitMatrix& checkMatrix) {checkMatrix_ = checkMatrix;}
        
        SparseBitMatrix checkMatrix() const {return checkMatrix_;}
        
      private:
        SparseBitMatrix checkMatrix_;
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
        std::unordered_map<size_t,std::vector<double>> scalingFactor() const {return scalingFactor_;}
        
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
        virtual ~Structure() = default;
        
        virtual const char * get_key() const;
        
        void setDecoderOptions(const DecoderOptions& decoder);
        DecoderOptions getDecoderOptions() const;
        Permutation puncturing(const PunctureOptions& options) const;
        
        inline const SparseBitMatrix& checks() const {return H_;}
        inline size_t iterations() const {return iterations_;}
        double scalingFactor(size_t i, size_t j) const; /**< Access the scalingFactor value used in decoder. */
        
        void syndrome(std::vector<uint8_t>::const_iterator parity, std::vector<uint8_t>::iterator syndrome) const;
        virtual bool check(std::vector<BitField<size_t>>::const_iterator parity) const;
        virtual void encode(std::vector<BitField<size_t>>::const_iterator msg, std::vector<BitField<size_t>>::iterator parity) const;
        
      protected:
        void setEncoderOptions(const EncoderOptions& encoder);
        
      private:
        template <typename Archive>
        void serialize(Archive & ar, const unsigned int version);
        
        void computeGeneratorMatrix(SparseBitMatrix H);
        std::vector<std::vector<double>> scalingMapToVector(const std::unordered_map<size_t,std::vector<double>>& map) const;
        std::unordered_map<size_t,std::vector<double>> scalingVectorToMap(const std::vector<std::vector<double>>& map) const;
        
        SparseBitMatrix H_;
        SparseBitMatrix DC_;
        SparseBitMatrix T_;
        SparseBitMatrix A_;
        SparseBitMatrix B_;
        
        size_t iterations_;
        std::vector<std::vector<double>> scalingFactor_;
      };
    }
  }
}



BOOST_CLASS_EXPORT_KEY(fec::detail::Ldpc::Structure);
BOOST_CLASS_TYPE_INFO(fec::detail::Ldpc::Structure,extended_type_info_no_rtti<fec::detail::Ldpc::Structure>);


template <typename Archive>
void fec::detail::Ldpc::Structure::serialize(Archive & ar, const unsigned int version) {
  using namespace boost::serialization;
  ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(Codec::Structure);
  ar & ::BOOST_SERIALIZATION_NVP(H_);
  ar & ::BOOST_SERIALIZATION_NVP(DC_);
  ar & ::BOOST_SERIALIZATION_NVP(T_);
  ar & ::BOOST_SERIALIZATION_NVP(A_);
  ar & ::BOOST_SERIALIZATION_NVP(B_);
  ar & ::BOOST_SERIALIZATION_NVP(iterations_);
  ar & ::BOOST_SERIALIZATION_NVP(scalingFactor_);
}

#endif
