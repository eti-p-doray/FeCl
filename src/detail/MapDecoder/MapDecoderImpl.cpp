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

#include "MapDecoderImpl.h"

using namespace fec;
using namespace fec::detail;

/**
 *  Constructor.
 *  Allocates metric buffers based on the given code structure.
 *  \param  codeStructure Convolutional code structure describing the code
 */
template <class LlrMetrics, template <class> class LogSumAlg>
MapDecoderImpl<LlrMetrics, LogSumAlg>::MapDecoderImpl(const Convolutional::Structure& structure) :
MapDecoder(structure)
{
  branch_.resize((this->structure().length()+this->structure().tailLength())*this->structure().trellis().inputCount()*this->structure().trellis().stateCount());
  forward_.resize((this->structure().length()+this->structure().tailLength())*this->structure().trellis().stateCount());
  backward_.resize((this->structure().length()+this->structure().tailLength())*this->structure().trellis().stateCount());
  
  buffer_.resize(std::max(this->structure().trellis().outputCount(), this->structure().trellis().inputCount()));
  if (!LogSumAlg<LlrMetrics>::isRecursive::value) {
    buffer_.resize(this->structure().trellis().stateCount()*(this->structure().trellis().inputCount()+1));
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
void MapDecoderImpl<LlrMetrics, LogSumAlg>::soDecodeBlock(Codec::const_iterator<double> input, Codec::iterator<double> output)
{
  branchUpdate(input);
  forwardUpdate();
  backwardUpdate();
  aPosterioriUpdate(input, output);
}

template <class LlrMetrics, template <class> class LogSumAlg>
void MapDecoderImpl<LlrMetrics, LogSumAlg>::branchUpdate(Codec::const_iterator<double> input)
{
  auto parity = input.at(Codec::Parity);
  auto syst = input.at(Codec::Syst);
  auto branch = branch_.begin();
  for (size_t i = 0; i < structure().length() + structure().tailLength(); ++i) {
    for (BitField<size_t> j = 0; j < structure().trellis().outputCount(); ++j) {
      buffer_[j] = correlation<LlrMetrics>(j, parity, structure().trellis().outputWidth());
    }
    auto branchTmp = branch;
    for (auto output = structure().trellis().beginOutput(); output < structure().trellis().endOutput();) {
      for (size_t k = 0; k < structure().trellis().inputCount(); ++k) {
        branch[k] = buffer_[size_t(output[k])];
      }
      output += structure().trellis().inputCount();
      branch += structure().trellis().inputCount();
    }
    
    if (input.count(Codec::Syst)) {
      branch = branchTmp;
      for (BitField<size_t> j = 0; j < structure().trellis().inputCount(); ++j) {
        buffer_[j] = correlation<LlrMetrics>(j, syst, structure().trellis().inputWidth());
      }
      for (size_t j = 0; j < structure().trellis().stateCount(); ++j) {
        for (size_t k = 0; k < structure().trellis().inputCount(); ++k) {
          branch[k] += buffer_[k];
        }
        branch += structure().trellis().inputCount();
      }
    }
    parity += structure().trellis().outputWidth();
    syst += structure().trellis().inputWidth();
  }
}

template <class LlrMetrics, template <class> class LogSumAlg>
void MapDecoderImpl<LlrMetrics, LogSumAlg>::forwardUpdate()
{
  auto forward = forward_.begin();
  auto branch = branch_.cbegin();
  
  *forward = 0;
  std::fill(forward+1, forward + structure().trellis().stateCount(), -llrMetrics_.max());
  
  for (; forward < forward_.end() - structure().trellis().stateCount();) {
    forwardUpdateImpl(forward, branch);
    forward += structure().trellis().stateCount();
    branch += structure().trellis().tableSize();
    typename LlrMetrics::Type max = -llrMetrics_.max();
    for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
      forward[j] = logSum_.post(forward[j]);
      max = std::max(forward[j], max);
    }
    for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
      forward[j] -= max;
    }
  }
}

template <class LlrMetrics, template <class> class LogSumAlg>
void MapDecoderImpl<LlrMetrics, LogSumAlg>::backwardUpdate()
{
  auto backward = backward_.end()-structure().trellis().stateCount();
  switch (structure().termination()) {
    case Trellis::Tail:
      *backward = 0;
      std::fill(backward+1, backward + structure().trellis().stateCount(), -llrMetrics_.max());
      break;
      
    default:
    case Trellis::Truncate:
      std::fill(backward, backward + structure().trellis().stateCount(), 0.0);
      break;
  }
  backward -= structure().trellis().stateCount();
  auto branch = branch_.cend()-structure().trellis().tableSize();
  
  for ( ; backward >= backward_.begin();) {
    backwardUpdateImpl(backward, branch);
    typename LlrMetrics::Type max = -llrMetrics_.max();
    for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
      backward[j] = logSum_.post(backward[j]);
      max = std::max(backward[j], max);
    }
    for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
      backward[j] -= max;
    }
    backward -= structure().trellis().stateCount();
    branch -= structure().trellis().tableSize();
  }
}

template <class LlrMetrics, template <class> class LogSumAlg>
void MapDecoderImpl<LlrMetrics, LogSumAlg>::aPosterioriUpdate(Codec::const_iterator<double> input, Codec::iterator<double> output)
{
  auto systOut = output.at(Codec::Syst);
  auto systIn = input.at(Codec::Syst);
  auto parityOut = output.at(Codec::Parity);
  auto parityIn = input.at(Codec::Parity);
  auto msgOut = output.at(Codec::Msg);

  for (size_t i = 0; i < structure().length() + structure().tailLength(); ++i) {
    auto branch = branch_.begin() + i * structure().trellis().tableSize();
    auto forward = forward_.cbegin() + i * structure().trellis().stateCount();
    auto backward = backward_.cbegin() + i * structure().trellis().stateCount();
    
    for (auto state = structure().trellis().beginState(); state < structure().trellis().endState(); ) {
      for (BitField<size_t> input = 0; input < structure().trellis().inputCount(); ++input) {
        branch[input] = logSum_.prior(branch[input] + *forward + backward[size_t(state[input])]);
      }
      state += structure().trellis().inputCount();
      branch += structure().trellis().inputCount();
      ++forward;
    }
    
    if (output.count(Codec::Syst) || (output.count(Codec::Msg) && i < structure().length())) {
      for (size_t j = 0; j < structure().trellis().inputWidth(); ++j) {
        branch = branch_.begin() + i * structure().trellis().tableSize();
        typename LlrMetrics::Type tmp = msgUpdateImpl(branch, j);

        if (output.count(Codec::Syst)) {
          if (input.count(Codec::Syst)) {
            systOut[j] = structure().scalingFactor() * (tmp - systIn[j]);
          }
          else {
            systOut[j] = structure().scalingFactor() * (tmp);
          }
        }
        if (output.count(Codec::Msg) && i < structure().length()) {
          msgOut[j] = structure().scalingFactor() * (tmp);
        }
      }
      systIn += structure().trellis().inputWidth();
      systOut += structure().trellis().inputWidth();
      msgOut += structure().trellis().inputWidth();
    }
    if (output.count(Codec::Parity)) {
      for (size_t j = 0; j < structure().trellis().outputWidth(); ++j) {
        branch = branch_.begin() + i * structure().trellis().tableSize();
        typename LlrMetrics::Type tmp = parityUpdateImpl(branch, j);
        
        if (input.count(Codec::Parity)) {
          parityOut[j] = structure().scalingFactor() * (tmp - parityIn[j]);
        }
        else {
          parityOut[j] = structure().scalingFactor() * (tmp);
        }
      }
      parityIn += structure().trellis().outputWidth();
      parityOut += structure().trellis().outputWidth();
    }
  }
}

template <class LlrMetrics, template <class> class LogSumAlg>
template <class U, typename std::enable_if<U::value>::type*>
void MapDecoderImpl<LlrMetrics, LogSumAlg>::forwardUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator forward, typename std::vector<typename LlrMetrics::Type>::const_iterator branch)
{
  std::fill(forward + structure().trellis().stateCount(), forward + 2*structure().trellis().stateCount(), logSum_.prior(-llrMetrics_.max()));
  auto state = structure().trellis().beginState();
  for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
    for (BitField<size_t> k = 0; k < structure().trellis().inputCount(); ++k) {
      auto & forwardMetricRef = forward[structure().trellis().stateCount() + size_t(state[k])];
      forwardMetricRef =
      logSum_.sum(forwardMetricRef, logSum_.prior(forward[j] + branch[k]));
    }
    branch += structure().trellis().inputCount();
    state += structure().trellis().inputCount();
  }
}

template <class LlrMetrics, template <class> class LogSumAlg>
template <class U, typename std::enable_if<!U::value>::type*>
void MapDecoderImpl<LlrMetrics, LogSumAlg>::forwardUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator forward, typename std::vector<typename LlrMetrics::Type>::const_iterator branch)
{
  auto state = structure().trellis().beginState();
  auto buffer = buffer_.begin();
  auto maxMetric = buffer + structure().trellis().stateCount()*structure().trellis().inputCount();
  std::fill(maxMetric, maxMetric + structure().trellis().stateCount(), -llrMetrics_.max());
  for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
    for (BitField<size_t> k = 0; k < structure().trellis().inputCount(); ++k) {
      buffer[k] = forward[j] + branch[k];
      maxMetric[size_t(state[k])] = logSum_.max(buffer[k], maxMetric[size_t(state[k])]);
    }
    branch += structure().trellis().inputCount();
    buffer += structure().trellis().inputCount();
    state += structure().trellis().inputCount();
  }
  forward += structure().trellis().stateCount();
  std::fill(forward, forward + structure().trellis().stateCount(), 0);
  state = structure().trellis().beginState();
  buffer = buffer_.begin();
  for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
    for (BitField<size_t> k = 0; k < structure().trellis().inputCount(); ++k) {
      buffer[k] = logSum_.prior(buffer[k], maxMetric[size_t(state[k])]);
      forward[size_t(state[k])] = logSum_.sum(buffer[k], forward[size_t(state[k])]);
    }
    buffer += structure().trellis().inputCount();
    state += structure().trellis().inputCount();
  }
  for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
    forward[j] = logSum_.post(forward[j], maxMetric[j]);
  }
}

template <class LlrMetrics, template <class> class LogSumAlg>
template <class U, typename std::enable_if<U::value>::type*>
void MapDecoderImpl<LlrMetrics, LogSumAlg>::backwardUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator backward, typename std::vector<typename LlrMetrics::Type>::const_iterator branch)
{
  std::fill(backward, backward + structure().trellis().stateCount(), logSum_.prior(-llrMetrics_.max()));
  auto state = structure().trellis().beginState();
  for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
    for (BitField<size_t> k = 0; k < structure().trellis().inputCount(); ++k) {
      backward[j] =
      logSum_.sum(
             backward[j],
             logSum_.prior(backward[size_t(state[k])+structure().trellis().stateCount()] + branch[k])
             );
    }
    branch += structure().trellis().inputCount();
    state += structure().trellis().inputCount();
  }
}

template <class LlrMetrics, template <class> class LogSumAlg>
template <class U, typename std::enable_if<!U::value>::type*>
void MapDecoderImpl<LlrMetrics, LogSumAlg>::backwardUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator backward, typename std::vector<typename LlrMetrics::Type>::const_iterator branch)
{
  auto state = structure().trellis().beginState();
  auto buffer = buffer_.begin();
  std::fill(backward, backward + structure().trellis().stateCount(), 0);
  for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
    typename LlrMetrics::Type max = -llrMetrics_.max();
    for (BitField<size_t> k = 0; k < structure().trellis().inputCount(); ++k) {
      buffer[k] = backward[size_t(state[k])+structure().trellis().stateCount()] + branch[k];
      max = logSum_.max(buffer[k], max);
    }
    for (BitField<size_t> k = 0; k < structure().trellis().inputCount(); ++k) {
      buffer[k] = logSum_.prior(buffer[k], max);
      backward[j] = logSum_.sum(buffer[k], backward[j]);
    }
    backward[j] = logSum_.post(backward[j], max);
    branch += structure().trellis().inputCount();
    buffer += structure().trellis().inputCount();
    state += structure().trellis().inputCount();
  }
}

template <class LlrMetrics, template <class> class LogSumAlg>
template <class U, typename std::enable_if<U::value>::type*>
typename LlrMetrics::Type MapDecoderImpl<LlrMetrics, LogSumAlg>::msgUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator branch, size_t j)
{
  typename LlrMetrics::Type metric[2] = {logSum_.prior(-llrMetrics_.max()), logSum_.prior(-llrMetrics_.max())};
  for (size_t k = 0; k < structure().trellis().stateCount(); ++k) {
    for (BitField<size_t> input = 0; input < structure().trellis().inputCount(); ++input) {
      metric[input.test(j)] = logSum_.sum(metric[input.test(j)], branch[input]);
    }
    branch += structure().trellis().inputCount();
  }
  return logSum_.post(metric[1]) - logSum_.post(metric[0]);
}

template <class LlrMetrics, template <class> class LogSumAlg>
template <class U, typename std::enable_if<U::value>::type*>
typename LlrMetrics::Type MapDecoderImpl<LlrMetrics, LogSumAlg>::parityUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator branch, size_t j)
{
  typename LlrMetrics::Type metric[2] = {logSum_.prior(-llrMetrics_.max()), logSum_.prior(-llrMetrics_.max())};
  for (auto output = structure().trellis().beginOutput(); output < structure().trellis().endOutput();) {
    for (BitField<size_t> input = 0; input < structure().trellis().inputCount(); ++input) {
      metric[output[input].test(j)] = logSum_.sum(metric[output[input].test(j)], branch[input]);
    }
    branch += structure().trellis().inputCount();
    output += structure().trellis().inputCount();
  }
  return logSum_.post(metric[1]) - logSum_.post(metric[0]);
}

template <class LlrMetrics, template <class> class LogSumAlg>
template <class U, typename std::enable_if<!U::value>::type*>
typename LlrMetrics::Type MapDecoderImpl<LlrMetrics, LogSumAlg>::msgUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator branch, size_t j)
{
  typename LlrMetrics::Type max[2] = {typename LlrMetrics::Type(-llrMetrics_.max()), typename LlrMetrics::Type(-llrMetrics_.max())};
  typename LlrMetrics::Type metric[2] = {
    typename LlrMetrics::Type(0),
    typename LlrMetrics::Type(0)};
  auto branchTmp = branch;
  for (size_t k = 0; k < structure().trellis().stateCount(); ++k) {
    for (BitField<size_t> input = 0; input < structure().trellis().inputCount(); ++input) {
      max[input.test(j)] = logSum_.max(branch[input], max[input.test(j)]);
    }
    branch += structure().trellis().inputCount();
  }
  branch = branchTmp;
  for (size_t k = 0; k < structure().trellis().stateCount(); ++k) {
    for (BitField<size_t> input = 0; input < structure().trellis().inputCount(); ++input) {
      metric[input.test(j)] = logSum_.sum(logSum_.prior(branch[input], max[input.test(j)]), metric[input.test(j)]);
    }
    branch += structure().trellis().inputCount();
  }
  return logSum_.post(metric[1], max[1]) - logSum_.post(metric[0], max[0]);
}

template <class LlrMetrics, template <class> class LogSumAlg>
template <class U, typename std::enable_if<!U::value>::type*>
typename LlrMetrics::Type MapDecoderImpl<LlrMetrics, LogSumAlg>::parityUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator branch, size_t j)
{
  typename LlrMetrics::Type max[2] = {typename LlrMetrics::Type(-llrMetrics_.max()), typename LlrMetrics::Type(-llrMetrics_.max())};
  typename LlrMetrics::Type metric[2] = {
    typename LlrMetrics::Type(0),
    typename LlrMetrics::Type(0)};
  auto branchTmp = branch;
  for (auto output = structure().trellis().beginOutput(); output < structure().trellis().endOutput();) {
    for (BitField<size_t> input = 0; input < structure().trellis().inputCount(); ++input) {
      max[output[input].test(j)] = logSum_.max(branch[input], max[output[input].test(j)]);
    }
    branch += structure().trellis().inputCount();
    output += structure().trellis().inputCount();
  }
  branch = branchTmp;
  for (auto output = structure().trellis().beginOutput(); output < structure().trellis().endOutput();) {
    for (BitField<size_t> input = 0; input < structure().trellis().inputCount(); ++input) {
      metric[output[input].test(j)] = logSum_.sum(logSum_.prior(branch[input], max[output[input].test(j)]), metric[output[input].test(j)]);
    }
    branch += structure().trellis().inputCount();
    output += structure().trellis().inputCount();
  }
  return logSum_.post(metric[1], max[1]) - logSum_.post(metric[0], max[0]);
}

//Explicit instantiation
template class fec::detail::MapDecoderImpl<FloatLlrMetrics, LogSum>;
template class fec::detail::MapDecoderImpl<FloatLlrMetrics, MaxLogSum>;
template class fec::detail::MapDecoderImpl<FloatLlrMetrics, LinearLogSum>;
