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

#ifndef FEC_MODULATION_H
#define FEC_MODULATION_H

#include <memory>
#include <thread>
#include <vector>

#include "detail/Modulation.h"
#include "detail/ModulationFunction.h"
#include "detail/MultiIterator.h"
#include "detail/WorkGroup.h"
#include "detail/rebind.h"

namespace fec {

  class Modulation {
  public:
    enum SymbolMapping {
      Binary,
      Grey,
      Custom,
    };
    
    using ModOptions = detail::Modulation::ModOptions;
    using DemodOptions = detail::Modulation::DemodOptions;
    
    struct RectangularQam {
    public:
      RectangularQam(size_t order);
      RectangularQam& avgPower(double power);
      RectangularQam& phaseOffset(double offset);
      RectangularQam& symbolMapping(SymbolMapping mapping);
      
      operator ModOptions();
      
      std::vector<std::vector<double>> constellation();
      
    private:
      size_t order_;
      
    };
    
    struct Pam {
    public:
      Pam(size_t order);
      Pam& avgPower(double power);
      Pam& phaseOffset(double offset);
      Pam& symbolMapping(SymbolMapping mapping);
      
      operator std::vector<std::vector<double>>();
      
    private:
      
    };

    struct Input {
      template <typename T> static detail::Modulation::ConstArguments<T> word(T& word) {return detail::Modulation::ConstArguments<T>{}.word(word);}
      template <typename T> static detail::Modulation::ConstArguments<T> symbol(T& symbol) {return detail::Modulation::ConstArguments<T>{}.symbol(symbol);}
    } Input;
    struct Output {
      template <typename T> static detail::Modulation::Arguments<T> word(T& word) {return detail::Modulation::Arguments<T>{}.word(word);}
      template <typename T> static detail::Modulation::Arguments<T> symbol(T& symbol) {return detail::Modulation::Arguments<T>{}.symbol(symbol);}
    } Output;
    
    template <typename T>
    static detail::Modulation::Arguments<T> word(T& word) {return detail::Modulation::Arguments<T>{}.word(word);}
    template <typename T>
    static detail::Modulation::Arguments<T> symbol(T& symbol) {return detail::Modulation::Arguments<T>{}.symbol(symbol);}
    
    Modulation(const ModOptions&);
    Modulation(const ModOptions&, const DemodOptions&);
    Modulation(const Modulation& other) {*this = other;}
    ~Modulation() = default;
    Modulation& operator=(const Modulation& other) {structure_ = std::unique_ptr<detail::Modulation::Structure>(new detail::Modulation::Structure(other.structure())); return *this;}
    
    const char * get_key() const;
    
    size_t wordCount() const {return structure().wordCount();} /**< Access the width of msg in each code bloc. */

    size_t wordWidth() const {return structure().wordWidth();} /**< Access the width of msg in each code bloc. */
    size_t symbolWidth() const {return structure().symbolWidth();} /**< Access the width of systematics in each code bloc. */
    
    size_t wordSize() const {return structure().wordSize();} /**< Access the width of msg in each code bloc. */
    size_t symbolSize() const {return structure().symbolSize();;} /**< Access the width of systematics in each code bloc. */
    
    void setDemodOptions(const DemodOptions& demod);
    DemodOptions getDemodOptions() const;
    
    template <class InputVector, class OutputVector>
    void modulate(const InputVector& word, OutputVector& symbol) const;
    template <class InputVector, class OutputVector = typename detail::rebind<InputVector, double>::type>
    OutputVector modulate(const InputVector& msg) const;
    
    template <class InputVector, class OutputVector>
    void demodulate(const InputVector& symbol, OutputVector& word) const;
    template <class InputVector, class OutputVector>
    OutputVector demodulate(const InputVector& word) const;
    
    template <class InputVector1, class InputVector2, class OutputVector>
    void soDemodulate(detail::Modulation::Arguments<const InputVector1> input, const InputVector2& variance, OutputVector& word) const;
    template <class InputVector1, class InputVector2 = std::vector<double>, class OutputVector = InputVector1>
    OutputVector soDemodulate(detail::Modulation::Arguments<const InputVector1> input, const InputVector2& variance) const;

  protected:
    inline const detail::Modulation::Structure& structure() const {return *structure_;}
    inline detail::Modulation::Structure& structure() {return *structure_;}
    
    template <class InputIterator, class OutputIterator>
    void modulateBlocks(detail::Modulation::iterator<InputIterator> first, detail::Modulation::iterator<InputIterator> last, detail::Modulation::iterator<OutputIterator> output) const;
    template <class InputIterator, class OutputIterator>
    void demodulateBlocks(detail::Modulation::iterator<InputIterator> first, detail::Modulation::iterator<InputIterator> last, detail::Modulation::iterator<OutputIterator> output) const;
    template <class InputIterator, class OutputIterator>
    void soDemodulateBlocks(detail::Modulation::iterator<InputIterator> first, detail::Modulation::iterator<InputIterator> last, typename InputIterator::value_type variance, detail::Modulation::iterator<OutputIterator> output) const;
    
    std::unique_ptr<detail::Modulation::Structure> structure_;
    
  private:
    Modulation(const detail::Modulation::Structure&);
    
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version);

    int workGroupSize_ = 0;
    
  };
  
}


BOOST_CLASS_EXPORT_KEY(fec::Modulation);
BOOST_CLASS_TYPE_INFO(fec::Modulation,extended_type_info_no_rtti<fec::Modulation>);


/**
 *  Encodes several blocks of information bits.
 *  Chunks of blocs are encded in parallel.
 *  \param  message  Vector containing information bits
 *  \return Vector containing parity bits
 *  \tparam A Container allocator. The reason for different allocator is to allow
 *    the matlab API to use a custom mex allocator
 */
template <class InputVector, class OutputVector>
OutputVector fec::Modulation::modulate(const InputVector& word) const
{
  OutputVector symbol;
  modulate(word, symbol);
  return symbol;
}

/**
 *  Encodes several blocks of information bits.
 *  Chunks of blocs are encded in parallel.
 *  \param  msg  Vector containing information bits
 *  \param  parity[out] Vector containing parity bits
 *  \tparam A Container allocator. The reason for different allocator is to allow
 *    the matlab API to use a custom mex allocator
 */
template <class InputVector, class OutputVector>
void fec::Modulation::modulate(const InputVector& word, OutputVector& symbol) const
{
  uint64_t blockCount = word.size() / (wordSize());
  if (word.size() != blockCount * wordSize()) {
    throw std::invalid_argument("Invalid size for word");
  }
  symbol.resize(blockCount * symbolSize() * symbolWidth(), 0);
  
  detail::Modulation::iterator<typename InputVector::const_iterator> begin{{detail::Modulation::Word, word.begin(), wordSize()}};
  detail::Modulation::iterator<typename InputVector::const_iterator> end{{detail::Modulation::Word, word.end(), wordSize()}};
  detail::Modulation::iterator<typename OutputVector::iterator> output{{detail::Modulation::Symbol, symbol.begin(), symbolSize()*symbolWidth()}};
  
  detail::WorkGroup workGroup(workGroupSize_);
  workGroup.executeTask(std::bind(&Modulation::modulateBlocks<typename InputVector::const_iterator, typename OutputVector::iterator>, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), begin, end, output);
}

/**
 *  Encodes several blocks of information bits.
 *  Chunks of blocs are encded in parallel.
 *  \param  message  Vector containing information bits
 *  \return Vector containing parity bits
 *  \tparam A Container allocator. The reason for different allocator is to allow
 *    the matlab API to use a custom mex allocator
 */
template <class InputVector, class OutputVector>
OutputVector fec::Modulation::demodulate(const InputVector& symbol) const
{
  OutputVector word;
  demodulate(symbol, word);
  return word;
}

/**
 *  Decodes several blocks of information bits.
 *  A posteriori information about the msg is output instead of the decoded bit sequence.
 *  Chunks of blocs are encded in parallel.
 *  \param  input  Map containing input vectors
 *  \param  output Map containing output vectors
 *  \tparam A Container allocator. The reason for different allocator is to allow
 *    the matlab API to use a custom mex allocator
 */
template <class InputVector, class OutputVector>
void fec::Modulation::demodulate(const InputVector& symbol, OutputVector& word) const
{
  size_t blockCount = symbol.size() / (symbolSize()*symbolWidth());
  
  detail::Modulation::iterator<typename InputVector::const_iterator> begin{{detail::Modulation::Symbol, symbol.begin(), symbolSize()*symbolWidth()}};
  detail::Modulation::iterator<typename InputVector::const_iterator> end{{detail::Modulation::Symbol, symbol.end(), symbolSize()*symbolWidth()}};
  detail::Modulation::iterator<typename OutputVector::iterator> outputIt;
  
  if (symbol.size() != blockCount * symbolSize()*symbolWidth()) {
    throw std::invalid_argument("Invalid size for symbol");
  }
  
  word.resize(blockCount * wordSize());
  outputIt.insert(detail::Modulation::Word, word.begin(), wordSize());
  
  detail::WorkGroup workGroup(workGroupSize_);
  workGroup.executeTask(std::bind(&Modulation::demodulateBlocks<typename InputVector::const_iterator, typename OutputVector::iterator>, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), begin, end, outputIt);
}

/**
 *  Encodes several blocks of information bits.
 *  Chunks of blocs are encded in parallel.
 *  \param  message  Vector containing information bits
 *  \return Vector containing parity bits
 *  \tparam A Container allocator. The reason for different allocator is to allow
 *    the matlab API to use a custom mex allocator
 */

template <class InputVector1, class InputVector2, class OutputVector>
OutputVector fec::Modulation::soDemodulate(detail::Modulation::Arguments<const InputVector1> input, const InputVector2& variance) const
{
  OutputVector word;
  soDemodulate(input, variance, word);
  return word;
}

/**
 *  Decodes several blocks of information bits.
 *  A posteriori information about the msg is output instead of the decoded bit sequence.
 *  Chunks of blocs are encded in parallel.
 *  \param  input  Map containing input vectors
 *  \param  output Map containing output vectors
 *  \tparam A Container allocator. The reason for different allocator is to allow
 *    the matlab API to use a custom mex allocator
 */
template <class InputVector1, class InputVector2, class OutputVector>
void fec::Modulation::soDemodulate(detail::Modulation::Arguments<const InputVector1> input, const InputVector2& variance, OutputVector& word) const
{
  if (!input.count(detail::Modulation::Symbol)) {
    throw std::invalid_argument("Input must contains symbol");
  }
  size_t blockCount = input.at(detail::Modulation::Symbol).size() / (symbolSize()*symbolWidth()*variance.size());
  
  detail::Modulation::iterator<typename InputVector1::const_iterator> begin{{detail::Modulation::Symbol, input.at(detail::Modulation::Symbol).begin(), symbolSize()*symbolWidth()}};
  detail::Modulation::iterator<typename InputVector1::const_iterator> end{{detail::Modulation::Symbol, input.at(detail::Modulation::Symbol).end(), symbolSize()*symbolWidth()}};
  detail::Modulation::iterator<typename OutputVector::iterator> outputIt;
  
  if (input.at(detail::Modulation::Symbol).size() != blockCount * symbolSize()*symbolWidth() * variance.size()) {
    throw std::invalid_argument("Invalid size for symbol");
  }
  if (input.count(detail::Modulation::Word)) {
    begin.insert(detail::Modulation::Word, input.at(detail::Modulation::Word).begin(), wordSize()*(structure().wordCount()-1));
    end.insert(detail::Modulation::Word, input.at(detail::Modulation::Word).end(), wordSize()*(structure().wordCount()-1));
    if (input.at(detail::Modulation::Word).size() != blockCount * wordSize()*(structure().wordCount()-1)) {
      throw std::invalid_argument("Invalid size for word");
    }
  }
  
  word.resize(variance.size() * blockCount * wordSize()*(structure().wordCount()-1));
  outputIt.insert(detail::Modulation::Word, word.begin(), wordSize()*(structure().wordCount()-1));
  
  detail::WorkGroup workGroup(workGroupSize_);
  for (auto i : variance) {
    workGroup.addTask(std::bind(&Modulation::soDemodulateBlocks<typename InputVector1::const_iterator, typename OutputVector::iterator>, this, std::placeholders::_1, std::placeholders::_2, i, std::placeholders::_3), begin, end, outputIt);
    begin += blockCount;
    outputIt += blockCount;
  }
  workGroup.wait();
}

template <class InputIterator, class OutputIterator>
void fec::Modulation::modulateBlocks(detail::Modulation::iterator<InputIterator> wordf, detail::Modulation::iterator<InputIterator> wordl, detail::Modulation::iterator<OutputIterator> symbol) const
{
  auto modulate = detail::Modulation::ModulateFunction<InputIterator, OutputIterator>::create(structure());
  (*modulate)(wordf, wordl, symbol);
}

template <class InputIterator, class OutputIterator>
void fec::Modulation::demodulateBlocks(detail::Modulation::iterator<InputIterator> symbolf, detail::Modulation::iterator<InputIterator> symboll, detail::Modulation::iterator<OutputIterator> word) const
{
  auto demodulate = detail::Modulation::DemodulateFunction<InputIterator, OutputIterator>::create(structure());
  (*demodulate)(symbolf, symboll, word);
}

template <class InputIterator, class OutputIterator>
void fec::Modulation::soDemodulateBlocks(detail::Modulation::iterator<InputIterator> first, detail::Modulation::iterator<InputIterator> last, typename InputIterator::value_type variance, detail::Modulation::iterator<OutputIterator> output) const
{
  auto soDemodulate = detail::Modulation::SoDemodulateFunction<InputIterator, OutputIterator>::create(structure());
  (*soDemodulate)(first, last, 1/(2*variance), output);
}


#endif