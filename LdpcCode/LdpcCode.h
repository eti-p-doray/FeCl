/*******************************************************************************
 *  \file LdpcCode.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-06-11
 *  \version Last update : 2015-06-11
 *
 *  Declaration of LdpcCode class
 ******************************************************************************/

#ifndef LDPC_CODE_H
#define LDPC_CODE_H

#include <thread>

#include <boost/serialization/export.hpp>
#include "../Archive.h"

#include "../ErrorCorrectingCode.h"
#include "../CodeStructure/LdpcCodeStructure.h"
#include "BpDecoder/BpDecoder.h"

/*******************************************************************************
 *  This class represents a map encode / decoder
 *  It offers methods encode and to decode data giving a posteriori informations 
 *  using a trellis.
 ******************************************************************************/
class LdpcCode : public ErrorCorrectingCode
{
  friend class boost::serialization::access;
public:
  LdpcCode(const LdpcCodeStructure& codeStructure, int workGroupdSize = 4);
  virtual ~LdpcCode() = default;
  
  virtual const char * get_key() const;
  
  virtual size_t msgSize() const {return codeStructure_.msgSize();}
  virtual size_t paritySize() const {return codeStructure_.paritySize();}
  virtual size_t extrinsicMsgSize() const {return codeStructure_.msgSize();}
  virtual size_t extrinsicParitySize() const {return codeStructure_.parityCheck().size();}
  virtual const CodeStructure& structure() const {return codeStructure_;}
  
  virtual void syndrome(const std::vector<uint8_t>& parity, std::vector<uint8_t>& syndrome) const;
  
protected:
  LdpcCode() = default;
  
  virtual void encodeBloc(std::vector<uint8_t>::const_iterator messageIt, std::vector<uint8_t>::iterator parityIt) const;
  virtual void parityAppDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const;
  virtual void appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const;
  virtual void softOutDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut, size_t n) const;
  virtual void decodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<uint8_t>::iterator messageOut, size_t n) const;
  
private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ErrorCorrectingCode);
    ar & BOOST_SERIALIZATION_NVP(codeStructure_);
  }
  
  LdpcCodeStructure codeStructure_;
};

BOOST_CLASS_EXPORT_KEY(LdpcCode);
BOOST_CLASS_TYPE_INFO(LdpcCode,extended_type_info_no_rtti<LdpcCode>);


#endif
