/* llparser.hpp -- parse LL message template file
 *
 *			Ryan McDougall
 */

#ifndef LLPARSER_H_
#define LLPARSER_H_

namespace Scaffold
{
    namespace LLStack
    {
        const size_t MAX_MESSAGE_SIZE (2048);
        const size_t MESSAGE_POOL_SIZE (16);
        const uint8_t ZERO_CODE_FLAG (0x80);
        const uint8_t RELIABLE_FLAG (0x40);
        const uint8_t RESENT_FLAG (0x20);
        const uint8_t ACK_FLAG (0x10);

        struct VariableInfo
        {
            enum 
            { 
                ERROR, BOOL, S8, S16, S32, S64, U8, U16, U32, U64, F32, F64,
                LLUUID, LLVECTOR3, LLVECTOR3D, LLVECTOR4, LLQUATERION, 
                IPADDR, IPPORT, FIXED, VARIABLE, VARIABLE1, VARIABLE2 
            };

            typedef std::vector <VariableInfo> List;

            string  name;
            int     type;
            int     size;
        };

        struct BlockInfo
        {
            enum { ERROR, SINGLE, MULTIPLE, VARIABLE };

            typedef std::vector <BlockInfo> List;

            string  name;
            int     repetition;
            int     multiplicy;

            VariableInfo::List variables;
        };

        struct PacketInfo
        {
            enum { ERROR, LOW, MEDIUM, HIGH };

            typedef std::vector <PacketInfo> List;

            string      name;
            uint32_t    id;

            int         priority;
            bool        trusted;
            bool        encoded;
            bool        deprecated;

            BlockInfo::List blocks;
        };

        struct ByteBuffer
        {
            typedef std::vector <ByteBuffer *> Heap;
            typedef std::set <ByteBuffer *> Set;

            size_t  size;
            uint8_t *data;
        
            ByteBuffer (size_t s) : size (s), data (new uint8_t [s]) {}
            void dispose () { delete [] data; }
            
            bool operator< (const ByteBuffer &r) { return size < r.size; }
        };

        class Message 
        {
            public:
                Message (shared_ptr <ByteBuffer> d) : 
                    data_ (d), id_ (0), begin_ (d.get()->data), 
                    pos_ (begin_), end_ (begin_), max_ (begin_ + d->size)
                {}

                Message (shared_ptr <ByteBuffer> d, uint32_t id) : 
                    data_ (d), id_ (id), begin_ (d.get()->data), 
                    pos_ (begin_), end_ (begin_), max_ (begin_ + d->size)
                {}

                void seek (size_t pos, ios_base::seekdir dir)
                {
                    switch (dir)
                    {
                        case ios_base::beg: pos_ = begin_ + pos; break;
                        case ios_base::cur: pos_ = pos_ + pos; break;
                        case ios_base::end: pos_ = end_ + pos; break;
                    }
                }

                template <typename T>
                void put (T value)
                {
                }

                template <typename T>
                void get (T& value)
                {
                }

                template <typename T>
                void push (T value)
                {
                }

                template <typename T>
                void pop (T& value)
                {
                }

                void push_header (uint8_t flags, uint32_t seq, uint8_t extra = 0)
                {
                }

                void pop_header (uint8_t &flags, uint32_t &seq, uint8_t &extra)
                {
                }

                void push_block (size_t repetitions)
                {
                }

                void pop_block (size_t &repetitions)
                {
                }

            private:
                shared_ptr <ByteBuffer> data_;

                uint32_t    id_;
                uint8_t     *begin_;
                uint8_t     *pos_;
                uint8_t     *end_;
                uint8_t     *max_;
        };

        class MessageParser
        {
            public:
                MessageParser (const char *filename);

            public:
                bool parse (PacketInfo &info);

            private:
                bool parse_ (PacketInfo &info);

            private:
                bool            run_;
                std::ifstream   file_;
        };
        
        class MessageFactory
        {
            public:
                MessageFactory ()
                    : error_ (false), parser_ ("message_template.msg")
                {
                    error_ = parser_.parse (info_);

                    for (int i=0; i < MESSAGE_POOL_SIZE; ++i)
                        free_.push_back (new ByteBuffer (MAX_MESSAGE_SIZE));

                    std::make_heap (free_.begin(), free_.end());
                }

                ~MessageFactory ()
                {
                    for_each (free_.begin(), free_.end(), mem_fn (&ByteBuffer::dispose));
                    for_each (used_.begin(), used_.end(), mem_fn (&ByteBuffer::dispose));
                }

            public:
                Message *create (const string &name, size_t size)
                {
                    if (next_free_buffer_()->size < size)
                        add_free_buffer_ (size);

                    ByteBuffer *buf = get_free_buffer_ ();

                    shared_ptr <ByteBuffer> ptr 
                        (buf, bind (&MessageFactory::set_free_buffer_, this, _1));

                    Message *m = new Message (ptr, size);

                    set_used_buffer_ (buf);

                    return m;
                }

            private:
                ByteBuffer *next_free_buffer_ ()
                {
                    return free_[0];
                }

                ByteBuffer *add_free_buffer_ (size_t size)
                {
                    ByteBuffer *buf = new ByteBuffer (size);

                    free_.push_back (buf);
                    push_heap (free_.begin(), free_.end());

                    return buf;
                }

                ByteBuffer *get_free_buffer_ ()
                {
                    pop_heap (free_.begin(), free_.end());
                    ByteBuffer *buf = free_.back ();

                    free_.pop_back ();
                    return buf;
                }

                void set_free_buffer_ (ByteBuffer *buf)
                {
                    free_.push_back (buf);
                    push_heap (free_.begin(), free_.end());

                    used_.erase (buf);
                }

                void set_used_buffer_ (ByteBuffer *buf)
                {
                    used_.insert (buf);
                }

            private:
                bool    error_;

                PacketInfo          info_;
                MessageParser       parser_;

                ByteBuffer::Set     used_;
                ByteBuffer::Heap    free_;
        };
    }
}

#endif //LLPARSER_H_
