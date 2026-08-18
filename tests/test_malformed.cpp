#include <cassert>
#include <cstdint>
#include <random>
#include <vector>
#include <ESPressio_BinaryArchive.hpp>
#include <ESPressio_CborArchive.hpp>
using namespace ESPressio;
template<class A> void Mutate(const std::vector<uint8_t>& seed){std::mt19937 rng(0x45535052);for(int n=0;n<2000;++n){auto d=seed;if(d.empty())d.push_back(0);int edits=1+(rng()%4);for(int e=0;e<edits;++e){switch(rng()%3){case 0:d[rng()%d.size()]^=uint8_t(1u<<(rng()%8));break;case 1:if(d.size()>1)d.erase(d.begin()+(rng()%d.size()));break;case 2:if(d.size()<2048)d.insert(d.begin()+(rng()%(d.size()+1)),uint8_t(rng()));break;}}A a; (void)a.Load(d);}}
int main(){Serializable::BinaryArchive b;b.Write("x",uint32_t(42));auto bd=b.GetData();Serializable::CborArchive c;c.Write("x",uint32_t(42));auto cd=c.GetData();Mutate<Serializable::BinaryArchive>(bd);Mutate<Serializable::CborArchive>(cd);std::vector<uint8_t> truncated={'E','S','P','B',2};Serializable::BinaryArchive bad;assert(!bad.Load(truncated));return 0;}
