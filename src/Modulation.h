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
#include "detail/WorkGroup.h"

namespace fec {

  class Modulation {
  public:
    enum SymbolMapping {
      Binary,
      Grey,
      Custom,
    };
    
    struct RectangularQam {
    public:
      RectangularQam(size_t order);
      RectangularQam& avgPower(double power);
      RectangularQam& phaseOffset(double offset);
      RectangularQam& symbolMapping(SymbolMapping mapping);
      
      operator std::vector<std::vector<double>>();
      
    private:
      
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
    
    template <class T, template <typename> class A = std::allocator>
    using Input = detail::Modulation::Arguments<const std::vector<T,A<T>>>;
    template <class T, template <typename> class A = std::allocator>
    using Output = detail::Modulation::Arguments<std::vector<T,A<T>>>;
    
    using ModOptions = detail::Modulation::ModOptions;
    using DemodOptions = detail::Modulation::DemodOptions;
    
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
    
    size_t wordWidth() const {return structure().wordWidth();} /**< Access the width of msg in each code bloc. */
    size_t symbolWidth() const {return structure().symbolWidth();} /**< Access the width of systematics in each code bloc. */
    
    size_t wordSize() const {return structure().wordSize();} /**< Access the width of msg in each code bloc. */
    size_t symbolSize() const {return structure().symbolSize();;} /**< Access the width of systematics in each code bloc. */
    
    void setDemodOptions(const DemodOptions& demod);
    DemodOptions getDemodOptions() const;
    
    template <template <typename> class A>
    void modulate(const std::vector<BitField<size_t>,A<BitField<size_t>>>& word, std::vector<double,A<double>>& symbol) const;
    template <template <typename> class A>
    std::vector<double> modulate(const std::vector<BitField<size_t>,A<BitField<size_t>>>& msg) const;
    
    template <template <typename> class A>
    void demodulate(const std::vector<double,A<double>>& symbol, std::vector<BitField<size_t>,A<BitField<size_t>>>& word) const;
    template <template <typename> class A>
    std::vector<BitField<size_t>> demodulate(const std::vector<double,A<double>>& word) const;
    
    template <class T, template <typename> class A>
    void soDemodulate(detail::Modulation::Arguments<T> input, const std::vector<double>& variance, std::vector<double,A<double>>& word) const;
    template <class T>
    std::vector<double> soDemodulate(detail::Modulation::Arguments<T> input, const std::vector<double>& variance) const;

  protected:
    inline const detail::Modulation::Structure& structure() const {return *structure_;}
    inline detail::Modulation::Structure& structure() {return *structure_;}
    
    void modulateBlocks(detail::Modulation::const_iterator<BitField<size_t>> first, detail::Modulation::const_iterator<BitField<size_t>> last, detail::Modulation::iterator<double> output) const;
    void demodulateBlocks(detail::Modulation::const_iterator<double> first, detail::Modulation::const_iterator<double> last, detail::Modulation::iterator<BitField<size_t>> output) const;
    void soDemodulateBlocks(detail::Modulation::const_iterator<double> first, detail::Modulation::const_iterator<double> last, double variance, detail::Modulation::iterator<double> output) const;
    
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
template <template <typename> class A>
std::vector<double> fec::Modulation::modulate(const std::vector<fec::BitField<size_t>,A<fec::BitField<size_t>>>& word) const
{
  std::vector<double> symbol;
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
template <template <typename> class A>
void fec::Modulation::modulate(const std::vector<BitField<size_t>,A<BitField<size_t>>>& word, std::vector<double,A<double>>& symbol) const
{
  uint64_t blockCount = word.size() / (wordSize());
  if (word.size() != blockCount * wordSize()) {
    throw std::invalid_argument("Invalid size for word");
  }
  symbol.resize(blockCount * symbolSize() * symbolWidth(), 0);
  
  detail::Modulation::const_iterator<BitField<size_t>> begin{{detail::Modulation::Word, word.begin(), wordSize()}};
  detail::Modulation::const_iterator<BitField<size_t>> end{{detail::Modulation::Word, word.end(), wordSize()}};
  detail::Modulation::iterator<double> output{{detail::Modulation::Symbol, symbol.begin(), symbolSize()*symbolWidth()}};
  
  detail::WorkGroup workGroup(workGroupSize_);
  workGroup.executeTask(begin, end, output, std::bind(&Modulation::modulateBlocks, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

/**
 *  Encodes several blocks of information bits.
 *  Chunks of blocs are encded in parallel.
 *  \param  message  Vector containing information bits
 *  \return Vector containing parity bits
 *  \tparam A Container allocator. The reason for different allocator is to allow
 *    the matlab API to use a custom mex allocator
 */
template <template <typename> class A>
std::vector<fec::BitField<size_t>> fec::Modulation::demodulate(const std::vector<double,A<double>>& symbol) const
{
  std::vector<BitField<size_t>> word;
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
template <template <typename> class A>
void fec::Modulation::demodulate(const std::vector<double,A<double>>& symbol, std::vector<BitField<size_t>,A<BitField<size_t>>>& word) const
{
  size_t blockCount = symbol.size() / (symbolSize()*symbolWidth());
  
  detail::Modulation::const_iterator<double> begin{{detail::Modulation::Symbol, symbol.begin(), symbolSize()*symbolWidth()}};
  detail::Modulation::const_iterator<double> end{{detail::Modulation::Symbol, symbol.end(), symbolSize()*symbolWidth()}};
  detail::Modulation::iterator<BitField<size_t>> outputIt;
  
  if (symbol.size() != blockCount * symbolSize()*symbolWidth()) {
    throw std::invalid_argument("Invalid size for symbol");
  }
  
  word.resize(blockCount * wordSize());
  outputIt.insert(detail::Modulation::Word, word.begin(), wordSize());
  
  detail::WorkGroup workGroup(workGroupSize_);
  workGroup.executeTask(begin, end, outputIt, std::bind(&Modulation::demodulateBlocks, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

/**
 *  Encodes several blocks of information bits.
 *  Chunks of blocs are encded in parallel.
 *  \param  message  Vector containing information bits
 *  \return Vector containing parity bits
 *  \tparam A Container allocator. The reason for different allocator is to allow
 *    the matlab API to use a custom mex allocator
 */

template <class T>
std::vector<double> fec::Modulation::soDemodulate(detail::Modulation::Arguments<T> input, const std::vector<double>& variance) const
{
  std::vector<double> word;
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
template <class T, template <typename> class A>
void fec::Modulation::soDemodulate(detail::Modulation::Arguments<T> input, const std::vector<double>& variance, std::vector<double,A<double>>& word) const
{
  if (!input.count(detail::Modulation::Symbol)) {
    throw std::invalid_argument("Input must contains symbol");
  }
  size_t blockCount = input.at(detail::Modulation::Symbol).size() / (symbolSize()*symbolWidth()*variance.size());
  
  detail::Modulation::const_iterator<double> begin{{detail::Modulation::Symbol, input.at(detail::Modulation::Symbol).begin(), symbolSize()*symbolWidth()}};
  detail::Modulation::const_iterator<double> end{{detail::Modulation::Symbol, input.at(detail::Modulation::Symbol).end(), symbolSize()*symbolWidth()}};
  detail::Modulation::iterator<double> outputIt;
  
  if (input.at(detail::Modulation::Symbol).size() != blockCount * symbolSize()*symbolWidth() * variance.size()) {
    throw std::invalid_argument("Invalid size for symbol");
  }
  if (input.count(detail::Modulation::Word)) {
    begin.insert(detail::Modulation::Word, input.at(detail::Modulation::Word).begin(), wordSize());
    end.insert(detail::Modulation::Word, input.at(detail::Modulation::Word).end(), wordSize());
    if (input.at(detail::Modulation::Word).size() != blockCount * wordSize()) {
      throw std::invalid_argument("Invalid size for word");
    }
  }
  
  word.resize(variance.size() * blockCount * wordSize());
  outputIt.insert(detail::Modulation::Word, word.begin(), wordSize());
  
  detail::WorkGroup workGroup(workGroupSize_);
  for (auto i : variance) {
    workGroup.addTask(begin, end, outputIt, std::bind(&Modulation::soDemodulateBlocks, this, std::placeholders::_1, std::placeholders::_2, i, std::placeholders::_3));
    begin += blockCount;
    outputIt += blockCount;
  }
  workGroup.wait();
}


#endif