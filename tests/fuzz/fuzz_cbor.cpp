#include <cstddef>
#include <cstdint>
#include <ESPressio_CborArchive.hpp>
extern "C" int LLVMFuzzerTestOneInput(const uint8_t*data,size_t size){ESPressio::Serializable::CborArchive a;a.Load(data,size);return 0;}
