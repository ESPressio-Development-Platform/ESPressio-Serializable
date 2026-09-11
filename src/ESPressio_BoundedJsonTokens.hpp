#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace ESPressio::Serializable::JsonDetail {
    /// <summary>Skips only JSON's four whitespace characters inside the already bounded input.</summary>
    inline void Space(const std::uint8_t*& p, const std::uint8_t* end) noexcept {
        while (p<end && (*p==' ' || *p=='\t' || *p=='\r' || *p=='\n')) ++p;
    }
    inline int Hex(std::uint8_t value) noexcept {
        if (value>='0' && value<='9') return value-'0';
        if (value>='a' && value<='f') return value-'a'+10;
        if (value>='A' && value<='F') return value-'A'+10;
        return -1;
    }
    /// <summary>Decodes one validated JSON string span to a bounded consumer; never allocates a decoded string.</summary>
    template<class Emit> bool DecodeString(std::string_view raw, Emit emit) noexcept {
        for (std::size_t i=0;i<raw.size();++i) {
            auto c=static_cast<std::uint8_t>(raw[i]);
            if (c<0x20 || c=='"') return false;
            if (c!='\\') {
                if (c<0x80) { if (!emit(c)) return false; continue; }
                // Validate raw UTF-8 before exposing any byte of the code point.
                unsigned n=c>=0xc2 && c<=0xdf ? 2 : c>=0xe0 && c<=0xef ? 3 : c>=0xf0 && c<=0xf4 ? 4 : 0;
                if (!n || raw.size()-i<n) return false;
                std::uint32_t point=c&((1u<<(7-n))-1);
                for (unsigned j=1;j<n;++j) {
                    auto next=static_cast<std::uint8_t>(raw[i+j]);
                    if ((next&0xc0)!=0x80) return false;
                    point=(point<<6)|(next&0x3f);
                }
                if ((n==3 && point<0x800) || (n==4 && point<0x10000) || point>0x10ffff || (point>=0xd800 && point<=0xdfff)) return false;
                for (unsigned j=0;j<n;++j) if (!emit(static_cast<std::uint8_t>(raw[i+j]))) return false;
                i+=n-1; continue;
            }
            if (++i==raw.size()) return false;
            switch (raw[i]) {
                case '"': case '\\': case '/': if (!emit(static_cast<std::uint8_t>(raw[i]))) return false; break;
                case 'b': if (!emit('\b')) return false; break;
                case 'f': if (!emit('\f')) return false; break;
                case 'n': if (!emit('\n')) return false; break;
                case 'r': if (!emit('\r')) return false; break;
                case 't': if (!emit('\t')) return false; break;
                case 'u': {
                    auto readUnit=[&](std::uint32_t& unit) {
                        if (raw.size()-i<=4) return false;
                        unit=0;
                        for (unsigned j=0;j<4;++j) { auto h=Hex(static_cast<std::uint8_t>(raw[++i])); if (h<0) return false; unit=(unit<<4)|unsigned(h); }
                        return true;
                    };
                    std::uint32_t point{};
                    if (!readUnit(point)) return false;
                    if (point>=0xd800 && point<=0xdbff) {
                        if (raw.size()-i<=2 || raw[i+1]!='\\' || raw[i+2]!='u') return false;
                        i+=2; std::uint32_t low{};
                        if (!readUnit(low) || low<0xdc00 || low>0xdfff) return false;
                        point=0x10000+((point-0xd800)<<10)+(low-0xdc00);
                    } else if (point>=0xdc00 && point<=0xdfff) return false;
                    if (point<0x80) { if (!emit(static_cast<std::uint8_t>(point))) return false; }
                    else {
                        unsigned n=point<0x800 ? 2 : point<0x10000 ? 3 : 4;
                        if (!emit(static_cast<std::uint8_t>((0xffu<<(8-n))|(point>>(6*(n-1)))))) return false;
                        for (unsigned j=n-1;j!=0;--j) if (!emit(static_cast<std::uint8_t>(0x80|((point>>(6*(j-1)))&0x3f)))) return false;
                    }
                    break;
                }
                default: return false;
            }
        }
        return true;
    }
    inline bool String(const std::uint8_t*& p, const std::uint8_t* end, std::string_view& raw) noexcept {
        Space(p,end);
        if (p==end || *p++!='"') return false;
        const auto* begin=p;
        while (p<end) {
            if (*p=='"') {
                raw={reinterpret_cast<const char*>(begin),std::size_t(p-begin)}; ++p;
                return DecodeString(raw,[](std::uint8_t) noexcept { return true; });
            }
            if (*p++=='\\') { if (p==end) return false; ++p; }
        }
        return false;
    }
    inline bool Equal(std::string_view raw, std::string_view expected) noexcept {
        std::size_t index=0;
        return DecodeString(raw,[&](std::uint8_t c) noexcept { return index<expected.size() && c==static_cast<std::uint8_t>(expected[index++]); }) && index==expected.size();
    }
    inline bool Literal(const std::uint8_t*& p, const std::uint8_t* end, std::string_view value) noexcept {
        if (std::size_t(end-p)<value.size()) return false;
        for (std::size_t i=0;i<value.size();++i) if (p[i]!=static_cast<std::uint8_t>(value[i])) return false;
        p+=value.size(); return true;
    }
    inline bool Number(const std::uint8_t*& p, const std::uint8_t* end, std::string_view& number) noexcept {
        Space(p,end); const auto* begin=p;
        if (p<end && *p=='-') ++p;
        if (p==end) return false;
        if (*p=='0') ++p;
        else {
            if (*p<'1' || *p>'9') return false;
            do { ++p; } while (p<end && *p>='0' && *p<='9');
        }
        if (p<end && *p=='.') {
            ++p; const auto* first=p;
            while (p<end && *p>='0' && *p<='9') ++p;
            if (first==p) return false;
        }
        if (p<end && (*p=='e' || *p=='E')) {
            ++p; if (p<end && (*p=='+' || *p=='-')) ++p;
            const auto* first=p; while (p<end && *p>='0' && *p<='9') ++p;
            if (first==p) return false;
        }
        number={reinterpret_cast<const char*>(begin),std::size_t(p-begin)}; return true;
    }
    inline bool SkipValue(const std::uint8_t*& p, const std::uint8_t* end, std::size_t depth) noexcept;
    /// <summary>Counts a definite JSON container without copying values; recursion is bounded by the schema depth.</summary>
    inline bool Container(const std::uint8_t*& p, const std::uint8_t* end, std::size_t depth, bool object, std::uint32_t& count) noexcept {
        Space(p,end); if (!depth || p==end || *p++!=(object ? '{' : '[')) return false;
        Space(p,end); count=0;
        if (p<end && *p==(object ? '}' : ']')) { ++p; return true; }
        while (p<end) {
            if (object) { std::string_view name; if (!String(p,end,name)) return false; Space(p,end); if (p==end || *p++!=':') return false; }
            if (!SkipValue(p,end,depth-1) || count==0xffffffffu) return false;
            ++count; Space(p,end);
            if (p==end) return false;
            if (*p==(object ? '}' : ']')) { ++p; return true; }
            if (*p++!=',') return false;
        }
        return false;
    }
    inline bool SkipValue(const std::uint8_t*& p, const std::uint8_t* end, std::size_t depth) noexcept {
        Space(p,end); if (p==end) return false;
        std::string_view value; std::uint32_t count{};
        switch (*p) {
            case '{': return Container(p,end,depth,true,count);
            case '[': return Container(p,end,depth,false,count);
            case '"': return String(p,end,value);
            case 'n': return Literal(p,end,"null");
            case 't': return Literal(p,end,"true");
            case 'f': return Literal(p,end,"false");
            default: return Number(p,end,value);
        }
    }
}
