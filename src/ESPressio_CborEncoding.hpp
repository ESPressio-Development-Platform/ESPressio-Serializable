#pragma once
#include <cstdint>
#include <string_view>
namespace ESPressio::Serializable::CborDetail {
    // Definite-length CBOR uses the same map/array/scalar representation as CborArchive.
    template<class Output> void Argument(Output& output, std::uint8_t major, std::uint64_t value) noexcept {
        if (value<24) { output.push_back(static_cast<std::uint8_t>((major<<5)|value)); return; }
        unsigned bytes=value<=0xff ? 1 : value<=0xffff ? 2 : value<=0xffffffffu ? 4 : 8;
        output.push_back(static_cast<std::uint8_t>((major<<5)|(bytes==1 ? 24 : bytes==2 ? 25 : bytes==4 ? 26 : 27)));
        for (unsigned i=bytes;i!=0;--i) output.push_back(static_cast<std::uint8_t>(value>>((i-1)*8)));
    }
    template<class Output> void Text(Output& output, std::string_view value) noexcept {
        Argument(output,3,value.size());
        output.insert(output.end(),value.begin(),value.end());
    }
}
