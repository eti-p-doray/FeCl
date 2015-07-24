/*******************************************************************************
 *  \file BpDecoder.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-06-16
 *  \version Last update : 2015-06-16
 *
 *  Declaration of BpDecoder class
 ******************************************************************************/

#ifndef BP_DECODER_H
#define BP_DECODER_H

#include <vector>
#include <memory>

#include "../../CodeStructure/LdpcCodeStructure.h"

namespace fec {

/*******************************************************************************
 *  This class represents a map decoder
 *  It offers methods to decode data giving a posteriori informations
 *  using a trellis.
 ******************************************************************************/
class DensityEvolutionBp
{
public:
  static std::unique_ptr<DensityEvolutionBp> create(const LdpcCodeStructure&);
  virtual ~DensityEvolutionBp() = default;
  
  void predictNBloc(boost::container::vector<LlrPdf>::const_iterator parityIn, boost::container::vector<LlrPdf>::iterator parityOut, boost::container::vector<LlrPdf>::iterator messageOut, size_t n);
  
protected:
  DensityEvolutionBp(const LdpcCodeStructure& codeStructure);
  
  void predictBloc(boost::container::vector<LlrPdf>::const_iterator parityIn, boost::container::vector<LlrPdf>::iterator parityOut, boost::container::vector<LlrPdf>::iterator messageOut);
  
  virtual void checkUpdate();
  virtual void bitUpdate(boost::container::vector<LlrPdf>::const_iterator parity);
  
  inline const LdpcCodeStructure& codeStructure() const {return codeStructure_;}
  
  boost::container::vector<LlrPdf> bitMetrics_;
  boost::container::vector<LlrPdf> checkMetrics_;
  boost::container::vector<LlrPdf> checkMetricsBuffer_;

private:
  
  const LdpcCodeStructure codeStructure_;
};
  
}

#endif
