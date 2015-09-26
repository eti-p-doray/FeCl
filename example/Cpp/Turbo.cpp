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

#include "Turbo/Turbo.h"

#include "operations.h"

int main( int argc, char* argv[] )
{
  //! [Creating a Turbo code]
  //! [Creating a Turbo code structure]
  //! [Creating a trellis]
  /*
   We are creating a trellis structure with 1 input bit.
   The constraint length is 3, which means there are 2 registers associated
   with the input bit.
   There is one output bits, with generator 5 (in octal) associated
   with the input bit.
   */
  fec::Trellis trellis({4}, {{017}}, {015});
  //! [Creating a trellis]
  
  uint64_t seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::srand ( unsigned (seed ) );
  std::vector<size_t> systIdx(2048);
  std::vector<size_t> permIdx(2048);
  for (size_t i = 0; i < systIdx.size(); ++i) {
    permIdx[i] = i;
    systIdx[i] = i;
  }
  std::random_shuffle (permIdx.begin(), permIdx.end());
  
  /*
   The trellis and interleaver indices are used to create a code structure.
   */
  auto encoder = fec::Turbo::EncoderOptions({trellis, trellis}, {systIdx, permIdx}).termination(fec::Convolutional::Truncate).bitOrdering(fec::Turbo::Group);
  auto decoder = fec::Turbo::DecoderOptions().algorithm(fec::Codec::Exact).iterations(10).scheduling(fec::Turbo::Serial);
  fec::Turbo::Structure structure(encoder, decoder);
  //! [Creating a Turbo code structure]
  
  
  /*
   A code is created and ready to operate
   */
  std::unique_ptr<fec::Codec> code = fec::Codec::create(structure, 1);
  //! [Creating a Turbo code]
  
  std::cout << per(code, -4) << std::endl;
  
  /*decoder.decoderType(fec::Codec::Table);
  code = fec::Codec::create(fec::Turbo::Structure(encoder, decoder), 1);
  
  std::cout << per(code, 4.0) << std::endl;
  
  decoder.decoderType(fec::Codec::Approximate);
  code = fec::Codec::create(fec::Turbo::Structure(encoder, decoder), 1);
  
  std::cout << per(code, 4.0) << std::endl;*/
  
  return 0;
}