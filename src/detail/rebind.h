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

#ifndef FEC_REBIND_H
#define FEC_REBIND_H


namespace fec {
  
  namespace detail {
    
    template <class Container, class NewType>
    struct rebind;
    
    template <class ValueType, class... Args, template <class...> class Container, class NewType>
    struct rebind<Container<ValueType, Args...>, NewType>
    {
      typedef Container<NewType, typename rebind<Args, NewType>::type...> type;
    };
    
  }
}

#endif