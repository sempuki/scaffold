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
        // get the length of a run of code characters
        size_t get_run_length (const uint8_t *begin, const uint8_t *end, uint8_t code);

        // get result size if Run-Length Encoding used on data
        size_t get_encode_size (uint8_t *begin, uint8_t *end, uint8_t code);

        // get result size if Run-Length Decoding used data
        size_t get_decode_size (uint8_t *begin, uint8_t *end, uint8_t code);

        // Run-Length Encoding
        // NOTE: in-place encoding. encoding must not overrun the buffer.
        // use get_encode_size() to verify.
        bool encode (uint8_t *begin, uint8_t *end, size_t bufsize, uint8_t code);

        // Run-Length Decoding
        // NOTE: in-place decoding. decoding must not overrun the buffer.
        // use get_decode_size() to verify.
        bool decode (uint8_t *begin, uint8_t *end, size_t bufsize, uint8_t code);
    }
}

#endif
