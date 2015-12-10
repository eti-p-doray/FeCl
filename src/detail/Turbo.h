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

#ifndef FEC_DETAIL_TURBO_H
#define FEC_DETAIL_TURBO_H

#include <boost/serialization/export.hpp>

#include "Codec.h"
#include "Convolutional.h"
#include "../Permutation.h"
#include "../SchedulingType.h"

namespace fec {
  
  namespace detail {
    
    namespace Turbo {
      
      struct Stage {
        std::vector<size_t> activation;
        std::vector<std::vector<size_t>> transfer;
        template <typename Archive>
        void serialize(Archive & ar, const unsigned int version) {
          using namespace boost::serialization;
          ar & ::BOOST_SERIALIZATION_NVP(activation);
          ar & ::BOOST_SERIALIZATION_NVP(transfer);
        }
      };
      using Scheduling = std::vector<Stage>;
      
      /**
       *  Ordering of parity bit in Turbo.
       *  This defines the ordering of parity bits that are output from a Turbo permutation of a PuncturedTurbo Codec.
       */
      enum BitOrdering {
        Alternate,/**< Systematic and parity bits are alternated */
        Group,/**< Systematic bits are group together and parity bits from each constituents are grouped together. */
      };
      
      struct EncoderOptions {
        friend class Structure;
      public:
        EncoderOptions(const std::vector<Trellis>& trellis, const std::vector<Permutation>& interleaver) {trellis_ = trellis; interleaver_ = interleaver;}
        EncoderOptions(const Trellis& trellis, const std::vector<Permutation>& interleaver) {trellis_ = {trellis}; interleaver_ = interleaver;}
        EncoderOptions& interleaver(const std::vector<Permutation>& interleaver) {interleaver_ = interleaver; return *this;}
        EncoderOptions& termination(Trellis::Termination type) {termination_ = {type}; return *this;}
        EncoderOptions& termination(std::vector<Trellis::Termination> type) {termination_ = type; return *this;}
        
      private:
        std::vector<Trellis> trellis_;
        std::vector<Permutation> interleaver_;
        std::vector<Trellis::Termination> termination_ = std::vector<Trellis::Termination>(1,Trellis::Tail);
      };
      
      /**
       *  This class gathers options affecting decoder in Turbo Codec.
       */
      struct DecoderOptions {
        friend class Structure;
      public:
        DecoderOptions() = default;
        
        DecoderOptions& iterations(size_t count) {iterations_ = count; return *this;}
        DecoderOptions& scheduling(SchedulingType type) {schedulingType_ = type; return *this;}
        DecoderOptions& scheduling(const Scheduling& sched) {schedulingType_ = Custom; scheduling_ = sched; return *this;}
        DecoderOptions& algorithm(DecoderAlgorithm algorithm) {algorithm_ = algorithm; return *this;}
        DecoderOptions& scalingFactor(double factor) {scalingFactor_ = {{factor}}; return *this;}
        DecoderOptions& scalingFactor(const std::vector<std::vector<double>>& factor) {scalingFactor_ = factor; return *this;}
        
        size_t iterations() const {return iterations_;}
        SchedulingType schedulingType() const {return schedulingType_;}
        Scheduling scheduling() const {return scheduling_;}
        DecoderAlgorithm algorithm() const {return algorithm_;}
        std::vector<std::vector<double>> scalingFactor() const {return scalingFactor_;}
        
      private:
        size_t iterations_ = 6;
        SchedulingType schedulingType_ = Serial;
        Scheduling scheduling_;
        DecoderAlgorithm algorithm_ = Linear;
        std::vector<std::vector<double>> scalingFactor_ = {{1.0}};
      };
      
      struct PunctureOptions {
        friend class Structure;
      public:
        PunctureOptions() = default;
        
        PunctureOptions& mask(std::vector<std::vector<bool>> mask) {mask_ = mask; return *this;}
        PunctureOptions& tailMask(std::vector<std::vector<bool>> mask) {tailMask_ = mask; return *this;}
        PunctureOptions& bitOrdering(BitOrdering ordering) {bitOrdering_ = ordering; return *this;}
        
      private:
        std::vector<std::vector<bool>> mask_;
        std::vector<std::vector<bool>> tailMask_;
        BitOrdering bitOrdering_ = Alternate;
      };
      
      /**
       *  This class represents a convolutional code structure.
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
        
        size_t msgWidth() const override {return 1;} /**< Access the width of msg in each code bloc. */
        size_t systWidth() const override {return 1;} /**< Access the width of systematics in each code bloc. */
        size_t parityWidth() const override {return 1;} /**< Access the width of parities in each code bloc. */
        size_t stateWidth() const override {return 1;} /**< Access the width of state information in each code bloc. */
        
        size_t msgSize() const override {return msgSize_;} /**< Access the size of msg in each code bloc. */
        size_t systSize() const override {return systSize_;} /**< Access the size of systematics in each code bloc. */
        size_t paritySize() const override {return paritySize_;} /**< Access the size of parities in each code bloc. */
        size_t stateSize() const override {return stateSize_;} /**< Access the size of state information in each code bloc. */
        
        /**
         *  Access the size of added msg bit for the trellis termination.
         *  This is zero in the cas of trunction.
         *  \return Tail size
         */
        inline size_t tailSize() const {return tailSize_;}
        inline size_t constituentCount() const {return constituents_.size();}
        inline const std::vector<Convolutional::Structure>& constituents() const {return constituents_;}
        inline const std::vector<Permutation>& interleavers() const {return interleaver_;}
        inline const Convolutional::Structure& constituent(size_t i) const {return constituents_[i];}
        inline const Permutation& interleaver(size_t i) const {return interleaver_[i];}
        
        inline size_t iterations() const {return iterations_;}
        inline SchedulingType schedulingType() const {return schedulingType_;}
        inline const Scheduling& scheduling() const {return scheduling_;}
        
        double scalingFactor(size_t i, size_t j) const; /**< Access the scalingFactor value used in decoder. */
        
        bool check(std::vector<BitField<size_t>>::const_iterator parity) const override;
        void encode(std::vector<BitField<size_t>>::const_iterator msg, std::vector<BitField<size_t>>::iterator parity) const override;
        
        Permutation puncturing(const PunctureOptions& options) const;
        
      protected:
        void setEncoderOptions(const EncoderOptions& encoder);
        
      private:
        template <typename Archive>
        void serialize(Archive & ar, const unsigned int version);
        
        size_t msgSize_;
        size_t systSize_;
        size_t paritySize_;
        size_t stateSize_;
        
        std::vector<Convolutional::Structure> constituents_;
        std::vector<Permutation> interleaver_;
        size_t tailSize_;
        size_t iterations_;
        SchedulingType schedulingType_;
        Scheduling scheduling_;
        std::vector<std::vector<double>> scalingFactor_;
      };
      
    }
  }
}



BOOST_CLASS_EXPORT_KEY(fec::detail::Turbo::Structure);
BOOST_CLASS_TYPE_INFO(fec::detail::Turbo::Structure,extended_type_info_no_rtti<fec::detail::Turbo::Structure>);


template <typename Archive>
void fec::detail::Turbo::Structure::serialize(Archive & ar, const unsigned int version) {
  using namespace boost::serialization;
  ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(Codec::Structure);
  ar & ::BOOST_SERIALIZATION_NVP(msgSize_);
  ar & ::BOOST_SERIALIZATION_NVP(systSize_);
  ar & ::BOOST_SERIALIZATION_NVP(paritySize_);
  ar & ::BOOST_SERIALIZATION_NVP(stateSize_);
  
  ar & ::BOOST_SERIALIZATION_NVP(constituents_);
  ar & ::BOOST_SERIALIZATION_NVP(interleaver_);
  ar & ::BOOST_SERIALIZATION_NVP(tailSize_);
  ar & ::BOOST_SERIALIZATION_NVP(iterations_);
  ar & ::BOOST_SERIALIZATION_NVP(schedulingType_);
  ar & ::BOOST_SERIALIZATION_NVP(scheduling_);
  ar & ::BOOST_SERIALIZATION_NVP(scalingFactor_);
}

#endif
