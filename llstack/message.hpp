/* llparser.hpp -- parse LL message template file
 *
 *			Ryan McDougall
 */

#ifndef LLPARSER_H_
#define LLPARSER_H_

#include <QtEndian>
#include <QVector3D>
#include <QQuaternion>

namespace Scaffold
{
    namespace LLStack
    {
        const float MAX_BPS (1000000.0f);
        const size_t MAX_MESSAGE_SIZE (2048);
        const size_t MAX_MESSAGE_APPEND_ACKS (100);
        const size_t MESSAGE_RESEND_AGE (5);
        const size_t MESSAGE_WINDOW (256);
        const size_t MESSAGE_POOL_SIZE (16);
        const size_t MESSAGE_HEADER_SIZE (6);
        const size_t MESSAGE_EXTRA_HEADER (5);
        const uint8_t ZERO_CODE_FLAG (0x80);
        const uint8_t RELIABLE_FLAG (0x40);
        const uint8_t RESEND_FLAG (0x20);
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
                struct SequenceComp
                {
                    bool operator() (const Message &l, const Message &r)
                    {
                        return l.getSequence() < r.getSequence();
                    }
                };
                
                struct AgeComp
                {
                    bool operator() (const Message &l, const Message &r)
                    {
                        return l.age() < r.age();
                    }
                };
                
            public:
                typedef std::map <uint32_t, Message> Map;
                typedef std::map <uint32_t, string> NameMap;
                typedef std::map <string, uint32_t> IDMap;
                typedef std::set <uint32_t> SequenceSet;

                typedef std::vector <string> GenericParams;

                typedef void (Listener) (Message);
                typedef Subscription <Listener> Signal;
                typedef std::map <msg_id_t, Signal> SubscriptionMap;

                enum SeekType { Begin, Body, Curr, Append, End };

            public:
                Message (shared_ptr <ByteBuffer> d, uint32_t id = 0, uint8_t flags = 0, uint32_t seq = 0);

                uint32_t getID () const;
                uint32_t getSequence () const;
                uint8_t getFlags () const;

                time_t age () const;

                int priority () const;
                int size () const;
                int headerSize () const;
                int bodySize () const;
                int appendAckSize () const;
                int bufferSize () const;

                void setID (uint32_t id);
                void setSequenceNumber (uint32_t seq);
                void setFlags (uint8_t flags);
                void enableFlags (uint8_t flags);
                void disableFlags (uint8_t flags);

                void setAge (time_t age);
                void setSize (int size);

                int seek (int pos, SeekType dir);
                void advance (int pos);

                template <typename T> void skip () { advance (sizeof (T)); }

                template <typename T> void put (T value) { T *ptr = (T *)pos_; *ptr = value; }
                template <typename T> void putBigEndian (T value) { qToBigEndian <T> (value, pos_); } 
                template <typename T> void putLittleEndian (T value) { qToLittleEndian <T> (value, pos_); } 

                template <typename T> void get (T& value) { T *ptr = (T *)pos_; value = *ptr; }
                template <typename T> void getBigEndian (T& value) { value = qFromBigEndian <T> (pos_); }
                template <typename T> void getLittleEndian (T& value) { value = qFromLittleEndian <T> (pos_); } 

                template <typename T> void push (T value) { put (value); skip <T> (); }
                template <typename T> void pushBigEndian (T value) { putBigEndian (value); skip <T> (); }
                template <typename T> void pushLittleEndian (T value) { putLittleEndian (value); skip <T> (); } 

                template <typename T> void pop (T& value) { get (value); skip <T> (); }
                template <typename T> void popBigEndian (T& value) { getBigEndian (value); skip <T> (); }
                template <typename T> void popLittleEndian (T& value) { getLittleEndian (value); skip <T> (); }

                void pushHeader ();
                void popHeader ();

                void pushMsgID ();
                void popMsgID ();

                void pushBlock (uint8_t repetitions);
                void popBlock (uint8_t &repetitions);

                void pushVariableSize (size_t size);
                void pushVariable (const std::vector <uint8_t> &buf);
                void popVariable1 (std::vector <uint8_t> &buf, uint8_t &size);
                void popVariable2 (std::vector <uint8_t> &buf, uint16_t &size);

                pair <const char*, size_t> readBuffer () const;
                pair <char*, size_t> writeBuffer () const;

                void clear ();

                void print (std::ostream &out);

            private:
                int get_priority_ (uint32_t id);

            private:
                shared_ptr <ByteBuffer> data_;

                uint32_t    id_;
                uint32_t    seq_;
                int         priority_;
                uint8_t     flags_;
                time_t      age_;

                uint8_t     *begin_;
                uint8_t     *pos_;
                uint8_t     *end_;
                uint8_t     *max_;
        };

        template <> void Message::push <string> (string value);
        template <> void Message::pop <string> (string &value);
        template <> void Message::push <QQuaternion> (QQuaternion value);
        template <> void Message::pop <QQuaternion> (QQuaternion &value);

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
                Message create (uint32_t id = 0, uint8_t flags = 0, size_t size = 0);

            private:
                ByteBuffer *add_free_buffer_ (size_t size);
                ByteBuffer *next_free_buffer_ ();
                ByteBuffer *get_free_buffer_ ();

                void set_free_buffer_ (ByteBuffer *buf);
                void set_used_buffer_ (ByteBuffer *buf);

            private:
                bool        error_;

                PacketInfo          info_;
                MessageParser       parser_;

                ByteBuffer::Set     used_;
                ByteBuffer::Heap    free_;
        };
    }
}

#endif //LLPARSER_H_
