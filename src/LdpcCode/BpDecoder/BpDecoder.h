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
  
  void parityAppDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator parityOut, std::vector<LlrType>::iterator extrinsicOut, size_t n);
  void appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n);
  void softOutDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut, size_t n);
  
protected:
  BpDecoder(const LdpcCodeStructure& codeStructure);
  
  void parityAppDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator parityOut, std::vector<LlrType>::iterator extrinsicOut);
  void appDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut);
  void softOutDecodeBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut);
  
  virtual void checkUpdate() = 0;
  virtual void bitUpdate(std::vector<LlrType>::const_iterator parity) = 0;
  
  inline const LdpcCodeStructure& codeStructure() const {return codeStructure_;}
 
  std::vector<uint8_t> hardParity_;
  
  std::vector<LlrType> bitMetrics_;
  std::vector<LlrType> checkMetrics_;
  std::vector<LlrType> checkMetricsBuffer_;

private:
  
  const LdpcCodeStructure codeStructure_;
};

#endif
