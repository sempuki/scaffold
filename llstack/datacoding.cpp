/* datacoding.cpp -- en/decode binary data for network transmission
 *
 *			Ryan McDougall
 */

#include "stdheaders.hpp"
#include "llstack/datacoding.hpp"

namespace Scaffold
{
    namespace Data
    {
        // Minimalist circular buffer API
        // Memory is passed via the constructor

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

        // get the length of a run of code characters
        size_t get_run_length (const uint8_t *begin, const uint8_t *end, uint8_t code)
        {
            size_t length (0);

            for (; (begin != end) && (*begin == code); ++begin, ++length);

            return length;
        }

        // get result size if Run-Length Encoding used on data
        size_t get_encode_size (uint8_t *begin, uint8_t *end, uint8_t code)
        {
            size_t length (0);

            while (begin != end)
            {
                if (*begin == code)
                {
                    length += 2;
                    begin += get_run_length (begin, end, code);
                }
                else
                {
                    ++length;
                    ++begin;
                }
            }

            return length;
        }

        // get result size if Run-Length Decoding used data
        size_t get_decode_size (uint8_t *begin, uint8_t *end, uint8_t code)
        {
            size_t length (0);

            while (begin != end)
            {
                if (*begin == code)
                {
                    assert (begin + 1 != end);

                    length += *(begin + 1);
                    begin += 2;
                }
                else
                {
                    ++begin;
                    ++length;
                }
            }

            return length;
        }

        // Run-Length Encoding
        // NOTE: in-place encoding. encoding must not overrun the buffer.
        // use get_encode_size() to verify.
        bool encode (uint8_t *begin, uint8_t *end, size_t bufsize, uint8_t code)
        {
            assert (end - begin >= 1);
            assert (end - begin <= bufsize);
            assert (get_encode_size (begin, end, code) <= bufsize);

            uint8_t skip;
            uint8_t *in (begin), *out (begin);
            uint8_t memory [end - begin];

            // buffer encoded input before writing to free space
            circular_buffer <uint8_t> buf (memory, end - begin);

            while (in < end)
            {
                // read input into buf
                if (*in == code)
                {
                    // buffer encoding
                    skip = get_run_length (in, end, code);
                    buf.push_back (code);
                    buf.push_back (skip);

                    // encoding bloats, displaces input
                    if (skip < 2) ++in, buf.push_back (*in);

                    // advance input
                    in += skip;
                }
                else
                    buf.push_back (*in), ++in;

                // write out buffered bytes
                for (; buf.size && (out < in); ++out)
                    *out = *buf.front, buf.pop_front();
            }
        }

        // Run-Length Decoding
        // NOTE: in-place decoding. decoding must not overrun the buffer.
        // use get_decode_size() to verify.
        bool decode (uint8_t *begin, uint8_t *end, size_t bufsize, uint8_t code)
        {
            assert (end - begin >= 2);
            assert (end - begin <= bufsize);
            assert (get_decode_size (begin, end, code) <= bufsize);

            using std::fill;

            uint8_t skip, value;
            uint8_t *in (begin), *out (begin);
            uint8_t memory [end - begin];

            // buffer input overwritten by decoded output
            circular_buffer <uint8_t> buf (memory, end - begin);

            // initialize input buffer
            buf.push_back (*in), ++in;

            while (in < end)
            {
                // keep filling buffer 
                buf.push_back (*in), ++in;

                // read next buffered value
                value = *buf.front, buf.pop_front ();

                if (value == code)
                {
                    // keep filling buffer 
                    buf.push_back (*in), ++in;

                    // find the code's runlength
                    skip = *buf.front, buf.pop_front ();

                    assert (skip <= buf.capacity - buf.size);

                    // buffer overwritten memory
                    for (uint8_t *i = in, *e = out + skip; i < e; ++i)
                        buf.push_back (*i);

                    // fill code in-place
                    fill (out, out + skip, code);
                    out += skip;
                }
                else
                    // write out current value
                    *out = value, ++out;
            }
        }
    }
}

