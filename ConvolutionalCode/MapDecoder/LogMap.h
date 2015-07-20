/*******************************************************************************
 *  \file LogMap.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-28
 *  \version Last update : 2015-05-28
 *
 *  Declaration of LogMapDecoder abstract class
 ******************************************************************************/

#ifndef LOG_MAP_H
#define LOG_MAP_H

#include <algorithm>
#include <cmath>

#include "MapDecoderImpl.h"
#include "../../CodeStructure/CodeStructure.h"

namespace fec {

/*******************************************************************************
 *  This class contains implementation specific functions of LogMap algorithm.
 ******************************************************************************/
class LogMap {
  friend class MapDecoderImpl<LogMap>;
public:
  
private:
  static inline LlrType logAdd(LlrType a, LlrType b) {
    LlrType sum = std::max(a,b);
    if (sum == -MAX_LLR) {
      return sum;
    }
    return sum + log(1.0 + exp(-fabs(a-b)));
  }
};
  
}

#endif
