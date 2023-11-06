#ifndef VECTOR_TILE_COMPRESSION_HPP_
#define VECTOR_TILE_COMPRESSION_HPP_

#include <string>
// zlib
#include <zlib.h>

namespace mapnik 
{
 
namespace vector_tile_impl 
{

inline bool is_zlib_compressed(const char * data, std::size_t size)
{
    return size > 2 &&
           static_cast<uint8_t>(data[0]) == 0x78 &&
           (
               static_cast<uint8_t>(data[1]) == 0x9C ||
               static_cast<uint8_t>(data[1]) == 0x01 ||
               static_cast<uint8_t>(data[1]) == 0xDA ||
               static_cast<uint8_t>(data[1]) == 0x5E
           );
}

inline bool is_gzip_compressed(const char * data, std::size_t size)
{
    return size > 2 && static_cast<uint8_t>(data[0]) == 0x1F && static_cast<uint8_t>(data[1]) == 0x8B;
}

// decodes both zlib and gzip
// http://stackoverflow.com/a/1838702/2333354
void zlib_decompress(const char * data,
                                          std::size_t size, 
                                          std::string & output);

} // end ns vector_tile_impl

} // end ns mapnik

#endif /* VECTOR_TILE_COMPRESSION_HPP_ */
