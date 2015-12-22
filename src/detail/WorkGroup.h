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
#include <future>

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
      
      template <class Function, class InputIterator, class OutputIterator>
      void executeTask(Function&& f, InputIterator&& first, InputIterator&& last, OutputIterator&& output);
      template <class Function, class InputIterator, class OutputIterator>
      void addTask(Function&& f, InputIterator&& first, InputIterator&& last, OutputIterator&& output);
      
      inline void wait();
      
      inline size_t getStep(size_t blocks) const;
      
    private:
      template <typename Archive>
      void serialize(Archive & ar, const unsigned int version);
      
      std::vector<std::future<void>> group_;
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
  if (maxSize_ > 0 && (n > maxSize_ || n == 0)) {
    n = maxSize_;
  }
  return (blockCount+n-1)/n;
}

template <class Function, class InputIterator, class OutputIterator>
void fec::detail::WorkGroup::executeTask(Function&& f, InputIterator&& first, InputIterator&& last, OutputIterator&& output)
{
  addTask(std::forward<Function>(f), std::forward<InputIterator>(first), std::forward<InputIterator>(last), std::forward<OutputIterator>(output));
  wait();
}

template <class Function, class InputIterator, class OutputIterator>
void fec::detail::WorkGroup::addTask(Function&& f, InputIterator&& first, InputIterator&& last, OutputIterator&& output)
{
  size_t blockCount = std::distance(first, last);
  size_t step = getStep(blockCount);
  for (int i = 0; i + step <= blockCount; i += step) {
    group_.push_back(std::async(std::forward<Function>(f), first, first+step, output));
    output += step; first += step;
  }
  f(first, last, output);
}

void fec::detail::WorkGroup::wait() {
  for (auto& t : group_) {
    t.wait();
  }
  group_.clear();
}


#endif