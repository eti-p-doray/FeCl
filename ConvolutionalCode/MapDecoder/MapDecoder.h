/*******************************************************************************
 *  \file MapDecoder.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-06-03
 *  \version Last update : 2015-06-03
 *
 *  Declaration of MapDecoder class
 ******************************************************************************/

#ifndef MAP_DECODER_H
#define MAP_DECODER_H

#include <vector>
#include <memory>

#include "../../CodeStructure/ConvolutionalCodeStructure.h"

namespace fec {

/*******************************************************************************
 *  This class represents a map decoder
 *  It offers methods to decode data giving a posteriori informations
 *  using a trellis.
 ******************************************************************************/
class MapDecoder
{
public:
  static std::unique_ptr<MapDecoder> create(const ConvolutionalCodeStructure&);
  virtual ~MapDecoder() = default;
  
  void softOutDecodeNBloc(boost::container::vector<LlrType>::const_iterator parityIn, boost::container::vector<LlrType>::iterator messageOut, size_t n);
  void appDecodeNBloc(boost::container::vector<LlrType>::const_iterator parityIn, boost::container::vector<LlrType>::const_iterator extrinsicIn, boost::container::vector<LlrType>::iterator messageOut, boost::container::vector<LlrType>::iterator extrinsicOut, size_t n);
  //void parityAppDecodeNBloc(boost::container::vector<LlrType>::const_iterator parityIn, boost::container::vector<LlrType>::const_iterator extrinsicIn, boost::container::vector<LlrType>::iterator messageOut, boost::container::vector<LlrType>::iterator extrinsicOut, size_t n);
  
  void softOutDecodeBloc(boost::container::vector<LlrType>::const_iterator parityIn, boost::container::vector<LlrType>::iterator messageOut);
  void appDecodeBloc(boost::container::vector<LlrType>::const_iterator parityIn, boost::container::vector<LlrType>::const_iterator extrinsicIn, boost::container::vector<LlrType>::iterator messageOut, boost::container::vector<LlrType>::iterator extrinsicOut);
  
protected:
  MapDecoder(const ConvolutionalCodeStructure& codeStructure);

  //void parityAppDecodeBloc(boost::container::vector<LlrType>::const_iterator parityIn, boost::container::vector<LlrType>::const_iterator extrinsicIn, boost::container::vector<LlrType>::iterator messageOut, boost::container::vector<LlrType>::iterator extrinsicOut);
  
  virtual void appBranchMetrics(boost::container::vector<LlrType>::const_iterator parity, boost::container::vector<LlrType>::const_iterator extrinsic) = 0;
  virtual void parityAppBranchMetrics(boost::container::vector<LlrType>::const_iterator parity, boost::container::vector<LlrType>::const_iterator extrinsic) = 0;
  virtual void branchMetrics(boost::container::vector<LlrType>::const_iterator parity) = 0;
  virtual void forwardMetrics() = 0;
  virtual void backwardMetrics() = 0;
  
  virtual void parityAPosteriori(boost::container::vector<LlrType>::iterator parityOut) = 0;
  virtual void messageAPosteriori(boost::container::vector<LlrType>::iterator messageOut) = 0;
  
  inline const ConvolutionalCodeStructure& codeStructure() const {return codeStructure_;}
  
  boost::container::vector<LlrType> branchOutputMetrics_;
  boost::container::vector<LlrType> branchInputMetrics_;
  
  boost::container::vector<LlrType> branchMetrics_;
  boost::container::vector<LlrType> forwardMetrics_;
  boost::container::vector<LlrType> backwardMetrics_;

private:
  
  const ConvolutionalCodeStructure codeStructure_;
};
  
}

#endif
