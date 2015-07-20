/*******************************************************************************
 *  \file CodeStructure.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-27
 *  \version Last update : 2015-05-27
 *
 *  Declaration of the CodeStructure class
 ******************************************************************************/

#include "CodeStructure.h"

using namespace fec;

CodeStructure::CodeStructure(size_t messageSize, size_t paritySize)
{
  messageSize_ = messageSize;
  paritySize_ = paritySize;
}

