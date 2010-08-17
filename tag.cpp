/* tag.cpp -- identifies class by string name or 4-byte integer
 *
 *			Ryan McDougall
 */

#include <cstring>
#include "stdheaders.hpp"
#include "tag.hpp"

//-----------------------------------------------------------------------------
// MurmurHash2, by Austin Appleby

// Note - This code makes a few assumptions about how your machine behaves -

// 1. We can read a 4-byte value from any address without crashing
// 2. sizeof(int) == 4

// And it has a few limitations -

// 1. It will not work incrementally.
// 2. It will not produce the same results on little-endian and big-endian
//    machines.

static unsigned int simple_hash (const void *key, int len, unsigned int seed)
{
	// 'm' and 'r' are mixing constants generated offline.
	// They're not really 'magic', they just happen to work well.

	const unsigned int m = 0x5bd1e995;
	const int r = 24;

	// Initialize the hash to a 'random' value

	unsigned int h = seed ^ len;

	// Mix 4 bytes at a time into the hash

	const unsigned char * data = (const unsigned char *)key;

	while(len >= 4)
	{
		unsigned int k = *(unsigned int *)data;

		k *= m; 
		k ^= k >> r; 
		k *= m; 
		
		h *= m; 
		h ^= k;

		data += 4;
		len -= 4;
	}
	
	// Handle the last few bytes of the input array

	switch(len)
	{
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
	        h *= m;
	};

	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.
	
    h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
} 

namespace Scaffold
{
    Tag::Tag () 
        : number (0) 
    {
    }

    Tag::Tag (const string &n, unsigned int seed)
        : name (n), number (simple_hash (n.c_str(), n.size(), seed)) 
    {
    }

    Tag::Tag (const char *n, unsigned int seed)
        : name (n), number (simple_hash (n, strlen (n), seed)) 
    {
    }

    bool Tag::operator== (const Tag &r) const
    { 
        return number == r.number; 
    }

    bool Tag::operator!= (const Tag &r) const
    { 
        return number != r.number; 
    }

    bool Tag::operator< (const Tag &r) const
    { 
        return number < r.number; 
    }

    bool Tag::operator> (const Tag &r) const
    { 
        return number > r.number; 
    }


    Tagged::Tagged (const Tag &t) 
        : tag_ (t) 
    {
    }

    Tag Tagged::tag() const
    { 
        return tag_; 
    }

    string Tagged::name() const
    { 
        return tag_.name; 
    }

    bool Tagged::operator== (const Tagged &r) const
    { 
        return tag_ == r.tag_; 
    }

    bool Tagged::operator!= (const Tagged &r) const
    { 
        return tag_ != r.tag_; 
    }
    
    bool Tagged::operator< (const Tagged &r) const
    { 
        return tag_ < r.tag_; 
    }

    bool Tagged::operator> (const Tagged &r) const
    { 
        return tag_ > r.tag_; 
    }
}

