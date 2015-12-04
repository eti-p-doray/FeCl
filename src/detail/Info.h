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

#ifndef FEC_DETAIL_INFO_H
#define FEC_DETAIL_INFO_H

#include <vector>

#include "../BitField.h"
#include "../DecoderAlgorithm.h"

namespace fec {
  
  /**
   *  detail namespace. You shouln't be much interested.
   */
  namespace detail {
  
    /**
     *  This class is an iterator on the codec data flow.
     */
    template <class Field, class Iterator>
    class InfoIterator {
    public:
      InfoIterator() = default;
      
      template <Field field> InfoIterator& add(Iterator it, size_t incr);
      template <Field field> Iterator get();
      template <Field field> bool has();
      
      inline void operator ++ ();
      inline void operator ++ (int);
      inline void operator -- ();
      inline void operator -- (int);
      inline void operator += (size_t x);
      inline void operator -= (size_t x);
      inline bool operator == (const InfoIterator& b);
      inline bool operator != (const InfoIterator& b);
      inline bool operator < (const InfoIterator& b);
      inline bool operator <= (const InfoIterator& b);
      inline bool operator > (const InfoIterator& b);
      inline bool operator >= (const InfoIterator& b);

      inline InfoIterator operator [] (size_t i);
      inline InfoIterator operator -> ();
      
    private:
    };
    
    template <class Field, class T>
    using InputIterator = InfoIterator<Field, std::vector<T>::const_iterator>;
    template <class Field, class T>
    using OutputIterator = InfoIterator<Field, std::vector<T>::iterator>;
    
    template <class Field, class Vector>
    class Info {
      using Iterator = InfoIterator<decltype(std::declval<Vector>().begin())>;
    public:
      Info() = default;
      
      template <Field field> InfoIterator& add(Iterator it);
      template <Field field> Iterator get();
      template <Field field> bool has();
      
      Iterator begin() const;
      Iterator end() const;
      
    private:
    };

    template <class Field, template <typename> class A = std::allocator>
    using Input = Info<Field, const std::vector<double,A<double>>>;
    template <class Field, template <typename> class A = std::allocator>
    using Output = Info<Field, std::vector<double,A<double>>>;
   fec::Codec::Input{}.add<fec::Parity>(llr); 
  }
  
}

#endif