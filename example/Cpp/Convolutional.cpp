/*******************************************************************************
 This file is part of C3sar.
 
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 Copyright (c) 2015, Leszek Szczecinski, INRS
 All rights reserved.
 
 C3sar is free software: you can redistribute it and/or modify
 it under the terms of the Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 C3sar is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the Lesser General Public License
 along with C3rel.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include <vector>
#include <random>
#include <memory>
#include <iostream>

#include "Convolutional/Convolutional.h"

#include "operations.h"

int main( int argc, char* argv[] )
{
  //! [Creating a Convolutional code]
  //! [Creating a Convolutional code structure]
  //! [Creating a trellis]
  /*
   We are creating a trellis structure with 1 input bit.
   The constraint length is 3, which means there are 2 registers associated
   with the input bit.
   There are two output bits, the first one with generator 4 (in octal) associated
   with the input bit.
   */
  //fec::Trellis trellis({3, 3}, {{05, 03, 0}, {0, 03, 07}}, {07, 05});
  fec::Trellis trellis({4}, {{013, 017}}, {015});
  //! [Creating a trellis]
  
  /*
   The trellis is used to create a code structure.
   We specify that one bloc will conatins 256 branches before being terminated.
   */
  auto encoder = fec::Convolutional::EncoderOptions(trellis, 1024).termination(fec::Convolutional::Tail);
  auto decoder = fec::Convolutional::DecoderOptions().algorithm(fec::Codec::Approximate);
  //! [Creating a Convolutional code structure]
  
  /*
   A code is created and ready to operate
   */
  std::unique_ptr<fec::Codec> codec(new fec::Convolutional(encoder, decoder));
  //! [Creating a Convolutional code]
  
  std::cout << per(codec, 3.0) << std::endl;
  
  //std::cout << trellis << std::endl;
  
  return 0;
}