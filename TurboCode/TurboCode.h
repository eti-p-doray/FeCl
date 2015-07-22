/*******************************************************************************
 *  \file TurboCode.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-12
 *  \version Last update : 2015-05-27
 *
 *  Declaration of ConvolutionalCode class
 ******************************************************************************/

#ifndef TURBO_CODE_H
#define TURBO_CODE_H

#include <boost/serialization/export.hpp>
#include "../Archive.h"

#include "../Code.h"
#include "../CodeStructure/TurboCodeStructure.h"
#include "../ConvolutionalCode/ConvolutionalCode.h"
#include "TurboCodeImpl.h"

namespace fec {

/*******************************************************************************
 *  This class represents a map encode / decoder
 *  It offers methods encode and to decode data giving a posteriori informations 
 *  using a trellis.
 ******************************************************************************/
class TurboCode : public Code
{
  friend class boost::serialization::access;
public:
  TurboCode(const TurboCodeStructure& codeStructure, int workGroupdSize = 4);
  virtual ~TurboCode() = default;
  
  virtual const char * get_key() const;
  
  virtual size_t msgSize() const {return codeStructure_.msgSize();}
  virtual size_t paritySize() const {return codeStructure_.paritySize();}
  virtual size_t extrinsicSize() const {
    size_t extrinsicSize = 0;
    switch (codeStructure_.structureType()) {
      default:
      case TurboCodeStructure::Serial:
        return codeStructure_.msgSize();
        break;
        
      case TurboCodeStructure::Parallel:
        for (auto & i : codeStructure_.structures()) {
          extrinsicSize += i.msgSize();
        }
        return extrinsicSize;
        break;
    }
  }
  virtual const CodeStructure& structure() const {return codeStructure_;}
  
protected:
  TurboCode() = default;
  
  virtual void encodeBloc(std::vector<uint8_t>::const_iterator messageIt, std::vector<uint8_t>::iterator parityIt) const;
  
  //virtual void parityAppDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const;
  virtual void appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const;
  virtual void softOutDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut, size_t n) const;
  virtual void decodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<uint8_t>::iterator messageOut, size_t n) const;

private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    using namespace boost::serialization;
    ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(Code);
    ar & ::BOOST_SERIALIZATION_NVP(code_);
    ar & ::BOOST_SERIALIZATION_NVP(codeStructure_);
  }
  
  std::vector<ConvolutionalCode> code_;
  TurboCodeStructure codeStructure_;
};
  
}

BOOST_CLASS_EXPORT_KEY(fec::TurboCode);
BOOST_CLASS_TYPE_INFO(fec::TurboCode,extended_type_info_no_rtti<fec::TurboCode>);

#endif
