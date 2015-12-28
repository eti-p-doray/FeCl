/*******************************************************************************
 This file is part of FeCl.
 
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 Copyright (c) 2015, Leszek Szczecinski, INRS
 All rights reserved.
 
 FeCl is free software: you can redistribute it and/or modify
 it under the terms of the Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 FeCl is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the Lesser General Public License
 along with FeCl.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef FEC_DETAIL_CODEC_FUNCTION_H
#define FEC_DETAIL_CODEC_FUNCTION_H

#include <vector>

#include "Codec.h"
#include "MapDecoder.h"
#include "ViterbiDecoder.h"
#include "BpDecoder.h"
#include "TurboDecoder.h"

namespace fec {
  
  /**
   *  detail namespace. You shouln't be much interested.
   */
  namespace detail {
    
    namespace Codec {

      template <class InputIterator, class OutputIterator>
      class EncodeFunction
      {
      public:
        static std::unique_ptr<EncodeFunction> create(const Codec::Structure&);
        virtual ~EncodeFunction() = default;
        
        void operator() (iterator<InputIterator> msgf, iterator<InputIterator> msgl, iterator<OutputIterator> parity);
        virtual void operator() (iterator<InputIterator> msgf, iterator<OutputIterator> parity) = 0;
      };
      
      template <class InputIterator, class OutputIterator>
      void EncodeFunction<InputIterator, OutputIterator>::operator() (iterator<InputIterator> msgf, iterator<InputIterator> msgl, iterator<OutputIterator> parity)
      {
        while (msgf != msgl) {
          (*this)(msgf++, parity++);
        }
      }
      
      template <class InputIterator>
      class CheckFunction
      {
      public:
        static std::unique_ptr<CheckFunction> create(const Codec::Structure&);
        virtual ~CheckFunction() = default;
        
        bool operator() (iterator<InputIterator> parityf, iterator<InputIterator> parityl);
        virtual bool operator() (iterator<InputIterator> parity) = 0;
      };
      
      template <class InputIterator>
      bool CheckFunction<InputIterator>::operator() (iterator<InputIterator> parityf, iterator<InputIterator> parityl)
      {
        while (parityf != parityl) {
          if (!(*this)(parityf++)) {
            return false;
          }
        }
        return true;
      }
      
      template <class InputIterator, class OutputIterator>
      class DecodeFunction
      {
      public:
        static std::unique_ptr<DecodeFunction> create(const Codec::Structure&);
        virtual ~DecodeFunction() = default;
        
        void operator() (iterator<InputIterator> parityf, iterator<InputIterator> parityl, iterator<OutputIterator> msg);
        virtual void operator() (iterator<InputIterator> parity, iterator<OutputIterator> msg) = 0;
      };
      
      template <class InputIterator, class OutputIterator>
      void DecodeFunction<InputIterator, OutputIterator>::operator() (iterator<InputIterator> parityf, iterator<InputIterator> parityl, iterator<OutputIterator> msg)
      {
        while (parityf != parityl) {
          (*this)(parityf++, msg++);
        }
      }
      
      template <class InputIterator, class OutputIterator>
      class SoDecodeFunction
      {
      public:
        static std::unique_ptr<SoDecodeFunction> create(const Codec::Structure&);
        virtual ~SoDecodeFunction() = default;
        
        void operator() (iterator<InputIterator> inf, iterator<InputIterator> inl, iterator<OutputIterator> out);
        virtual void operator() (iterator<InputIterator> in, iterator<OutputIterator> out) = 0;
      };
      
      template <class InputIterator, class OutputIterator>
      void SoDecodeFunction<InputIterator, OutputIterator>::operator() (iterator<InputIterator> inf, iterator<InputIterator> inl, iterator<OutputIterator> out)
      {
        while (inf != inl) {
          (*this)(inf++, out++);
        }
      }
      
    }
    
    namespace Convolutional {
      
      template <class InputIterator, class OutputIterator>
      class EncodeFunction : public Codec::EncodeFunction<InputIterator, OutputIterator>
      {
      public:
        static std::unique_ptr<EncodeFunction> create(const Structure& structure) {return std::unique_ptr<EncodeFunction>(new EncodeFunction(structure));}
        virtual ~EncodeFunction() = default;
        
        void operator() (Codec::iterator<InputIterator> msg, Codec::iterator<OutputIterator> parity) override {structure_.encode(msg.at(Codec::Msg), parity.at(Codec::Parity));}
        
      private:
        EncodeFunction(const Structure& structure) : structure_(structure) {}
        Structure structure_;
      };
      
      template <class InputIterator>
      class CheckFunction : public Codec::CheckFunction<InputIterator>
      {
      public:
        static std::unique_ptr<CheckFunction> create(const Structure& structure) {return std::unique_ptr<CheckFunction>(new CheckFunction(structure));}
        virtual ~CheckFunction() = default;
        
        bool operator() (Codec::iterator<InputIterator> parity) override {return structure_.check(parity.at(Codec::Parity));}
        
      private:
        CheckFunction(const Structure& structure) : structure_(structure) {}
        Structure structure_;
      };
      
      template <class InputIterator, class OutputIterator>
      class DecodeFunction : public Codec::DecodeFunction<InputIterator, OutputIterator>
      {
      public:
        static std::unique_ptr<DecodeFunction> create(const Structure& structure) {return std::unique_ptr<DecodeFunction>(new DecodeFunction(structure));}
        virtual ~DecodeFunction() = default;
        
        void operator() (Codec::iterator<InputIterator> parity, Codec::iterator<OutputIterator> msg) override {decoder_.decode(parity.at(Codec::Parity), msg.at(Codec::Msg));}
        
      private:
        DecodeFunction(const Structure&  structure) : decoder_(structure) {}
        ViterbiDecoder<typename InputIterator::value_type> decoder_;
      };
      
      template <class InputIterator, class OutputIterator>
      class SoDecodeFunction : public Codec::SoDecodeFunction<InputIterator, OutputIterator>
      {
      public:
        static std::unique_ptr<SoDecodeFunction> create(const Structure&);
        virtual ~SoDecodeFunction() = default;
      };
      
      template <class InputIterator, class OutputIterator, template <class> class Algorithm>
      class SoDecodeFunctionImpl : public SoDecodeFunction<InputIterator, OutputIterator>
      {
      public:
        static std::unique_ptr<SoDecodeFunctionImpl> create(const Structure& structure) {return std::unique_ptr<SoDecodeFunctionImpl>(new SoDecodeFunctionImpl(structure));}
        virtual ~SoDecodeFunctionImpl() = default;
        
        void operator() (Codec::iterator<InputIterator> in, Codec::iterator<OutputIterator> out) override {decoder_.soDecode(in, out);}
        
      private:
        SoDecodeFunctionImpl(const Structure& structure) : decoder_(structure) {}
        MapDecoder<typename InputIterator::value_type, Algorithm> decoder_;
      };
      
    }
    
    namespace Turbo {
      
      template <class InputIterator, class OutputIterator>
      class EncodeFunction : public Codec::EncodeFunction<InputIterator, OutputIterator>
      {
      public:
        static std::unique_ptr<EncodeFunction> create(const Structure& structure) {return std::unique_ptr<EncodeFunction>(new EncodeFunction(structure));}
        virtual ~EncodeFunction() = default;
        
        void operator() (Codec::iterator<InputIterator> msg, Codec::iterator<OutputIterator> parity) override {structure_.encode(msg.at(Codec::Msg), parity.at(Codec::Parity));}
        
      private:
        EncodeFunction(const Structure& structure) : structure_(structure) {}
        Structure structure_;
      };
      
      template <class InputIterator>
      class CheckFunction : public Codec::CheckFunction<InputIterator>
      {
      public:
        static std::unique_ptr<CheckFunction> create(const Structure& structure) {return std::unique_ptr<CheckFunction>(new CheckFunction(structure));}
        virtual ~CheckFunction() = default;
        
        bool operator() (Codec::iterator<InputIterator> parity) override {return structure_.check(parity.at(Codec::Parity));}
        
      private:
        CheckFunction(const Structure& structure) : structure_(structure) {}
        Structure structure_;
      };
      
      template <class InputIterator, class OutputIterator>
      class DecodeFunction : public Codec::DecodeFunction<InputIterator, OutputIterator>
      {
      public:
        static std::unique_ptr<DecodeFunction> create(const Structure&);
        virtual ~DecodeFunction() = default;
      };
      
      template <class InputIterator, class OutputIterator>
      class SoDecodeFunction : public Codec::SoDecodeFunction<InputIterator, OutputIterator>
      {
      public:
        static std::unique_ptr<SoDecodeFunction> create(const Structure&);
        virtual ~SoDecodeFunction() = default;
      };
      
      template <class InputIterator, class OutputIterator, template <class> class Algorithm>
      class DecodeFunctionImpl : public DecodeFunction<InputIterator, OutputIterator>
      {
      public:
        static std::unique_ptr<DecodeFunctionImpl> create(const Structure& structure) {return std::unique_ptr<DecodeFunctionImpl>(new DecodeFunctionImpl(structure));}
        virtual ~DecodeFunctionImpl() = default;
        
        void operator() (Codec::iterator<InputIterator> parity, Codec::iterator<OutputIterator> msg) override {decoder_.decode(parity.at(Codec::Parity), msg.at(Codec::Msg));}
        
      private:
        DecodeFunctionImpl(const Structure& structure) : decoder_(structure) {}
        TurboDecoder<typename InputIterator::value_type, Algorithm> decoder_;
      };
      
      template <class InputIterator, class OutputIterator, template <class> class Algorithm>
      class SoDecodeFunctionImpl : public SoDecodeFunction<InputIterator, OutputIterator>
      {
      public:
        static std::unique_ptr<SoDecodeFunctionImpl> create(const Structure& structure) {return std::unique_ptr<SoDecodeFunctionImpl>(new SoDecodeFunctionImpl(structure));}
        virtual ~SoDecodeFunctionImpl() = default;
        
        void operator() (Codec::iterator<InputIterator> in, Codec::iterator<OutputIterator> out) override {decoder_.soDecode(in, out);}
        
      private:
        SoDecodeFunctionImpl(const Structure& structure) : decoder_(structure) {}
        TurboDecoder<typename InputIterator::value_type, Algorithm> decoder_;
      };
      
    }
    
    namespace Ldpc {
      
      template <class InputIterator, class OutputIterator>
      class EncodeFunction : public Codec::EncodeFunction<InputIterator, OutputIterator>
      {
      public:
        static std::unique_ptr<EncodeFunction> create(const Structure& structure) {return std::unique_ptr<EncodeFunction>(new EncodeFunction(structure));}
        virtual ~EncodeFunction() = default;
        
        void operator() (Codec::iterator<InputIterator> msg, Codec::iterator<OutputIterator> parity) override {structure_.encode(msg.at(Codec::Msg), parity.at(Codec::Parity));}
        
      private:
        EncodeFunction(const Structure& structure) : structure_(structure) {}
        Structure structure_;
      };
      
      template <class InputIterator>
      class CheckFunction : public Codec::CheckFunction<InputIterator>
      {
      public:
        static std::unique_ptr<CheckFunction> create(const Structure& structure) {return std::unique_ptr<CheckFunction>(new CheckFunction(structure));}
        virtual ~CheckFunction() = default;
        
        bool operator() (Codec::iterator<InputIterator> parity) override {return structure_.check(parity.at(Codec::Parity));}
        
      private:
        CheckFunction(const Structure& structure) : structure_(structure) {}
        Structure structure_;
      };
      
      template <class InputIterator, class OutputIterator>
      class DecodeFunction : public Codec::DecodeFunction<InputIterator, OutputIterator>
      {
      public:
        static std::unique_ptr<DecodeFunction> create(const Structure&);
        virtual ~DecodeFunction() = default;
      };
      
      template <class InputIterator, class OutputIterator>
      class SoDecodeFunction : public Codec::SoDecodeFunction<InputIterator, OutputIterator>
      {
      public:
        static std::unique_ptr<SoDecodeFunction> create(const Structure&);
        virtual ~SoDecodeFunction() = default;
      };
      
      template <class InputIterator, class OutputIterator, template <class> class Algorithm>
      class DecodeFunctionImpl : public DecodeFunction<InputIterator, OutputIterator>
      {
      public:
        static std::unique_ptr<DecodeFunctionImpl> create(const Structure& structure) {return std::unique_ptr<DecodeFunctionImpl>(new DecodeFunctionImpl(structure));}
        virtual ~DecodeFunctionImpl() = default;
        
        void operator() (Codec::iterator<InputIterator> parity, Codec::iterator<OutputIterator> msg) override {decoder_.decode(parity.at(Codec::Parity), msg.at(Codec::Msg));}
        
      private:
        DecodeFunctionImpl(const Structure& structure) : decoder_(structure) {}
        BpDecoder<typename InputIterator::value_type, Algorithm> decoder_;
      };
      
      template <class InputIterator, class OutputIterator, template <class> class Algorithm>
      class SoDecodeFunctionImpl : public SoDecodeFunction<InputIterator, OutputIterator>
      {
      public:
        static std::unique_ptr<SoDecodeFunctionImpl> create(const Structure& structure) {return std::unique_ptr<SoDecodeFunctionImpl>(new SoDecodeFunctionImpl(structure));}
        virtual ~SoDecodeFunctionImpl() = default;
        
        void operator() (Codec::iterator<InputIterator> in, Codec::iterator<OutputIterator> out) override {decoder_.soDecode(in, out);}
        
      private:
        SoDecodeFunctionImpl(const Structure& structure) : decoder_(structure) {}
        BpDecoder<typename InputIterator::value_type, Algorithm> decoder_;
      };
      
    }
    
    template <class InputIterator, class OutputIterator>
    std::unique_ptr<Codec::EncodeFunction<InputIterator,OutputIterator>> Codec::EncodeFunction<InputIterator,OutputIterator>::create(const Codec::Structure& structure)
    {
      if (dynamic_cast<const Convolutional::Structure*>(&structure) != nullptr) {
        return Convolutional::EncodeFunction<InputIterator,OutputIterator>::create(dynamic_cast<const Convolutional::Structure&>(structure));
      } else if (dynamic_cast<const Turbo::Structure*>(&structure) != nullptr) {
        return Turbo::EncodeFunction<InputIterator,OutputIterator>::create(dynamic_cast<const Turbo::Structure&>(structure));
      } else if (dynamic_cast<const Ldpc::Structure*>(&structure) != nullptr) {
        return Ldpc::EncodeFunction<InputIterator,OutputIterator>::create(dynamic_cast<const Ldpc::Structure&>(structure));
      }
      return {};
    }
    
    template <class InputIterator>
    std::unique_ptr<Codec::CheckFunction<InputIterator>> Codec::CheckFunction<InputIterator>::create(const Codec::Structure& structure)
    {
      if (dynamic_cast<const Convolutional::Structure*>(&structure) != nullptr) {
        return Convolutional::CheckFunction<InputIterator>::create(dynamic_cast<const Convolutional::Structure&>(structure));
      } else if (dynamic_cast<const Turbo::Structure*>(&structure) != nullptr) {
        return Turbo::CheckFunction<InputIterator>::create(dynamic_cast<const Turbo::Structure&>(structure));
      } else if (dynamic_cast<const Ldpc::Structure*>(&structure) != nullptr) {
        return Ldpc::CheckFunction<InputIterator>::create(dynamic_cast<const Ldpc::Structure&>(structure));
      }
      return {};
    }
    
    template <class InputIterator, class OutputIterator>
    std::unique_ptr<Codec::DecodeFunction<InputIterator,OutputIterator>> Codec::DecodeFunction<InputIterator,OutputIterator>::create(const Codec::Structure& structure)
    {
      if (dynamic_cast<const Convolutional::Structure*>(&structure) != nullptr) {
        return Convolutional::DecodeFunction<InputIterator,OutputIterator>::create(dynamic_cast<const Convolutional::Structure&>(structure));
      } else if (dynamic_cast<const Turbo::Structure*>(&structure) != nullptr) {
        return Turbo::DecodeFunction<InputIterator,OutputIterator>::create(dynamic_cast<const Turbo::Structure&>(structure));
      } else if (dynamic_cast<const Ldpc::Structure*>(&structure) != nullptr) {
        return Ldpc::DecodeFunction<InputIterator,OutputIterator>::create(dynamic_cast<const Ldpc::Structure&>(structure));
      }
      return {};
    }
    
    template <class InputIterator, class OutputIterator>
    std::unique_ptr<Turbo::DecodeFunction<InputIterator,OutputIterator>> Turbo::DecodeFunction<InputIterator,OutputIterator>::create(const Turbo::Structure& structure)
    {
      switch (structure.decoderAlgorithm()) {
        default:
        case Exact:
          return DecodeFunctionImpl<InputIterator,OutputIterator,LogSum>::create(structure);
          
        case Linear:
          return DecodeFunctionImpl<InputIterator,OutputIterator,LinearLogSum>::create(structure);
          
        case Approximate:
          return DecodeFunctionImpl<InputIterator,OutputIterator,MaxLogSum>::create(structure);
      }
    }
    
    template <class InputIterator, class OutputIterator>
    std::unique_ptr<Ldpc::DecodeFunction<InputIterator,OutputIterator>> Ldpc::DecodeFunction<InputIterator,OutputIterator>::create(const Ldpc::Structure& structure)
    {
      switch (structure.decoderAlgorithm()) {
        default:
        case Exact:
          return DecodeFunctionImpl<InputIterator,OutputIterator,BoxSum>::create(structure);
          
        case Linear:
          return DecodeFunctionImpl<InputIterator,OutputIterator,LinearBoxSum>::create(structure);
          
        case Approximate:
          return DecodeFunctionImpl<InputIterator,OutputIterator,MinBoxSum>::create(structure);
      }
    }
    
    template <class InputIterator, class OutputIterator>
    std::unique_ptr<Codec::SoDecodeFunction<InputIterator,OutputIterator>> Codec::SoDecodeFunction<InputIterator,OutputIterator>::create(const Codec::Structure& structure)
    {
      if (dynamic_cast<const Convolutional::Structure*>(&structure) != nullptr) {
        return Convolutional::SoDecodeFunction<InputIterator,OutputIterator>::create(dynamic_cast<const Convolutional::Structure&>(structure));
      } else if (dynamic_cast<const Turbo::Structure*>(&structure) != nullptr) {
        return Turbo::SoDecodeFunction<InputIterator,OutputIterator>::create(dynamic_cast<const Turbo::Structure&>(structure));
      } else if (dynamic_cast<const Ldpc::Structure*>(&structure) != nullptr) {
        return Ldpc::SoDecodeFunction<InputIterator,OutputIterator>::create(dynamic_cast<const Ldpc::Structure&>(structure));
      }
      return {};
    }
    
    template <class InputIterator, class OutputIterator>
    std::unique_ptr<Convolutional::SoDecodeFunction<InputIterator,OutputIterator>> Convolutional::SoDecodeFunction<InputIterator,OutputIterator>::create(const Convolutional::Structure& structure)
    {
      switch (structure.decoderAlgorithm()) {
        default:
        case Exact:
          return SoDecodeFunctionImpl<InputIterator,OutputIterator,LogSum>::create(structure);
          
        case Linear:
          return SoDecodeFunctionImpl<InputIterator,OutputIterator,LinearLogSum>::create(structure);
          
        case Approximate:
          return SoDecodeFunctionImpl<InputIterator,OutputIterator,MaxLogSum>::create(structure);
      }
    }
    
    template <class InputIterator, class OutputIterator>
    std::unique_ptr<Turbo::SoDecodeFunction<InputIterator,OutputIterator>> Turbo::SoDecodeFunction<InputIterator,OutputIterator>::create(const Turbo::Structure& structure)
    {
      switch (structure.decoderAlgorithm()) {
        default:
        case Exact:
          return SoDecodeFunctionImpl<InputIterator,OutputIterator,LogSum>::create(structure);
          
        case Linear:
          return SoDecodeFunctionImpl<InputIterator,OutputIterator,LinearLogSum>::create(structure);
          
        case Approximate:
          return SoDecodeFunctionImpl<InputIterator,OutputIterator,MaxLogSum>::create(structure);
      }
    }
    
    template <class InputIterator, class OutputIterator>
    std::unique_ptr<Ldpc::SoDecodeFunction<InputIterator,OutputIterator>> Ldpc::SoDecodeFunction<InputIterator,OutputIterator>::create(const Ldpc::Structure& structure)
    {
      switch (structure.decoderAlgorithm()) {
        default:
        case Exact:
          return SoDecodeFunctionImpl<InputIterator,OutputIterator,BoxSum>::create(structure);
          
        case Linear:
          return SoDecodeFunctionImpl<InputIterator,OutputIterator,LinearBoxSum>::create(structure);
          
        case Approximate:
          return SoDecodeFunctionImpl<InputIterator,OutputIterator,MinBoxSum>::create(structure);
      }
    }
    
  }
  
}

#endif