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

#ifndef FEC_WORK_GROUP_H
#define FEC_WORK_GROUP_H

#include <memory>
#include <thread>
#include <vector>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>

namespace fec {
  
  namespace detail {
    
    class WorkGroup
    {
      friend class boost::serialization::access;
    public:
      WorkGroup(int maxSize = 0) {maxSize_ = maxSize;}
      
      void setMaxSize(int maxSize) {maxSize_ = maxSize;}
      int getMaxSize() const {return maxSize_;}
      
      template <class InputIterator, class OutputIterator, class Functor>
      void execute(InputIterator first, InputIterator last, OutputIterator output, Functor f) const;
      
      inline size_t getStep(size_t blocks) const;
      
    private:
      template <typename Archive>
      void serialize(Archive & ar, const unsigned int version);
      
      int maxSize_;
    };
    
  }
}


template <typename Archive>
void fec::detail::WorkGroup::serialize(Archive & ar, const unsigned int version) {
  using namespace boost::serialization;
  ar & ::BOOST_SERIALIZATION_NVP(maxSize_);
}

size_t fec::detail::WorkGroup::getStep(size_t blockCount) const
{
  int n = std::thread::hardware_concurrency();
  if (maxSize_ <= 0 && (n > maxSize_ || n == 0)) {
    n = maxSize_;
  }
  return (blockCount+n-1)/n;
}

template <class InputIterator, class OutputIterator, class Functor>
void fec::detail::WorkGroup::execute(InputIterator first, InputIterator last, OutputIterator output, Functor f) const
{
  size_t blockCount = std::distance(first, last);
  size_t step = getStep(blockCount);
  std::vector<std::thread> group;
  for (int i = 0; i + step <= blockCount; i += step) {
    group.push_back( std::thread(f, first, first+step, output) );
    output += step; first += step;
  }
  if (first != last) {
    f(first, last, output);
  }
  for (auto& t : group) {
    t.join();
  }
}


#endif