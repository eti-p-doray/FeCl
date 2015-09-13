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
 
 Declaration of MapDecoder abstract class
 ******************************************************************************/

#include "MapDecoderImpl.h"

using namespace fec;

/**
 *  Constructor.
 *  Allocates metric buffers based on the given code structure.
 *  \param  codeStructure Convolutional code structure describing the code
 */
template <class LlrMetrics, template <class> class LogSumAlg>
MapDecoderImpl<LlrMetrics, LogSumAlg>::MapDecoderImpl(const Convolutional::Structure& structure) :
MapDecoder(structure)
{
  branchMetrics_.resize((structure.length()+structure.tailSize())*structure.trellis().inputCount()*structure.trellis().stateCount());
  forwardMetrics_.resize((structure.length()+structure.tailSize())*structure.trellis().stateCount());
  backwardMetrics_.resize((structure.length()+structure.tailSize())*structure.trellis().stateCount());
  
  bufferMetrics_.resize(std::max(structure.trellis().outputCount(), structure.trellis().inputCount()));
  if (!LogSumAlg<LlrMetrics>::isRecursive::value) {
    bufferMetrics_.resize(structure.trellis().stateCount()*(structure.trellis().inputCount()+1));
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
void MapDecoderImpl<LlrMetrics, LogSumAlg>::soDecodeBlock(Code::InputIterator input, Code::OutputIterator output)
{
  soDecodeBlockImpl<LlrType>(input, output);
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
template <class T>
void MapDecoderImpl<LlrMetrics, LogSumAlg>::soDecodeBlockImpl(Code::InfoIterator<typename std::vector<T>::const_iterator> input, Code::InfoIterator<typename std::vector<T>::iterator> output)
{
  branchUpdate<T>(input);
  forwardUpdate();
  backwardUpdate();
  aPosterioriUpdate<T>(input, output);
}

template <class LlrMetrics, template <class> class LogSumAlg>
template <class T>
void MapDecoderImpl<LlrMetrics, LogSumAlg>::branchUpdate(Code::InfoIterator<typename std::vector<T>::const_iterator> input)
{
  auto parity = input.parity();
  auto syst = input.syst();
  auto branchMetric = branchMetrics_.begin();
  for (size_t i = 0; i < structure().length() + structure().tailSize(); ++i) {
    for (BitField<size_t> j = 0; j < structure().trellis().outputCount(); ++j) {
      bufferMetrics_[j] = correlation<LlrMetrics>(j, parity, structure().trellis().outputSize());
    }
    auto branchMetricTmp = branchMetric;
    for (auto output = structure().trellis().beginOutput(); output < structure().trellis().endOutput();) {
      for (size_t k = 0; k < structure().trellis().inputCount(); ++k) {
        branchMetric[k] = bufferMetrics_[size_t(output[k])];
      }
      output += structure().trellis().inputCount();
      branchMetric += structure().trellis().inputCount();
    }
    
    if (input.hasSyst()) {
      branchMetric = branchMetricTmp;
      for (BitField<size_t> j = 0; j < structure().trellis().inputCount(); ++j) {
        bufferMetrics_[j] = correlation<LlrMetrics>(j, syst, structure().trellis().inputSize());
      }
      for (size_t j = 0; j < structure().trellis().stateCount(); ++j) {
        for (size_t k = 0; k < structure().trellis().inputCount(); ++k) {
          branchMetric[k] += bufferMetrics_[k];
        }
        branchMetric += structure().trellis().inputCount();
      }
    }
    parity += structure().trellis().outputSize();
    syst += structure().trellis().inputSize();
  }
}

template <class LlrMetrics, template <class> class LogSumAlg>
void MapDecoderImpl<LlrMetrics, LogSumAlg>::forwardUpdate()
{
  auto forwardMetric = forwardMetrics_.begin();
  auto branchMetric = branchMetrics_.cbegin();
  
  *forwardMetric = 0;
  std::fill(forwardMetric+1, forwardMetric + structure().trellis().stateCount(), -llrMetrics_.max());
  
  for (; forwardMetric < forwardMetrics_.end() - structure().trellis().stateCount();) {
    forwardUpdateImpl<LogSumAlg<LlrMetrics>::isRecursive::value>(forwardMetric, branchMetric);
    forwardMetric += structure().trellis().stateCount();
    branchMetric += structure().trellis().tableSize();
    typename LlrMetrics::Type max = -llrMetrics_.max();
    for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
      forwardMetric[j] = logSum_.post(forwardMetric[j]);
      max = std::max(forwardMetric[j], max);
    }
    for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
      forwardMetric[j] -= max;
    }
  }
}

template <class LlrMetrics, template <class> class LogSumAlg>
void MapDecoderImpl<LlrMetrics, LogSumAlg>::backwardUpdate()
{
  auto backwardMetric = backwardMetrics_.end()-structure().trellis().stateCount();
  switch (structure().terminationType()) {
    case Convolutional::Tail:
      *backwardMetric = 0;
      std::fill(backwardMetric+1, backwardMetric + structure().trellis().stateCount(), -llrMetrics_.max());
      break;
      
    default:
    case Convolutional::Truncation:
      std::fill(backwardMetric, backwardMetric + structure().trellis().stateCount(), 0.0);
      break;
  }
  backwardMetric -= structure().trellis().stateCount();
  auto branchMetric = branchMetrics_.cend()-structure().trellis().tableSize();
  
  for ( ; backwardMetric >= backwardMetrics_.begin();) {
    backwardUpdateImpl<LogSumAlg<LlrMetrics>::isRecursive::value>(backwardMetric, branchMetric);
    typename LlrMetrics::Type max = -llrMetrics_.max();
    for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
      backwardMetric[j] = logSum_.post(backwardMetric[j]);
      max = std::max(backwardMetric[j], max);
    }
    for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
      backwardMetric[j] -= max;
    }
    backwardMetric -= structure().trellis().stateCount();
    branchMetric -= structure().trellis().tableSize();
  }
}

template <class LlrMetrics, template <class> class LogSumAlg>
template <typename T>
void MapDecoderImpl<LlrMetrics, LogSumAlg>::aPosterioriUpdate(Code::InfoIterator<typename std::vector<T>::const_iterator> input, Code::InfoIterator<typename std::vector<T>::iterator> output)
{
  auto systOut = output.syst();
  auto systIn = input.syst();
  auto parityOut = output.parity();
  auto parityIn = input.parity();

  for (size_t i = 0; i < structure().length() + structure().tailSize(); ++i) {
    auto branchMetric = branchMetrics_.begin() + i * structure().trellis().tableSize();
    auto forwardMetric = forwardMetrics_.cbegin() + i * structure().trellis().stateCount();
    auto backwardMetric = backwardMetrics_.cbegin() + i * structure().trellis().stateCount();
    
    for (auto state = structure().trellis().beginState(); state < structure().trellis().endState(); ) {
      for (BitField<size_t> input = 0; input < structure().trellis().inputCount(); ++input) {
        branchMetric[input] = logSum_.prior(branchMetric[input] + *forwardMetric + backwardMetric[size_t(state[input])]);
      }
      state += structure().trellis().inputCount();
      branchMetric += structure().trellis().inputCount();
      ++forwardMetric;
    }
    
    if (output.hasSyst()) {
      for (size_t j = 0; j < structure().trellis().inputSize(); ++j) {
        branchMetric = branchMetrics_.begin() + i * structure().trellis().tableSize();
        typename LlrMetrics::Type tmp = msgUpdateImpl<LogSumAlg<LlrMetrics>::isRecursive::value>(branchMetric, j);

        if (input.hasSyst()) {
          systOut[j] = tmp - systIn[j];
        }
        else {
          systOut[j] = tmp;
        }
      }
      systOut += structure().trellis().inputSize();
    }
    if (output.hasParity()) {
      for (size_t j = 0; j < structure().trellis().outputSize(); ++j) {
        branchMetric = branchMetrics_.begin() + i * structure().trellis().tableSize();
        typename LlrMetrics::Type tmp = parityUpdateImpl<LogSumAlg<LlrMetrics>::isRecursive::value>(branchMetric, j);
        
        if (input.hasParity()) {
          parityOut[j] = tmp - parityIn[j];
        }
        else {
          parityOut[j] = tmp;
        }
      }
      parityOut += structure().trellis().outputSize();
    }
  }
}

template <class LlrMetrics, template <class> class LogSumAlg>
template <bool isRecursive>
void MapDecoderImpl<LlrMetrics, LogSumAlg>::forwardUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator forwardMetric, typename std::vector<typename LlrMetrics::Type>::const_iterator branchMetric, char(*)[isRecursive])
{
  std::fill(forwardMetric + structure().trellis().stateCount(), forwardMetric + 2*structure().trellis().stateCount(), logSum_.prior(-llrMetrics_.max()));
  auto state = structure().trellis().beginState();
  for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
    for (BitField<size_t> k = 0; k < structure().trellis().inputCount(); ++k) {
      auto & forwardMetricRef = forwardMetric[structure().trellis().stateCount() + size_t(state[k])];
      forwardMetricRef =
      logSum_.sum(forwardMetricRef, logSum_.prior(forwardMetric[j] + branchMetric[k]));
    }
    branchMetric += structure().trellis().inputCount();
    state += structure().trellis().inputCount();
  }
}

template <class LlrMetrics, template <class> class LogSumAlg>
template <bool isRecursive>
void MapDecoderImpl<LlrMetrics, LogSumAlg>::forwardUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator forwardMetric, typename std::vector<typename LlrMetrics::Type>::const_iterator branchMetric, char(*)[!isRecursive])
{
  auto state = structure().trellis().beginState();
  auto bufferMetric = bufferMetrics_.begin();
  auto maxMetric = bufferMetric + structure().trellis().stateCount()*structure().trellis().inputCount();
  std::fill(maxMetric, maxMetric + structure().trellis().stateCount(), -llrMetrics_.max());
  for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
    for (BitField<size_t> k = 0; k < structure().trellis().inputCount(); ++k) {
      bufferMetric[k] = forwardMetric[j] + branchMetric[k];
      maxMetric[size_t(state[k])] = logSum_.max(bufferMetric[k], maxMetric[size_t(state[k])]);
    }
    branchMetric += structure().trellis().inputCount();
    bufferMetric += structure().trellis().inputCount();
    state += structure().trellis().inputCount();
  }
  forwardMetric += structure().trellis().stateCount();
  std::fill(forwardMetric, forwardMetric + structure().trellis().stateCount(), 0);
  state = structure().trellis().beginState();
  bufferMetric = bufferMetrics_.begin();
  for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
    for (BitField<size_t> k = 0; k < structure().trellis().inputCount(); ++k) {
      bufferMetric[k] = logSum_.prior(bufferMetric[k], maxMetric[size_t(state[k])]);
      forwardMetric[size_t(state[k])] = logSum_.sum(bufferMetric[k], forwardMetric[size_t(state[k])]);
    }
    bufferMetric += structure().trellis().inputCount();
    state += structure().trellis().inputCount();
  }
  for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
    forwardMetric[j] = logSum_.post(forwardMetric[j], maxMetric[j]);
  }
}

template <class LlrMetrics, template <class> class LogSumAlg>
template <bool isRecursive>
void MapDecoderImpl<LlrMetrics, LogSumAlg>::backwardUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator backwardMetric, typename std::vector<typename LlrMetrics::Type>::const_iterator branchMetric, char(*)[isRecursive])
{
  std::fill(backwardMetric, backwardMetric + structure().trellis().stateCount(), logSum_.prior(-llrMetrics_.max()));
  auto state = structure().trellis().beginState();
  for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
    for (BitField<size_t> k = 0; k < structure().trellis().inputCount(); ++k) {
      backwardMetric[j] =
      logSum_.sum(
             backwardMetric[j],
             logSum_.prior(backwardMetric[size_t(state[k])+structure().trellis().stateCount()] + branchMetric[k])
             );
    }
    branchMetric += structure().trellis().inputCount();
    state += structure().trellis().inputCount();
  }
}

template <class LlrMetrics, template <class> class LogSumAlg>
template <bool isRecursive>
void MapDecoderImpl<LlrMetrics, LogSumAlg>::backwardUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator backwardMetric, typename std::vector<typename LlrMetrics::Type>::const_iterator branchMetric, char(*)[!isRecursive])
{
  auto state = structure().trellis().beginState();
  auto bufferMetric = bufferMetrics_.begin();
  std::fill(backwardMetric, backwardMetric + structure().trellis().stateCount(), 0);
  for (BitField<size_t> j = 0; j < structure().trellis().stateCount(); ++j) {
    typename LlrMetrics::Type max = -llrMetrics_.max();
    for (BitField<size_t> k = 0; k < structure().trellis().inputCount(); ++k) {
      bufferMetric[k] = backwardMetric[size_t(state[k])+structure().trellis().stateCount()] + branchMetric[k];
      max = logSum_.max(bufferMetric[k], max);
    }
    for (BitField<size_t> k = 0; k < structure().trellis().inputCount(); ++k) {
      bufferMetric[k] = logSum_.prior(bufferMetric[k], max);
      backwardMetric[j] = logSum_.sum(bufferMetric[k], backwardMetric[j]);
    }
    backwardMetric[j] = logSum_.post(backwardMetric[j], max);
    branchMetric += structure().trellis().inputCount();
    bufferMetric += structure().trellis().inputCount();
    state += structure().trellis().inputCount();
  }
}

template <class LlrMetrics, template <class> class LogSumAlg>
template <bool isRecursive>
typename LlrMetrics::Type MapDecoderImpl<LlrMetrics, LogSumAlg>::msgUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator branchMetric, size_t j, char(*)[isRecursive])
{
  typename LlrMetrics::Type metric[2] = {logSum_.prior(-llrMetrics_.max()), logSum_.prior(-llrMetrics_.max())};
  for (size_t k = 0; k < structure().trellis().stateCount(); ++k) {
    for (BitField<size_t> input = 0; input < structure().trellis().inputCount(); ++input) {
      metric[input.test(j)] = logSum_.sum(metric[input.test(j)], branchMetric[input]);
    }
    branchMetric += structure().trellis().inputCount();
  }
  return logSum_.post(metric[1]) - logSum_.post(metric[0]);
}

template <class LlrMetrics, template <class> class LogSumAlg>
template <bool isRecursive>
typename LlrMetrics::Type MapDecoderImpl<LlrMetrics, LogSumAlg>::parityUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator branchMetric, size_t j, char(*)[isRecursive])
{
  typename LlrMetrics::Type metric[2] = {logSum_.prior(-llrMetrics_.max()), logSum_.prior(-llrMetrics_.max())};
  for (auto output = structure().trellis().beginOutput(); output < structure().trellis().endOutput();) {
    for (BitField<size_t> input = 0; input < structure().trellis().inputCount(); ++input) {
      metric[output[input].test(j)] = logSum_.sum(metric[output[input].test(j)], branchMetric[input]);
    }
    branchMetric += structure().trellis().inputCount();
    output += structure().trellis().inputCount();
  }
  return logSum_.post(metric[1]) - logSum_.post(metric[0]);
}

template <class LlrMetrics, template <class> class LogSumAlg>
template <bool isRecursive>
typename LlrMetrics::Type MapDecoderImpl<LlrMetrics, LogSumAlg>::msgUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator branchMetric, size_t j, char(*)[!isRecursive])
{
  typename LlrMetrics::Type max[2] = {typename LlrMetrics::Type(-llrMetrics_.max()), typename LlrMetrics::Type(-llrMetrics_.max())};
  typename LlrMetrics::Type metric[2] = {
    typename LlrMetrics::Type(0),
    typename LlrMetrics::Type(0)};
  auto branchMetricTmp = branchMetric;
  for (size_t k = 0; k < structure().trellis().stateCount(); ++k) {
    for (BitField<size_t> input = 0; input < structure().trellis().inputCount(); ++input) {
      max[input.test(j)] = logSum_.max(branchMetric[input], max[input.test(j)]);
    }
    branchMetric += structure().trellis().inputCount();
  }
  branchMetric = branchMetricTmp;
  for (size_t k = 0; k < structure().trellis().stateCount(); ++k) {
    for (BitField<size_t> input = 0; input < structure().trellis().inputCount(); ++input) {
      branchMetric[input] = logSum_.prior(branchMetric[input], max[input.test(j)]);
      metric[input.test(j)] = logSum_.sum(branchMetric[input], metric[input.test(j)]);
    }
    branchMetric += structure().trellis().inputCount();
  }
  return logSum_.post(metric[1], max[1]) - logSum_.post(metric[0], max[0]);
}

template <class LlrMetrics, template <class> class LogSumAlg>
template <bool isRecursive>
typename LlrMetrics::Type MapDecoderImpl<LlrMetrics, LogSumAlg>::parityUpdateImpl(typename std::vector<typename LlrMetrics::Type>::iterator branchMetric, size_t j, char(*)[!isRecursive])
{
  typename LlrMetrics::Type max[2] = {typename LlrMetrics::Type(-llrMetrics_.max()), typename LlrMetrics::Type(-llrMetrics_.max())};
  typename LlrMetrics::Type metric[2] = {
    typename LlrMetrics::Type(0),
    typename LlrMetrics::Type(0)};
  auto branchMetricTmp = branchMetric;
  for (auto output = structure().trellis().beginOutput(); output < structure().trellis().endOutput();) {
    for (BitField<size_t> input = 0; input < structure().trellis().inputCount(); ++input) {
      max[output[input].test(j)] = logSum_.max(branchMetric[input], max[output[input].test(j)]);
    }
    branchMetric += structure().trellis().inputCount();
    output += structure().trellis().inputCount();
  }
  branchMetric = branchMetricTmp;
  for (auto output = structure().trellis().beginOutput(); output < structure().trellis().endOutput();) {
    for (BitField<size_t> input = 0; input < structure().trellis().inputCount(); ++input) {
      branchMetric[input] = logSum_.prior(branchMetric[input], max[output[input].test(j)]);
      metric[output[input].test(j)] = logSum_.sum(branchMetric[input], metric[output[input].test(j)]);
    }
    branchMetric += structure().trellis().inputCount();
    output += structure().trellis().inputCount();
  }
  return logSum_.post(metric[1], max[1]) - logSum_.post(metric[0], max[0]);
}

//Explicit instantiation
template class fec::MapDecoderImpl<FloatLlrMetrics, LogSum>;
template class fec::MapDecoderImpl<FloatLlrMetrics, MaxLogSum>;
template class fec::MapDecoderImpl<FloatLlrMetrics, TableLogSum>;

template class fec::MapDecoderImpl<FixLlrMetrics, LogSum>;
template class fec::MapDecoderImpl<FixLlrMetrics, MaxLogSum>;
template class fec::MapDecoderImpl<FixLlrMetrics, TableLogSum>;
