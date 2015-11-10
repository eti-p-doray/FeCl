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

#include "Ldpc.h"

#include "operations.h"

int main( int argc, char* argv[] )
{
  //! [Creating an ldpc code]
  //! [Creating an ldpc code structure]
  //! [Creating an ldpcMatrix]
  /*
   We are creating an ldpc matrix
   */
  auto checkMatrix = fec::Ldpc::DvbS2::matrix(64800, 1.0/2.0);
  //! [Creating an ldpcMatrix]
  
  /*
   The matrix is used to create a code structure.
   */
  auto options = fec::Ldpc::Options(checkMatrix);
  options.iterations(20).algorithm(fec::Exact);
  //! [Creating a Turbo code structure]
  
  /*
   A code is created and ready to operate
   */
  std::unique_ptr<fec::Codec> codec(new fec::Ldpc(options));
  //! [Creating an ldpc code]
  
  std::cout << per(codec, 1.0) << std::endl;
  
  return 0;
}