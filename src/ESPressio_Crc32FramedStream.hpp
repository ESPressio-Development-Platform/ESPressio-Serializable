#pragma once
#if !__has_include(<Arduino.h>)
#error "ESPressio_Crc32FramedStream.hpp requires Arduino Stream."
#endif
#include <Arduino.h>
#include <cstdint>
#include <vector>
namespace ESPressio::Serializable {
/// <summary>Reads and writes length-delimited stream frames protected by a CRC-32 payload checksum.</summary>
class Crc32FramedStream {
 static uint32_t Crc(const uint8_t*d,size_t n){uint32_t c=0xFFFFFFFFu;for(size_t i=0;i<n;++i){c^=d[i];for(int b=0;b<8;++b)c=(c>>1)^(0xEDB88320u & (0u-(c&1u)));}return ~c;}
 static bool ReadExact(Stream&s,uint8_t*d,size_t n,uint32_t timeout){size_t p=0;uint32_t start=millis();while(p<n){while(s.available()&&p<n){int v=s.read();if(v>=0)d[p++]=uint8_t(v);}if(uint32_t(millis()-start)>=timeout)return false;yield();}return true;}
public:
 /// <summary>Writes one ESPF frame containing payload length, CRC-32, and payload bytes.</summary>
 /// <returns><c>true</c> when the complete frame was accepted by the output.</returns>
 static bool Write(Print&out,const uint8_t*data,size_t size){uint8_t h[12]={'E','S','P','F',uint8_t(size),uint8_t(size>>8),uint8_t(size>>16),uint8_t(size>>24),0,0,0,0};uint32_t c=Crc(data,size);h[8]=uint8_t(c);h[9]=uint8_t(c>>8);h[10]=uint8_t(c>>16);h[11]=uint8_t(c>>24);return out.write(h,sizeof(h))==sizeof(h) && (size==0||out.write(data,size)==size);}
 /// <summary>Reads one ESPF frame, validates its CRC-32, and returns the payload bytes.</summary>
 /// <returns><c>true</c> when a complete valid frame is received before the timeout.</returns>
 static bool Read(Stream&in,std::vector<uint8_t>&data,uint32_t timeout=1000){uint8_t h[12];if(!ReadExact(in,h,sizeof(h),timeout)||h[0]!='E'||h[1]!='S'||h[2]!='P'||h[3]!='F')return false;uint32_t n=uint32_t(h[4])|(uint32_t(h[5])<<8)|(uint32_t(h[6])<<16)|(uint32_t(h[7])<<24);uint32_t expected=uint32_t(h[8])|(uint32_t(h[9])<<8)|(uint32_t(h[10])<<16)|(uint32_t(h[11])<<24);data.resize(n);if(n&&!ReadExact(in,data.data(),n,timeout))return false;return Crc(data.data(),data.size())==expected;}
};
}
