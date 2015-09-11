/*******************************************************************************
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 Declaration of MapDecoder class
 ******************************************************************************/

#ifndef MAP_DECODER_H
#define MAP_DECODER_H

#include <vector>
#include <memory>

#include "../Convolutional.h"

namespace fec {

  /**
   *  This class contains the abstract implementation of the map decoder.
   *  This algorithm is used for decoding with a-priori information
   *  in a ConvolutionalCode.
   *  The reason for this class is to offer an common interface of map decoders
   *  while allowing the compiler to inline implementation specific functions
   *  by using templates instead of polymorphism.
   */
class MapDecoder
{
public:  
  static std::unique_ptr<MapDecoder> create(const Convolutional::Structure&); /**< Creating function */
  virtual ~MapDecoder() = default; /**< Default destructor */
  
  void soDecodeBlocks(Code::InputIterator input, Code::OutputIterator output, size_t n);
  virtual void soDecodeBlock(Code::InputIterator input, Code::OutputIterator output) = 0;
  
protected:
  MapDecoder(const Convolutional::Structure&); /**< Constructor */
  
  inline const Convolutional::Structure& structure() const {return structure_;} /**< Access the code structure */

private:
  const Convolutional::Structure structure_;
};
  
}

#endif
