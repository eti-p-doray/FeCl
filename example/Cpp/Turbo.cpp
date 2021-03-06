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

#include <vector>
#include <random>
#include <memory>
#include <iostream>

#include "Turbo.h"
#include "Modulation.h"

#include "operations.h"

int main( int argc, char* argv[] )
{
  //! [Creating a Turbo code]
  //! [Creating a Turbo code structure]
  //! [Creating a trellis]
  /*
   We are creating a trellis structure with 1 input bit.
   The constraint length is 4, which means there are 3 registers associated
   with the input bit.
   There is one output bits, with generator 17 (in octal) associated
   with the input bit.
   */
  fec::Trellis trellis({4}, {{017}}, {015});
  //! [Creating a trellis]
  
  uint64_t seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::srand ( unsigned (seed ) );
  std::vector<size_t> permIdx(2048);
  for (size_t i = 0; i < permIdx.size(); ++i) {
    permIdx[i] = i;
  }
  std::random_shuffle (permIdx.begin(), permIdx.end());
  
  /*
   The trellis and interleaver indices are used to create a code structure.
   */
  auto encOptions = fec::Turbo::EncoderOptions({trellis, trellis}, {{}, permIdx}).termination(fec::Trellis::Truncate);
  auto decOptions = fec::Turbo::DecoderOptions({}).algorithm(fec::Approximate).iterations(10).scheduling({ {{0, 1}, {{1}, {0}}} });
  //! [Creating a Turbo code structure]
  
  
  /*
   A codec is created and ready to operate
   */
  std::unique_ptr<fec::Codec> codec(new fec::Turbo(encOptions, decOptions));
  //! [Creating a Turbo code]
  
  std::cout << per(codec, -1) << std::endl;
  
  return 0;
}