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
  
  void softOutDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut, size_t n);
  void appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n);
  void parityAppDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n);
  
protected:
  MapDecoder(const ConvolutionalCodeStructure& codeStructure);
  
  void softOutDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut);
  void appDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut);
  void parityAppDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut);
  
  virtual void appBranchMetrics(std::vector<LlrType>::const_iterator parity, std::vector<LlrType>::const_iterator extrinsic) = 0;
  virtual void parityAppBranchMetrics(std::vector<LlrType>::const_iterator parity, std::vector<LlrType>::const_iterator extrinsic) = 0;
  virtual void branchMetrics(std::vector<LlrType>::const_iterator parity) = 0;
  virtual void forwardMetrics() = 0;
  virtual void backwardMetrics() = 0;
  
  virtual void parityAPosteriori(std::vector<LlrType>::iterator parityOut) = 0;
  virtual void messageAPosteriori(std::vector<LlrType>::iterator messageOut) = 0;
  
  inline const ConvolutionalCodeStructure& codeStructure() const {return codeStructure_;}
  
  std::vector<LlrType> branchOutputMetrics_;
  std::vector<LlrType> branchInputMetrics_;
  
  std::vector<LlrType> branchMetrics_;
  std::vector<LlrType> forwardMetrics_;
  std::vector<LlrType> backwardMetrics_;

private:
  
  const ConvolutionalCodeStructure codeStructure_;
};

#endif
