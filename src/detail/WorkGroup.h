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
      template <typename Functor, typename Input, typename Output>
      void executeTask(Functor f, Input begin, Input end, in, Output out);
      
    private:
      template <typename Archive>
      void serialize(Archive & ar, const unsigned int version);
      
      std::vector<std::thread> createWorkGroup() const;
      size_t taskSize(size_t blockCount) const;
      
      int workGroupSize_;
    };
    
  }
}


template <typename Archive>
void fec::WorkGroup::serialize(Archive & ar, const unsigned int version) {
  using namespace boost::serialization;
  ar & ::BOOST_SERIALIZATION_NVP(structure_);
}

template <typename Functor, typename Input, typename Output>
void fec::WorkGroup::executeTask(Functor f, Input begin, Input end, Output out)
{
  size_t blockCount = end - begin;
  
  auto threadGroup = createWorkGroup();
  auto thread = threadGroup.begin();
  size_t step = taskSize(blockCount);
  for (int i = 0; i + step <= blockCount; i += step) {
    threadGroup.push_back( std::thread(&f, begin, out, step) );
    ++begin;
    ++out;
    ++thread;
  }
  if (msgOutIt != msg.end()) {
    decodeBlocks(begin, out, blockCount % step);
  }
  for (auto & thread : threadGroup) {
    thread.join();
  }
}

#endif