/* tag.hpp -- identifies class by string name or 4-byte integer
 *
 *			Ryan McDougall
 */

#ifndef TAG_H_
#define TAG_H_

namespace Scaffold
{
    typedef unsigned int tag_t;

    struct Tag
    {
        typedef std::set <Tag> Set;
        typedef std::vector <Tag> List;

        Tag ();
        Tag (const string &n, unsigned int seed = 0);
        Tag (const char *n, unsigned int seed = 0);

        bool operator== (const Tag &r) const;
        bool operator!= (const Tag &r) const;
        bool operator< (const Tag &r) const;
        bool operator> (const Tag &r) const;

        string  name;
        tag_t   number; 
    };

    class Tagged
    {
        public:
            Tagged (const Tag &t);

            Tag tag() const;
            string name() const;

            bool operator== (const Tagged &r) const;
            bool operator!= (const Tagged &r) const;
            bool operator< (const Tagged &r) const;
            bool operator> (const Tagged &r) const;

        private:
            Tag tag_;
    };
}
#endif //TAG_H_
