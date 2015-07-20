/*******************************************************************************
 *  \file MinSumBp.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-06-16
 *  \version Last update : 2015-06-17
 *
 *  Declaration of LogMapDecoder abstract class
 ******************************************************************************/

#ifndef MIN_SUM_BP_H
#define MIN_SUM_BP_H

#include <algorithm>
#include <cmath>

#include "BpDecoderImpl.h"
#include "../../CodeStructure/CodeStructure.h"

namespace fec {

/*******************************************************************************
 *  This class contains implementation specific functions of LogMap algorithm.
 ******************************************************************************/
class MinSumBp {
  friend class BpDecoderImpl<MinSumBp>;
public:
  
private:
  static inline void checkMetric(std::vector<LlrType>::iterator first, std::vector<LlrType>::iterator last, std::vector<LlrType>::iterator buffer) {
    bool sign = false;
    std::vector<LlrType>::iterator min[2];
    min[0] = first;
    for (auto metricIt = first; metricIt < last; ++metricIt) {
      sign ^= !std::signbit(*metricIt);
      if (fabs(*metricIt) < fabs(*min[0])) {
        min[0] = metricIt;
      }
    }
    min[1] = min[0] == first ? first+1 : first;
    for (auto metricIt = first; metricIt < last; ++metricIt) {
      if (fabs(*metricIt) < fabs(*min[1]) && metricIt != min[0]) {
        min[1] = metricIt;
      }
    }
    
    LlrType minValue[2] = {*min[0], *min[1]};
    for (auto metricIt = first; metricIt < last; ++metricIt) {
      LlrType currentMin = fabs(minValue[0]);
      if (metricIt == min[0]) {
        currentMin = fabs(minValue[1]);
      }
      if (sign ^ !std::signbit(*metricIt)) {
        *metricIt = currentMin;
      }
      else {
        *metricIt = -currentMin;
      }
    }
  }
};
  
}

#endif
