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

#ifndef FEC_TURBO_H
#define FEC_TURBO_H

#include <boost/serialization/export.hpp>

#include "detail/Turbo.h"
#include "Codec.h"
#include "Convolutional.h"
#include "Permutation.h"

namespace fec {
  
  /**
   *  This class represents a turbo encode / decoder.
   *  It offers methods to encode and to decode data given a Turbo::Structure.
   *
   *  The structure of the parity bits generated by a Turbo object is as follow
   *
   *    | syst | systTail | convOutput1 | tailOutpu1 | convOutput2 | tailOutpu2 | ... |
   *
   *  where syst are the systematic bits, systTail are the tail bit
   *  added to the msg for termination of the constituents 1, 2, ..., respectively,
   *  convOutputX and tailOutputX are the output parity of the  msg and the tail
   *  generated by the constituent convolutional code X.
   *
   *  The structure of the extrinsic information
   *
   *    | msg1 | systTail1 | msg2 | systTail2 | ... |
   *
   *  where msgX are the extrinsic msg L-values generated by the constituent code X
   *  and systTailX are the tail bit added to the msg
   *  for termination of the constituent X.
   */
  class Turbo : public Codec
  {
    friend class boost::serialization::access;
  public:
    
    using Scheduling = detail::Turbo::Scheduling;
    using BitOrdering = detail::Turbo::BitOrdering;
    
    using EncoderOptions = detail::Turbo::EncoderOptions;
    using DecoderOptions = detail::Turbo::DecoderOptions;
    using PunctureOptions = detail::Turbo::PunctureOptions;
    
    /**
     *  This class contains functions associated with the 3GPP LTE standard.
     */
    struct Lte3Gpp {
    public:
      /*
       *  Access an interleaver defined in the standard.
       *  \param  length  Lenght of the interleaver to be created.
       *  \return A Permutation of the specified length as defined in the standard.
       */
      static Permutation interleaver(size_t length);
      static Trellis trellis() {return Trellis({4}, {{13}}, {15});}
      
    private:
      static const std::array<size_t, 188> length_;
      static const std::array<double, 188> rate_;
      static const std::vector<std::vector<size_t>> parameter_;
    };
    
    Turbo() = default;
    Turbo(const EncoderOptions& encoder, const DecoderOptions& decoder, int workGroupSize = 8);
    Turbo(const EncoderOptions& encoder, int workGroupSize = 8);
    Turbo(const Turbo& other) {*this = other;}
    virtual ~Turbo() = default;
    Turbo& operator=(const Turbo& other) {Codec::operator=(other); structure_ = std::unique_ptr<detail::Turbo::Structure>(new detail::Turbo::Structure(other.structure())); return *this;}
    
    virtual const char * get_key() const;
    
    void setDecoderOptions(const DecoderOptions& decoder) {structure().setDecoderOptions(decoder);}
    DecoderOptions getDecoderOptions() const {return structure().getDecoderOptions();}
    
    Permutation puncturing(const PunctureOptions& options) {return structure().puncturing(options);}
    
  protected:
    Turbo(std::unique_ptr<detail::Turbo::Structure>&& structure, int workGroupSize = 4) : Codec(std::move(structure), workGroupSize) {}
    
    inline const detail::Turbo::Structure& structure() const {return dynamic_cast<const detail::Turbo::Structure&>(Codec::structure());}
    inline detail::Turbo::Structure& structure() {return dynamic_cast<detail::Turbo::Structure&>(Codec::structure());}
    
    virtual void decodeBlocks(detail::Codec::const_iterator<double> first, detail::Codec::const_iterator<double> last, detail::Codec::iterator<BitField<size_t>> output) const;
    virtual void soDecodeBlocks(detail::Codec::const_iterator<double> first, detail::Codec::const_iterator<double> last, detail::Codec::iterator<double> output) const;
    
  private:
    Turbo(const detail::Turbo::Structure& structure, int workGroupSize = 8);
    
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version);
  };
  
}

BOOST_CLASS_EXPORT_KEY(fec::Turbo);
BOOST_CLASS_TYPE_INFO(fec::Turbo,extended_type_info_no_rtti<fec::Turbo>);


template <typename Archive>
void fec::Turbo::serialize(Archive & ar, const unsigned int version) {
  using namespace boost::serialization;
  ar.template register_type<detail::Turbo::Structure>();
  ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(Codec);
}

#endif
