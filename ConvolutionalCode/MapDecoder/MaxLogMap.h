/*******************************************************************************
 *  \file MaxLogMap.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-27
 *  \version Last update : 2015-05-27
 *
 *  Declaration of MaxLogMapDecoder abstract class
 ******************************************************************************/

#ifndef MAX_LOG_MAP_H
#define MAX_LOG_MAP_H

#include <algorithm>
#include <math.h>

#include "MapDecoderImpl.h"
#include "../../CodeStructure/CodeStructure.h"

namespace fec {

/*******************************************************************************
 *  This class contains implementation specific functions of MaxLogMap algorithm.
 ******************************************************************************/
class MaxLogMap {
  friend class MapDecoderImpl<MaxLogMap>;
public:
  
private: 
  static inline LlrType logAdd(LlrType a, LlrType b) {return std::max(a,b);}
};
  
}

#endif
