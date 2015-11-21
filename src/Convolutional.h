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

#ifndef FEC_CONVOLUTIONAL_H
#define FEC_CONVOLUTIONAL_H

#include "detail/Convolutional.h"
#include "Codec.h"
#include "Trellis.h"
#include "Permutation.h"

namespace fec {
  
  /**
   *  This class represents a convolutional encode / decoder.
   *  It offers methods encode and to decode data given a Structure.
   *
   *  The structure of the parity bits generated by a ConvolutionalCodec object is as follows
   *
   *    | parity1 | parity2 | ... | tail1 | tail2 ... |
   *
   *  where parityX is the output symbol sequence of the branch used at stage X
   *  in the trellis, tailX is are the output symbol sequence of the tail branch
   *  at stage X.
   *
   *  The structure of the extrinsic information
   *
   *    | msg | systTail |
   *
   *  where msg are the extrinsic msg L-values generated by the map decoder
   *  and systTail are the tail bit added to the message for trellis termination.
   */
  class Convolutional : public Codec
  {
    friend class boost::serialization::access;
  public:
    
    using EncoderOptions = detail::Convolutional::EncoderOptions;
    using DecoderOptions = detail::Convolutional::DecoderOptions;
    using PunctureOptions = detail::Convolutional::PunctureOptions;
    
    Convolutional() = default;
    Convolutional(const detail::Convolutional::Structure& structure, int workGroupSize = 8);
    Convolutional(const EncoderOptions& encoder, const DecoderOptions& decoder, int workGroupSize = 8);
    Convolutional(const EncoderOptions& encoder, int workGroupSize = 8);
    Convolutional(const Convolutional& other) {*this = other;}
    virtual ~Convolutional() = default;
    Convolutional& operator=(const Convolutional& other) {Codec::operator=(other); structure_ = std::unique_ptr<detail::Convolutional::Structure>(new detail::Convolutional::Structure(other.structure())); return *this;}
    
    virtual const char * get_key() const;
    
    void setDecoderOptions(const DecoderOptions& decoder) {structure().setDecoderOptions(decoder);}
    DecoderOptions getDecoderOptions() const {return structure().getDecoderOptions();}
    
    Permutation puncturing(const PunctureOptions& options) {return structure().puncturing(options);}
    
  protected:
    Convolutional(std::unique_ptr<detail::Convolutional::Structure>&& structure, int workGroupSize = 4) : Codec(std::move(structure), workGroupSize) {}
    
    inline const detail::Convolutional::Structure& structure() const {return dynamic_cast<const detail::Convolutional::Structure&>(Codec::structure());}
    inline detail::Convolutional::Structure& structure() {return dynamic_cast<detail::Convolutional::Structure&>(Codec::structure());}
    
    virtual void decodeBlocks(std::vector<double>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg, size_t n) const;
    virtual void soDecodeBlocks(detail::Codec::InputIterator input, detail::Codec::OutputIterator output, size_t n) const;
    
  private:
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version);
  };
  
}

BOOST_CLASS_TYPE_INFO(fec::Convolutional,extended_type_info_no_rtti<fec::Convolutional>);
BOOST_CLASS_EXPORT_KEY(fec::Convolutional);


template <typename Archive>
void fec::Convolutional::serialize(Archive & ar, const unsigned int version) {
  using namespace boost::serialization;
  ar.template register_type<detail::Convolutional::Structure>();
  ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(Codec);
}

#endif
