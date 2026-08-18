#pragma once
#if !__has_include(<Arduino.h>)
#error "ESPressio_CborStreamDeserializer.hpp requires Arduino Stream."
#endif
#include <Arduino.h>
#include <cstring>
#include "ESPressio_Serializable.hpp"
namespace ESPressio::Serializable {
class CborStreamDeserializer {
    Stream& _in;
    size_t _maxDepth;
    bool byte(uint8_t& b){ int c=_in.read(); if(c<0) return false; b=(uint8_t)c; return true; }
    bool bytes(uint8_t* out,size_t n){ for(size_t i=0;i<n;++i) if(!byte(out[i])) return false; return true; }
    bool argument(uint8_t ai,uint64_t& v){ if(ai<24){v=ai;return true;} size_t n=ai==24?1:ai==25?2:ai==26?4:ai==27?8:0; if(!n)return false;v=0;for(size_t i=0;i<n;++i){uint8_t b;if(!byte(b))return false;v=(v<<8)|b;}return true; }
    bool node(SerializationNode& out,size_t depth){ if(depth>_maxDepth)return false; uint8_t initial;if(!byte(initial))return false;uint8_t major=initial>>5, ai=initial&31;
        if(major==7){ if(ai==20||ai==21){out.SetType(SerializationNodeType::Boolean);out.BooleanValue()=ai==21;return true;} if(ai==22){out.SetType(SerializationNodeType::Null);return true;} if(ai==26){uint8_t b[4];if(!bytes(b,4))return false;uint32_t raw=((uint32_t)b[0]<<24)|((uint32_t)b[1]<<16)|((uint32_t)b[2]<<8)|b[3];float f;memcpy(&f,&raw,4);out.SetType(SerializationNodeType::Float32);out.Float32Value()=f;return true;} if(ai==27){uint8_t b[8];if(!bytes(b,8))return false;uint64_t raw=0;for(auto x:b)raw=(raw<<8)|x;double d;memcpy(&d,&raw,8);out.SetType(SerializationNodeType::Float64);out.Float64Value()=d;return true;} return false; }
        uint64_t n;if(!argument(ai,n))return false;
        if(major==0){out.SetType(SerializationNodeType::UnsignedInteger);out.UnsignedIntegerValue()=n;return true;}
        if(major==1){if(n>(uint64_t)INT64_MAX)return false;out.SetType(SerializationNodeType::SignedInteger);out.SignedIntegerValue()=-1-(int64_t)n;return true;}
        if(major==3){out.SetType(SerializationNodeType::String);out.StringValue().resize((size_t)n);return n==0||bytes((uint8_t*)out.StringValue().data(),(size_t)n);}
        if(major==4){out.SetType(SerializationNodeType::Array);out.ReserveArray((size_t)n);for(uint64_t i=0;i<n;++i){SerializationNode c;if(!node(c,depth+1))return false;out.Append(std::move(c));}return true;}
        if(major==5){out.SetType(SerializationNodeType::Object);out.ReserveObject((size_t)n);for(uint64_t i=0;i<n;++i){SerializationNode k,v;if(!node(k,depth+1)||k.GetType()!=SerializationNodeType::String||!node(v,depth+1))return false;out.Set(k.StringValue().c_str(),std::move(v));}return true;}
        return false;
    }
public:
    explicit CborStreamDeserializer(Stream& input,size_t maximumDepth=64):_in(input),_maxDepth(maximumDepth){}
    template<class T> DeserializationResult Deserialize(T& object){ SerializationNode root; DeserializationResult r; if(!node(root,0)||root.GetType()!=SerializationNodeType::Object){r.Add(SerializationErrorCode::MalformedInput,"","Malformed or truncated CBOR stream");return r;} TreeArchive archive; archive.GetNode()=std::move(root); return object.DeserializeDetailed(archive); }
};
}
