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

        class MessageInfo
        {
            public:
                typedef std::map <string, MessageInfo> Map;

                MessageInfo (const string &name, const OffsetMap &map)
                    : name_ (name), map_ (map)
                {}

            public:
                string name () const { return name_; }
                size_t offset (const string &v) const { return map_.find(v)->second; }

            private:
                string          name_;
                OffsetMap       map_;
        };

        class Message 
        {
            public:
                typedef std::set <Message *> Set;

                Message (const MessageInfo &info) 
                    : info_ (info), id_ (0), seq_ (0), data_ (MAX_MESSAGE_SIZE)
                {}

                Message (const MessageInfo &info, uint32_t id, uint32_t seq, size_t size)
                    : info_ (info), id_ (id), seq_ (seq), data_ (size)
                {}

            public:
                const MessageInfo &info() { return info_; }

            private:
                const MessageInfo       &info_;

                uint32_t                id_;
                uint32_t                seq_;

                std::vector <uint8_t>   data_;
        };

        class MessageParser
        {
            public:
                MessageParser (const char *filename);

            public:
                bool parse (MessageInfo::Map &info);

            private:
                bool parse_ (MessageInfo::Map &info);

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
                }

                ~MessageFactory ()
                {
                    for_each (pool_.begin(), pool_.end(),
                            safe_delete <Message>);
                }

            public:
                Message *create (const string &name)
                {
                    const MessageInfo &i = info_.find(name)->second;

                    Message *m = new Message (i);
                    pool_.insert (m);

                    return m;
                }

            private:
                bool                error_;

                MessageParser       parser_;
                Message::Set        pool_;
                MessageInfo::Map    info_;
        };
    }
}

#endif //LLPARSER_H_
