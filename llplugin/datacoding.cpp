/* datacoding.cpp -- en/decode binary data for network transmission
 *
 *			Ryan McDougall
 */

#include "stdheaders.hpp"
#include "llplugin/datacoding.hpp"

namespace Scaffold
{
    namespace Data
    {
        // Minimalist circular buffer API
        // Memory is passed via the constructor
        // NOTE: can be used to avoid copying with a circular buffer 
        // the size of the difference between en/decoding.

        template <typename T>
        struct circular_buffer
        {
            T *const buffer; 
            T *front, *back;
            size_t const capacity;
            size_t size;

            circular_buffer (T *buf, size_t cap) : 
                buffer (buf), front (0), back (0), 
                capacity (cap), size (0) {}

            void push_front (T v)
            {
                if (!size) 
                    front = back = buffer + capacity-1;
                else 
                    -- front;

                if (front < buffer) 
                    front += capacity;

                *front = v;
                ++ size;

                assert (size <= capacity);
            }

            void pop_front ()
            {
                ++ front;
                -- size;

                if (front >= buffer + capacity)
                    front -= capacity;

                assert (size <= capacity);
            }

            void push_back (T v)
            {
                if (!size) 
                    front = back = buffer;
                else
                    ++ back;

                if (back == buffer + capacity)
                    back -= capacity;

                ++ size;
                *back = v;

                assert (size <= capacity);
            }

            void pop_back ()
            {
                -- back;
                -- size;

                if (back < buffer)
                    back += capacity;

                assert (size <= capacity);
            }
        };

        namespace runlength
        {
            // get result size if Run-Length Encoding used on data
            size_t get_encode_size (const uint8_t *begin, const uint8_t *end, uint8_t code)
            {
                size_t length (0);

                while (begin != end)
                {
                    if (*begin == code)
                    {
                        for (; (begin != end) && (*begin == code); ++begin);
                        length += 2;
                    }
                    else
                        ++begin, ++length;
                }

                return length;
            }

            // get result size if Run-Length Decoding used data
            size_t get_decode_size (const uint8_t *begin, const uint8_t *end, uint8_t code)
            {
                size_t length (0);

                while (begin != end)
                {
                    if (*begin == code)
                        length += *(begin + 1), begin += 2;
                    else
                        ++length, ++begin;
                }

                return length;
            }

            // Run-Length Encoding
            // [begin,end) denotes the range of valid data.
            int encode (uint8_t *begin, uint8_t *end, uint8_t code)
            {
                assert (end - begin >= 1);

                using std::copy;

                int size = get_encode_size (begin, end, code);
                uint8_t buffer [size];
                uint8_t *in (begin), *out (buffer);
                uint8_t skip;

                if (size >= (end - begin))
                    return end - begin - size;

                while (in != end)
                {
                    if (*in == code)
                    {
                        for (skip = 0; (in != end) && (*in == code); ++in, ++skip);
                        *out++ = code, *out++ = skip;
                    }
                    else
                        *out++ = *in++;
                }

                assert (out == buffer + size);
                copy (buffer, out, begin);

                return out - buffer;
            }

            // Run-Length Decoding
            // [begin,end) denotes the range of valid data.
            int decode (uint8_t *begin, uint8_t *end, uint8_t code)
            {
                assert (end - begin >= 2);

                using std::copy;

                int size = get_decode_size (begin, end, code);
                uint8_t buffer [size];
                uint8_t *in (begin), *out (buffer);
                uint8_t skip;

                while (in != end)
                {
                    if (*in == code)
                    {
                        ++in, skip = *in, ++in;
                        while (skip--) *out++ = code;
                    }
                    else
                        *out++ = *in++;
                }

                assert (out == buffer + size);
                copy (buffer, out, begin);

                return out - buffer;
            }
        }
    }
}
