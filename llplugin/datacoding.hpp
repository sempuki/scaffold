/* datacoding.hpp -- en/decode binary data for network transmission
 *
 *			Ryan McDougall
 */

#ifndef DATACODING_H_
#define DATACODING_H_

namespace Scaffold
{
    namespace Data
    {
        namespace runlength
        {
            // get result size if Run-Length Encoding used on data
            size_t get_encode_size (const uint8_t *begin, const uint8_t *end, uint8_t code);

            // get result size if Run-Length Decoding used data
            size_t get_decode_size (const uint8_t *begin, const uint8_t *end, uint8_t code);

            // Run-Length Encoding
            // [begin,end) denotes the range of valid data.
            int encode (uint8_t *begin, uint8_t *end, uint8_t code);

            // Run-Length Decoding
            // [begin,end) denotes the range of valid data.
            int decode (uint8_t *begin, uint8_t *end, uint8_t code);
        }
    }
}

#endif
