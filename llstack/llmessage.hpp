/* llparser.hpp -- parse LL message template file
 *
 *			Ryan McDougall
 */

#ifndef LLPARSER_H_
#define LLPARSER_H_

#include <QtEndian>

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
            enum { ERROR, LOW, MEDIUM, HIGH, FIXED };

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
                enum SeekType { Beg, Cur, End };

                Message (shared_ptr <ByteBuffer> d) : 
                    data_ (d), id_ (0), begin_ (d.get()->data), 
                    pos_ (begin_), end_ (begin_), max_ (begin_ + d->size)
                {}

                Message (shared_ptr <ByteBuffer> d, uint32_t id) : 
                    data_ (d), id_ (id), begin_ (d.get()->data), 
                    pos_ (begin_), end_ (begin_), max_ (begin_ + d->size)
                {}

                void seek (size_t pos, SeekType dir = Cur)
                {
                    switch (dir)
                    {
                        case Beg: pos_ = begin_ + pos; break;
                        case Cur: pos_ = pos_ + pos; break;
                        case End: pos_ = end_ + pos; break;
                    }
                }

                template <typename T> 
                void next () 
                { 
                    using std::max;

                    pos_ += sizeof (T); 
                    end_ = max (pos_, end_);

                    assert (end_ <= max_);
                }

                template <typename T> void put (T value) { T *ptr = (T *)pos_; *ptr = value; }
                template <typename T> void putBig (T value) { qToBigEndian <T> (value, pos_); } 
                template <typename T> void putLittle (T value) { qToLittleEndian <T> (value, pos_); } 

                template <typename T> void get (T& value) { T *ptr = (T *)pos_; value = *ptr; }
                template <typename T> void getBig (T& value) { value = qFromBigEndian <T> (pos_); }
                template <typename T> void getLittle (T& value) { value = qFromLittleEndian <T> (pos_); } 

                template <typename T> void push (T value) { put (value); next <T> (); }
                template <typename T> void pushBig (T value) { putBig (value); next <T> (); }
                template <typename T> void pushLittle (T value) { putLittle (value); next <T> (); } 

                template <typename T> void pop (T& value) { get (value); next <T> (); }
                template <typename T> void popBig (T& value) { getBig (value); next <T> (); }
                template <typename T> void popLittle (T& value) { getLittle (value); next <T> (); }

                void pushHeader (uint8_t flags, uint32_t seq, uint8_t extra = 0)
                {
                    push (flags);
                    pushBig (seq);
                    push (extra);
                }

                void popHeader (uint8_t &flags, uint32_t &seq, uint8_t &extra)
                {
                    pop (flags);
                    popBig (seq);
                    pop (extra);
                }

                void pushMsgID (int priority, uint32_t id)
                {
                    switch (priority)
                    {
                        case PacketInfo::HIGH: 
                            pushBig <uint8_t> (id);
                            break;

                        case PacketInfo::MEDIUM: 
                            pushBig <uint16_t> (id | 0x0000FF00);
                            break;

                        case PacketInfo::LOW: 
                            pushBig <uint32_t> (id | 0xFFFF0000);
                            break;

                        case PacketInfo::FIXED: 
                            pushBig <uint32_t> (id);
                            break;
                    }
                }

                void popMsgID (int &priority, uint32_t &id)
                {
                    uint8_t id1; uint16_t id2; uint32_t id4;

                    get (id1);
                    if (id1 & 0xFF)
                    {
                        get (id2);
                        if (id2 & 0xFFFF)
                        {
                            get (id4);
                            if (id4 & 0xFFFFFF00)
                            {
                                priority = PacketInfo::FIXED;
                                popBig <uint32_t> (id4);
                                id = id4 & 0xFFFFFFFF;
                            }
                            else
                            {
                                priority = PacketInfo::LOW;
                                popBig <uint32_t> (id4);
                                id = id4 & 0x0000FFFF;
                            }
                        }
                        else
                        {
                            priority = PacketInfo::MEDIUM;
                            popBig <uint16_t> (id2);
                            id = id2 & 0x000000FF;
                        }
                    }
                    else
                    {
                        priority = PacketInfo::HIGH;
                        pop <uint8_t> (id1);
                        id = id1 & 0x000000FF;
                    }
                }

                void pushBlock (uint8_t repetitions) { push (repetitions); }
                void popBlock (uint8_t &repetitions) { pop (repetitions); }

                void print (std::ostream &out)
                {
                    using std::ostream_iterator;
                    using std::copy;
                    using std::hex;

                    copy (begin_, end_, ostream_iterator <int> (out << hex, " "));
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
                MessageFactory ();
                ~MessageFactory ();

            public:
                Message *create (const string &name, size_t size);

            private:
                ByteBuffer *add_free_buffer_ (size_t size);
                ByteBuffer *next_free_buffer_ ();
                ByteBuffer *get_free_buffer_ ();

                void set_free_buffer_ (ByteBuffer *buf);
                void set_used_buffer_ (ByteBuffer *buf);

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
