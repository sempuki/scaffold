// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   LLUUID.h
 *  @brief  LLUUID is a 16-byte identifier for resources in a virtual world.
 */

#ifndef LLUUID_H_
#define LLUUID_H_

#include "stdheaders.hpp"

namespace Scaffold
{
    namespace LLPlugin
    {
        class UUID
        {
            public:
                /// Constructs an LLUUID from a string in form "1c1bbda2-304b-4cbf-ba3f-75324b044c73" or "1c1bbda2304b4cbfba3f75324b044c73".
                /// @param str String.
                explicit UUID(const char *str);
                explicit UUID(const string &str);

                /// Constructs a null LLUUID.
                UUID();

                /// Sets all 16 bytes of the ID to '00'.
                void clear();

                /// Checks is the UUID null.
                bool isNull() const;

                /// randomizes the contents
                void randomize();

                /// creates a random UUID
                static UUID random() { UUID r; r.randomize(); return r; }

                /// Creates an UUID from string.
                /// @param str String.
                void fromString(const char *str);
                void fromString(const string &str);

                /// Converts the UUID to a string.
                /// @return UUID as a string.
                string toString() const;
                operator string () const { return toString(); }

                /// Tests whether a string contains a valid UUID
                /// @param str String.
                static bool isValid(const char *str);
                static bool isValid(const string &str);

                UUID &operator= (const UUID &rhs);
                bool operator== (const UUID &rhs) const;
                bool operator!= (const UUID &rhs) const;
                bool operator< (const UUID &rhs) const;

                friend std::ostream& operator<< (std::ostream &out, const UUID &r)
                {
                    out << "LLUUID(" << r.toString() << ")";
                    return out;
                }

                /// Size in bytes.
                static const uint8_t SIZE = 16;

                /// Data.
                uint8_t data[SIZE];
        };
    }
}

#endif
