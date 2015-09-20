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
 
 Ldpc code example
 ******************************************************************************/

#include <vector>
#include <random>
#include <memory>
#include <iostream>

#include "Ldpc/Ldpc.h"

#include "operations.h"

int main( int argc, char* argv[] )
{
  //! [Creating an ldpc code]
  //! [Creating an ldpc code structure]
  //! [Creating an ldpcMatrix]
  /*
   We are creating an ldpc matrix
   */
  auto checkMatrix = fec::Ldpc::gallagerConstruction(1024, 8, 16);
  //! [Creating an ldpcMatrix]
  
  
  auto encoder = fec::Ldpc::EncoderOptions(checkMatrix);
  auto decoder = fec::Ldpc::DecoderOptions().iterations(20).algorithm(fec::Codec::Exact);
  
  /*
   The matrix is used to create a code structure.
   */
  fec::Ldpc::Structure structure(encoder, decoder);
  //! [Creating a Turbo code structure]
  
  /*
   A code is created and ready to operate
   */
  std::unique_ptr<fec::Codec> codec = fec::Codec::create(structure, 1);
  //! [Creating an ldpc code]
  
  std::cout << per(codec, 1.0) << std::endl;
  
  return 0;
}