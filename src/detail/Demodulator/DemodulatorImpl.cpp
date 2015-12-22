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

#include "DemodulatorImpl.h"

using namespace fec;
using namespace fec::detail;

/**
 *  Constructor.
 *  Allocates metric buffers based on the given code structure.
 *  \param  codeStructure Convolutional code structure describing the code
 */
template <class LlrMetrics, template <class> class LogSumAlg>
DemodulatorImpl<LlrMetrics, LogSumAlg>::DemodulatorImpl(const Modulation::Structure& structure) :
Demodulator(structure)
{
  distance_.resize(this->structure().constellation().size() / this->structure().dimension());
}

/**
 *  Implementation of Codec#softOutDecodeNBloc.
 */
template <class LlrMetrics, template <class> class LogSumAlg>
void DemodulatorImpl<LlrMetrics, LogSumAlg>::soDemodulateBlocks(Modulation::const_iterator<double> inputf, Modulation::const_iterator<double> inputl, double k, std::vector<double>::iterator word)
{
  while (inputf != inputl) {
    soDemodulateBlock(inputf++,k,word);
    word += structure().wordSize();
  }
}

/**
 *  Decodes one blocs of information bits.
 *  A posteriori information about the msg is output instead of the decoded bit sequence.
 *  \param  parityIn  Input iterator pointing to the first element
 *    in the parity L-value sequence
 *  \param  messageOut[out] Output iterator pointing to the first element
 *    in the a posteriori information L-value sequence.
 *    Output needs to be pre-allocated.
 */
template <class LlrMetrics, template <class> class LogSumAlg>
void DemodulatorImpl<LlrMetrics, LogSumAlg>::soDemodulateBlock(Modulation::const_iterator<double> input, double k, std::vector<double>::iterator word)
{
  for (size_t i = 0; i < structure().length(); ++i) {
    distanceUpdate(input);
    aPosterioriUpdate(input, k, word);
    input.at(Modulation::Symbol) += structure().symbolWidth();
    input.at(Modulation::Word) += structure().size();
    word += structure().size();
  }
}

template <class LlrMetrics, template <class> class LogSumAlg>
void DemodulatorImpl<LlrMetrics, LogSumAlg>::distanceUpdate(Modulation::const_iterator<double> input)
{
  auto symbol = input.at(Modulation::Symbol);
  auto word = input.at(Modulation::Word);
  auto distance = distance_.begin();
  for (auto j = structure().constellation().begin(); j != structure().constellation().end(); j += structure().dimension(), ++distance) {
    *distance = -sqDistance<LlrMetrics>(symbol, j, structure().dimension());
  }
  if (input.count(Modulation::Word)) {
    for (BitField<size_t> j = 0; j < structure().constellation().size(); j += structure().dimension(), ++distance) {
      *distance += accumulate(j, word, structure().dimension());
    }
  }
}

template <class LlrMetrics, template <class> class LogSumAlg>
void DemodulatorImpl<LlrMetrics, LogSumAlg>::aPosterioriUpdate(Modulation::const_iterator<double> input, double k, std::vector<double>::iterator output)
{
  auto wordIn = input.at(Modulation::Word);
  auto wordOut = output;

  for (size_t j = 0; j < structure().size(); ++j) {
    typename LlrMetrics::Type tmp = wordUpdateImpl(j);

    if (input.count(Modulation::Word)) {
      wordOut[j] = k * structure().scalingFactor() * (tmp - wordIn[j]);
    }
    else {
      wordOut[j] = k * structure().scalingFactor() * (tmp);
    }
  }
}

template <class LlrMetrics, template <class> class LogSumAlg>
template <class U, typename std::enable_if<U::value>::type*>
typename LlrMetrics::Type DemodulatorImpl<LlrMetrics, LogSumAlg>::wordUpdateImpl(size_t j)
{
  typename LlrMetrics::Type metric[2] = {logSum_.prior(-llrMetrics_.max()), logSum_.prior(-llrMetrics_.max())};
  for (BitField<size_t> input = 0; input < distance_.size(); ++input) {
    metric[input.test(j)] = logSum_.sum(metric[input.test(j)], distance_[input]);
  }
  return logSum_.post(metric[1]) - logSum_.post(metric[0]);
}

template <class LlrMetrics, template <class> class LogSumAlg>
template <class U, typename std::enable_if<!U::value>::type*>
typename LlrMetrics::Type DemodulatorImpl<LlrMetrics, LogSumAlg>::wordUpdateImpl(size_t j)
{
  typename LlrMetrics::Type max[2] = {typename LlrMetrics::Type(-llrMetrics_.max()), typename LlrMetrics::Type(-llrMetrics_.max())};
  typename LlrMetrics::Type metric[2] = {
    typename LlrMetrics::Type(0),
    typename LlrMetrics::Type(0)};
  
  for (BitField<size_t> input = 0; input < distance_.size(); ++input) {
    max[input.test(j)] = logSum_.max(distance_[input], max[input.test(j)]);
  }
  for (BitField<size_t> input = 0; input < distance_.size(); ++input) {
    metric[input.test(j)] = logSum_.sum(logSum_.prior(distance_[input], max[input.test(j)]), metric[input.test(j)]);
  }
  return logSum_.post(metric[1], max[1]) - logSum_.post(metric[0], max[0]);
}

//Explicit instantiation
template class fec::detail::DemodulatorImpl<FloatLlrMetrics, LogSum>;
template class fec::detail::DemodulatorImpl<FloatLlrMetrics, MaxLogSum>;
template class fec::detail::DemodulatorImpl<FloatLlrMetrics, LinearLogSum>;
