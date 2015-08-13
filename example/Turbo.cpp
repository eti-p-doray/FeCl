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
 
 Turbo code example
 ******************************************************************************/

#include <vector>
#include <random>
#include <memory>
#include <iostream>

#include "TurboCode/TurboCode.h"

#include "Operation.h"

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
  fec::TrellisStructure trellis({3}, {{05}});
  //! [Creating a trellis]
  
  uint64_t seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::srand ( unsigned (seed ) );
  std::vector<size_t> systIdx(256);
  std::vector<size_t> permIdx(256);
  for (size_t i = 0; i < systIdx.size(); ++i) {
    permIdx[i] = i;
    systIdx[i] = i;
  }
  std::random_shuffle (permIdx.begin(), permIdx.end());
  
  /*
   The trellis and interleaver indices are used to create a code structure.
   */
  fec::TurboCodeStructure structure({trellis, trellis}, {systIdx, permIdx}, {fec::ConvolutionalCodeStructure::Truncation,fec::ConvolutionalCodeStructure::Truncation});
  //! [Creating a Turbo code structure]
  
  
  /*
   A code is created and ready to operate
   */
  std::unique_ptr<fec::Code> code = fec::Code::create(structure);
  //! [Creating a Turbo code]
  
  std::cout << per(code, -2.0) << std::endl;
  
  return 0;
}