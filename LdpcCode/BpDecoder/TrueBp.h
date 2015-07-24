/*******************************************************************************
 *  \file TrueBp.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-06-16
 *  \version Last update : 2015-06-17
 *
 *  Declaration of TrueBp class
 ******************************************************************************/

#ifndef TRUE_BP_H
#define TRUE_BP_H

#include <algorithm>
#include <math.h>

#include "BpDecoderImpl.h"
#include "../../CodeStructure/CodeStructure.h"

namespace fec {

/*******************************************************************************
 *  This class contains implementation specific functions of TrueBp algorithm.
 ******************************************************************************/
class TrueBp {
  friend class BpDecoderImpl<TrueBp>;
  friend class DensityEvolutionBp;
public:
  
private:
  static inline void checkMetric(boost::container::vector<LlrType>::iterator first, boost::container::vector<LlrType>::iterator last, boost::container::vector<LlrType>::iterator buffer)
  {
    LlrType sum = *first;
    auto tmp = buffer + 1;
    for (auto metric = first+1; metric < last-1; ++metric, ++tmp) {
      *tmp = *metric;
      *metric = sum;
      sum = boxPlus(sum, *tmp);
    }
    std::swap(sum, *(last-1));
    --tmp;
    for (auto metric = last-2; metric > first; --metric, --tmp) {
      *metric = boxPlus(sum, *metric);
      sum = boxPlus(sum, *tmp);
    }
    *first = sum;
  }
  
  static inline void checkMetricImpl(boost::container::vector<LlrPdf>::iterator first, boost::container::vector<LlrPdf>::iterator last, boost::container::vector<LlrPdf>::iterator buffer)
  {
    LlrType sum = first->mean;
    auto tmp = buffer + 1;
    for (auto metric = first+1; metric < last-1; ++metric, ++tmp) {
      //*tmp = *metric;
      tmp->mean = sum;
      //*metric = sum;
      //sum = boxPlus(sum, *tmp);
      sum = boxPlus(sum, metric->mean);
    }
    std::swap(tmp->mean, (last-1)->mean);
    std::swap(sum, (last-1)->mean);
    --tmp;
    for (auto metric = last-2; metric > first; --metric, --tmp) {
      //*metric = boxPlus(sum, *metric);
      tmp->mean = boxPlus(sum, tmp->mean);
      //sum = boxPlus(sum, *tmp);
      sum = boxPlus(sum, metric->mean);
    }
    tmp->mean = sum;
  }
  
  static inline void checkMetric(boost::container::vector<LlrPdf>::iterator first, boost::container::vector<LlrPdf>::iterator last, boost::container::vector<LlrPdf>::iterator buffer)
  {
    LlrType weight = 0.0;
    
    checkMetricImpl(first, last, buffer);
    for (size_t i = 0; i < (last-first); ++i) {
      checkMetricImpl(first, last, buffer);
      checkMetricImpl(first, last, buffer);
    }
  }
     
  static inline LlrType boxPlus(LlrType a, LlrType b) {
    if (std::signbit(a) ^ std::signbit(b)) {
      return std::min(std::abs(a),std::abs(b)) - log((exp(-std::abs(a+b))+1)/(exp(-std::abs(a-b))+1));
    }
    else {
      return -std::min(std::abs(a),std::abs(b)) - log((exp(-std::abs(a+b))+1)/(exp(-std::abs(a-b))+1));
    }
  }
};
  
}

#endif