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
class BpDecoder
{
public:
  static std::unique_ptr<BpDecoder> create(const LdpcCodeStructure&);
  virtual ~BpDecoder() = default;
  
  //void parityAppDecodeNBloc(boost::container::vector<LlrType>::const_iterator parityIn, boost::container::vector<LlrType>::const_iterator extrinsicIn, boost::container::vector<LlrType>::iterator messageOut, boost::container::vector<LlrType>::iterator extrinsicOut, size_t n);
  void appDecodeNBloc(boost::container::vector<LlrType>::const_iterator parityIn, boost::container::vector<LlrType>::const_iterator extrinsicIn, boost::container::vector<LlrType>::iterator messageOut, boost::container::vector<LlrType>::iterator extrinsicOut, size_t n);
  void softOutDecodeNBloc(boost::container::vector<LlrType>::const_iterator parityIn, boost::container::vector<LlrType>::iterator messageOut, size_t n);
  
protected:
  BpDecoder(const LdpcCodeStructure& codeStructure);
  
  //void parityAppDecodeBloc(boost::container::vector<LlrType>::const_iterator parityIn, boost::container::vector<LlrType>::const_iterator extrinsicIn, boost::container::vector<LlrType>::iterator messageOut, boost::container::vector<LlrType>::iterator extrinsicOut);
  void appDecodeBloc(boost::container::vector<LlrType>::const_iterator parityIn, boost::container::vector<LlrType>::const_iterator extrinsicIn, boost::container::vector<LlrType>::iterator messageOut, boost::container::vector<LlrType>::iterator extrinsicOut);
  void softOutDecodeBloc(boost::container::vector<LlrType>::const_iterator parityIn, boost::container::vector<LlrType>::iterator messageOut);
  
  virtual void checkUpdate() = 0;
  virtual void bitUpdate(boost::container::vector<LlrType>::const_iterator parity) = 0;
  
  inline const LdpcCodeStructure& codeStructure() const {return codeStructure_;}
 
  boost::container::vector<uint8_t> hardParity_;
  
  boost::container::vector<LlrType> bitMetrics_;
  boost::container::vector<LlrType> checkMetrics_;
  boost::container::vector<LlrType> checkMetricsBuffer_;

private:
  
  const LdpcCodeStructure codeStructure_;
};
  
}

#endif
