/* main.cpp -- main module
 *
 *			Ryan McDougall
 */

#include "stdheaders.hpp"
#include "subscription.hpp"
#include "llstack/message.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/io.hpp>

#include <cstring>

//=============================================================================

namespace Scaffold
{
    namespace LLStack 
    {
        //=============================================================================
        // boost.spirit based parser implementation

        template <typename Iterator>
        struct skip_parser : boost::spirit::qi::grammar <Iterator>
        {
            boost::spirit::qi::rule <Iterator> start, comment;

            skip_parser () : skip_parser::base_type (start)
            {
                using namespace boost::spirit::qi;

                comment = "//" >> *(char_ - eol) >> eol ;
                start = ascii::space | comment;
            }
        };

        template <typename Iterator>
        struct packet_info_parser : boost::spirit::qi::grammar <Iterator, skip_parser<Iterator> >
        {
            boost::spirit::qi::rule <Iterator, skip_parser<Iterator> > 
                start, info, pinfo, binfo, vinfo, 
                version, name, priority, id, trusted, encoded, deprecated, 
                block_name, block_repetition, block_multiplicity, 
                variable_name, variable_type, variable_length;

            PacketInfo::List result;

            packet_info_parser () : packet_info_parser::base_type (start)
            {
                using namespace boost::spirit::qi;
                using boost::spirit::qi::hex;
                using std::tr1::placeholders::_1;

                version = omit [-("version" >> *(char_ - '{'))];

                name = lexeme [*ascii::alnum] [bind (&packet_info_parser::parse_name, this, _1)];
                priority = lexeme [*ascii::alnum] [bind (&packet_info_parser::parse_priority, this, _1)];
                id = ("0x" >> hex | int_ [bind (&packet_info_parser::parse_id, this, _1)]) ;
                trusted = lexeme [*ascii::alnum] [bind (&packet_info_parser::parse_trusted, this, _1)];
                encoded = lexeme [*ascii::alnum] [bind (&packet_info_parser::parse_encoded, this, _1)];
                deprecated = -(lexeme [*ascii::alnum]) [bind (&packet_info_parser::parse_deprecated, this, _1)];

                block_name = lexeme [*ascii::alnum] [bind (&packet_info_parser::parse_block_name, this, _1)];
                block_repetition = lexeme [*ascii::alnum] [bind (&packet_info_parser::parse_block_repetition, this, _1)];
                block_multiplicity = (int_ [bind (&packet_info_parser::parse_block_muliplicity, this, _1)] | &lit('{'));

                variable_name = lexeme [*ascii::alnum] [bind (&packet_info_parser::parse_variable_name, this, _1)];
                variable_type = lexeme [*ascii::alnum] [bind (&packet_info_parser::parse_variable_type, this, _1)];
                variable_length = (int_ [bind (&packet_info_parser::parse_variable_length, this, _1)] | &lit('}'));

                vinfo = variable_name >> variable_type >> variable_length;
                binfo = block_name >> block_repetition >> block_multiplicity;
                pinfo = name >> priority >> id >> trusted >> encoded >> deprecated;

                info = lit('{') [bind (&packet_info_parser::begin_packet, this)] >> pinfo >>
                    *(lit('{') [bind (&packet_info_parser::begin_block, this)] >> binfo >>
                            *(lit('{') [bind (&packet_info_parser::begin_variable, this)] >> vinfo >>
                                '}') >> '}') >> '}';

                start = version >> *(info);
            }

            PacketInfo &packet () { return result.back(); }
            BlockInfo &block () { return packet().blocks.back(); }
            VariableInfo &variable () { return block().variables.back(); }

            void begin_packet () { result.push_back (PacketInfo ()); }
            void begin_block () { packet().blocks.push_back (BlockInfo ()); }
            void begin_variable () { block().variables.push_back (VariableInfo ()); }

            void parse_name (const std::vector<char> &v)
            {
                packet().name.assign (v.begin(), v.end());
            }

            void parse_priority (const std::vector<char> &v)
            {
                string s (v.begin(), v.end());

                if (s == "Low") packet().priority = PacketInfo::LOW;
                else if (s == "Medium") packet().priority = PacketInfo::MEDIUM;
                else if (s == "High") packet().priority = PacketInfo::HIGH;
                else if (s == "Fixed") packet().priority = PacketInfo::FIXED;
                else packet().priority = PacketInfo::ERROR;
            }

            void parse_id (int v)
            {
                uint32_t id = v;

                // synthezie an unique id 
                if (packet().priority == PacketInfo::MEDIUM)
                    id |= 0x0000FF00;
                else if (packet().priority == PacketInfo::HIGH)
                    id |= 0xFFFF0000;
                else ; // LOW

                packet().id = id;
            }

            void parse_trusted (const std::vector<char> &v)
            {
                string s (v.begin(), v.end());
                packet().trusted = (s == "Trusted");
            }

            void parse_encoded (const std::vector<char> &v)
            {
                string s (v.begin(), v.end());
                packet().encoded = (s == "Encoded");
            }

            void parse_deprecated (const std::vector<char> &v)
            {
                string s (v.begin(), v.end());
                packet().deprecated = (s == "Deprecated");
            }

            void parse_block_name (const std::vector<char> &v)
            {
                block().name.assign (v.begin(), v.end());
            }

            void parse_block_repetition (const std::vector<char> &v)
            {
                string s (v.begin(), v.end());

                if (s == "Single") 
                {
                    block().repetition = BlockInfo::SINGLE;
                    block().multiplicy = 1;
                }
                else if (s == "Multiple") 
                {
                    block().repetition = BlockInfo::MULTIPLE;
                    // multiplicy to be parsed
                }
                else if (s == "Variable") 
                {
                    block().repetition = BlockInfo::VARIABLE;
                    block().multiplicy = 0;
                }
                else 
                {
                    block().repetition = BlockInfo::ERROR;
                    block().multiplicy = 0;
                }
            }

            void parse_block_muliplicity (int v)
            {
                block().multiplicy = v;
            }

            void parse_variable_name (const std::vector<char> &v)
            {
                variable().name.assign (v.begin(), v.end());
            }

            void parse_variable_type (const std::vector<char> &v)
            {
                string s (v.begin(), v.end());

                if (s == "BOOL")
                {
                    variable().type = VariableInfo::BOOL;
                    variable().size = 1;
                }
                else if (s == "S8")
                {
                    variable().type = VariableInfo::S8;
                    variable().size = 1;
                }
                else if (s == "S16")
                {
                    variable().type = VariableInfo::S16;
                    variable().size = 2;
                }
                else if (s == "S32")
                {
                    variable().type = VariableInfo::S32;
                    variable().size = 4;
                }
                else if (s == "S64")
                {
                    variable().type = VariableInfo::S64;
                    variable().size = 8;
                }
                else if (s == "U8")
                {
                    variable().type = VariableInfo::U8;
                    variable().size = 1;
                }
                else if (s == "U16")
                {
                    variable().type = VariableInfo::U16;
                    variable().size = 2;
                }
                else if (s == "U32")
                {
                    variable().type = VariableInfo::U32;
                    variable().size = 4;
                }
                else if (s == "U64")
                {
                    variable().type = VariableInfo::U64;
                    variable().size = 8;
                }
                else if (s == "F32")
                {
                    variable().type = VariableInfo::F32;
                    variable().size = 4;
                }
                else if (s == "F64")
                {
                    variable().type = VariableInfo::F64;
                    variable().size = 8;
                }
                else if (s == "LLUUID")
                {
                    variable().type = VariableInfo::LLUUID;
                    variable().size = 16;
                }
                else if (s == "LLVector3")
                {
                    variable().type = VariableInfo::LLVECTOR3;
                    variable().size = 12;
                }
                else if (s == "LLVector3d")
                {
                    variable().type = VariableInfo::LLVECTOR3;
                    variable().size = 24;
                }
                else if (s == "LLVector4")
                {
                    variable().type = VariableInfo::LLVECTOR3;
                    variable().size = 16;
                }
                else if (s == "LLQuaterion")
                {
                    variable().type = VariableInfo::LLQUATERION;
                    variable().size = 12;
                }
                else if (s == "IPADDR")
                {
                    variable().type = VariableInfo::IPADDR;
                    variable().size = 4;
                }
                else if (s == "IPPORT")
                {
                    variable().type = VariableInfo::IPPORT;
                    variable().size = 2;
                }
                else if (s == "Fixed")
                {
                    variable().type = VariableInfo::FIXED;
                    // size to be parsed
                }
                else if (s == "Variable")
                {
                    variable().type = VariableInfo::VARIABLE;
                    // size to be parsed
                }
                else variable().type = VariableInfo::ERROR;
            }

            void parse_variable_length (int v)
            {
                if (variable().type == VariableInfo::VARIABLE)
                {
                    switch (v)
                    {
                        case 1: variable().type = VariableInfo::VARIABLE1;
                        case 2: variable().type = VariableInfo::VARIABLE2;
                    }
                }

                variable().size = v;
            }

        }; 

        //=============================================================================
        // template parser

        MessageParser::MessageParser (const char *filename)
            : run_ (false), file_ (filename)
        { 
            if (file_) file_.unsetf (std::ios::skipws);
        }

        bool MessageParser::parse (PacketInfo &info)
        {
            if (!run_ && file_) return parse_ (info);
            else return false;
        }

        bool MessageParser::parse_ (PacketInfo &info)
        {
            run_ = true;

            using boost::spirit::qi::phrase_parse;
            using boost::spirit::istream_iterator;

            istream_iterator begin (file_), end;
            packet_info_parser <istream_iterator> parser;
            skip_parser <istream_iterator> skipper;

            bool result = phrase_parse (begin, end, parser, skipper);

            if (!result || (begin != end))
                return false;

            //PacketInfo::List::iterator i = parser.result.begin();
            //PacketInfo::List::iterator e = parser.result.end();
            //for (; i != e; ++i)
            //{
            //    //cout << "name: " << i->name << endl;

            //    size_t offset = 0;
            //    MessageInfo::OffsetMap map;

            //    BlockInfo::List::iterator bi = i->blocks.begin();
            //    BlockInfo::List::iterator be = i->blocks.end();
            //    for (; bi != be; ++bi)
            //    {
            //        //cout << "\tblock name: " << bi->name << endl;

            //        if (bi->repetition != BlockInfo::SINGLE)
            //            offset += 1; // uint8 repetition var

            //        for (int i=0; i < bi->multiplicy; ++i)
            //        {
            //            VariableInfo::List::iterator vi = bi->variables.begin();
            //            VariableInfo::List::iterator ve = bi->variables.end();
            //            for (; vi != ve; ++vi)
            //            {
            //                //cout << "\t\tvariable name: " << vi->name << endl;
            //                //cout << "\t\tvariable type: " << vi->type << endl;
            //                //cout << "\t\tvariable size: " << vi->size << endl;

            //                string name; 
            //                name += bi->name; 
            //                name += SEPERATOR; 
            //                name += vi->name;

            //                map.insert (make_pair (name, offset));
            //                offset += vi->size;
            //            }
            //        }
            //    }

            //    info.insert (make_pair (i->name, MessageInfo (i->name, map)));
            //}

            return true;
        }


        //=============================================================================
        // Unchecked message type

        Message::Message (shared_ptr <ByteBuffer> d, uint32_t id) : 
            data_ (d), id_ (id), begin_ (d.get()->data), 
            pos_ (begin_), end_ (begin_), max_ (begin_ + d->size)
        {}

        void Message::seek (size_t pos, SeekType dir)
        {
            using std::max;

            switch (dir)
            {
                case Beg: pos_ = begin_ + pos; break;
                case Cur: pos_ = pos_ + pos; break;
                case End: pos_ = end_ - pos; break;
            }

            end_ = max (pos_, end_); 

            assert (pos_ <= end_);
            assert (end_ <= max_);
        }
        
        template <> void Message::push <string> (string str)
        {
            using std::copy;

            pushVariableSize (str.size() + 1); 

            copy (str.begin(), str.end(), pos_);
            seek (str.size());

            push <uint8_t> (0); // null term
        }

        template <> void Message::pop <string> (string &str)
        {
            using std::copy;

            size_t size;
            uint8_t size1; get (size1);
            uint16_t size2; get (size2);

            // null terminated
            if (*(pos_ + size1) == 0)
            {
                size = size1;
                next <uint8_t> ();
            }
            else if (*(pos_ + size2) == 0)
            {
                size = size2;
                next <uint16_t> ();
            }
            else return;

            str.resize (size - 1);
            copy (pos_, pos_+size-1, str.begin());
            seek (size);
        }

        template <> void Message::push <QQuaternion> (QQuaternion value)
        {
            value.normalize(); // send normalized vector
            push <float> (value.x());
            push <float> (value.y());
            push <float> (value.z());
        }

        template <> void Message::pop <QQuaternion> (QQuaternion &value)
        {
            QQuaternion result;
            
            float x, y, z; 
            pop (x); pop (y); pop (z);
            
            if (isfinite (x) && !isnan (x) && 
                isfinite (y) && !isnan (y) && 
                isfinite (z) && !isnan (z))
            {
                result.setVector (x, y, z);

                // de-normalize (w = sqrt (1 - x*x - y*y - z*z))
                float w = 0.f, sq = x*x + y*y + z*z;

                if (sq < 1.f) w = sqrt (1.f - sq);
                else // numerically unstable as w->0
                    result.normalize ();

                result.setScalar (w);
            }

            value = result;
        }

        void Message::pushHeader (uint8_t flags, uint32_t seq, uint8_t extra)
        {
            push (flags);
            pushBigEndian (seq);
            push (extra);
        }

        void Message::popHeader (uint8_t &flags, uint32_t &seq, uint8_t &extra)
        {
            pop (flags);
            popBigEndian (seq);
            pop (extra);
        }

        void Message::pushMsgID (int priority, uint32_t id)
        {
            switch (priority)
            {
                case PacketInfo::HIGH: 
                    pushBigEndian <uint8_t> (id);
                    break;

                case PacketInfo::MEDIUM: 
                    pushBigEndian <uint16_t> (id | 0x0000FF00);
                    break;

                case PacketInfo::LOW: 
                    pushBigEndian <uint32_t> (id | 0xFFFF0000);
                    break;

                case PacketInfo::FIXED: 
                    pushBigEndian <uint32_t> (id);
                    break;
            }
        }

        void Message::popMsgID (int &priority, uint32_t &id)
        {
            uint32_t quad; getBigEndian (quad);

            if ((quad & 0xFF000000) == 0xFF000000)
            {
                if ((quad & 0xFFFF0000) == 0xFFFF0000)
                {
                    if ((quad & 0xFFFFFF00) == 0xFFFFFF00)
                    {
                        id = quad & 0xFFFFFFFF;
                        priority = PacketInfo::FIXED;
                        next <uint32_t> ();
                    }
                    else
                    {
                        id = quad & 0x0000FFFF;
                        priority = PacketInfo::LOW;
                        next <uint32_t> ();
                    }
                }
                else
                {
                    id = quad & 0x00FF0000;
                    priority = PacketInfo::MEDIUM;
                    next <uint16_t> ();
                }
            }
            else
            {
                id = quad & 0xFF000000;
                priority = PacketInfo::HIGH;
                next <uint8_t> ();
            }
        }

        void Message::pushBlock (uint8_t repetitions) 
        { 
            push (repetitions); 
        }

        void Message::popBlock (uint8_t &repetitions) 
        { 
            pop (repetitions); 
        }

        void Message::pushVariableSize (size_t size)
        {
            if (size < 256)
                pushBigEndian ((uint8_t) size);
            else if (size < 65536)
                pushBigEndian ((uint16_t) size);
            else // 4GB
                pushBigEndian ((uint32_t) size);
        }

        void Message::pushVariable (const std::vector <uint8_t> &buf)
        {
            using std::copy;

            pushVariableSize (buf.size());

            copy (buf.begin(), buf.end(), pos_);
            seek (buf.size());
        }

        void Message::popVariable1 (std::vector <uint8_t> &buf, uint8_t &size)
        {
            using std::copy;

            pop (size); 
            buf.resize (size);

            copy (pos_, pos_+size, buf.begin());
            seek (size);
        }

        void Message::popVariable2 (std::vector <uint8_t> &buf, uint16_t &size)
        {
            using std::copy;

            popBigEndian (size); 
            buf.resize (size);

            copy (pos_, pos_+size, buf.begin());
            seek (size);
        }

        void Message::print (std::ostream &out)
        {
            using std::ostream_iterator;
            using std::copy;
            using std::hex;

            copy (begin_, end_, ostream_iterator <int> (out << hex, " "));
        }

        pair <const char*, size_t> Message::sendBuffer () const
        {
            return make_pair ((const char *)begin_, end_ - begin_);
        }
                
        pair <char*, size_t> Message::recvBuffer () const
        {
            return make_pair ((char *)begin_, max_ - begin_);
        }
                

        //=============================================================================
        // Message factory

        MessageFactory::MessageFactory () : 
            error_ (false), parser_ ("message_template.msg")
        {
            error_ = parser_.parse (info_);

            for (int i=0; i < MESSAGE_POOL_SIZE; ++i)
                free_.push_back (new ByteBuffer (MAX_MESSAGE_SIZE));

            std::make_heap (free_.begin(), free_.end());
        }

        MessageFactory::~MessageFactory ()
        {
            for_each (free_.begin(), free_.end(), mem_fn (&ByteBuffer::dispose));
            for_each (used_.begin(), used_.end(), mem_fn (&ByteBuffer::dispose));
        }

        auto_ptr <Message> MessageFactory::create (uint32_t id, size_t size)
        {
            if (size == 0)
                size = MAX_MESSAGE_SIZE;

            if (next_free_buffer_()->size < size)
                add_free_buffer_ (size);

            ByteBuffer *buf = get_free_buffer_ ();

            shared_ptr <ByteBuffer> ptr 
                (buf, bind (&MessageFactory::set_free_buffer_, this, _1));

            auto_ptr <Message> msg (new Message (ptr, id));

            set_used_buffer_ (buf);

            return msg;
        }

        ByteBuffer *MessageFactory::next_free_buffer_ ()
        {
            return free_[0];
        }

        ByteBuffer *MessageFactory::add_free_buffer_ (size_t size)
        {
            ByteBuffer *buf = new ByteBuffer (size);

            free_.push_back (buf);
            push_heap (free_.begin(), free_.end());

            return buf;
        }

        ByteBuffer *MessageFactory::get_free_buffer_ ()
        {
            pop_heap (free_.begin(), free_.end());
            ByteBuffer *buf = free_.back ();

            free_.pop_back ();
            return buf;
        }

        void MessageFactory::set_free_buffer_ (ByteBuffer *buf)
        {
            free_.push_back (buf);
            push_heap (free_.begin(), free_.end());

            used_.erase (buf);
        }

        void MessageFactory::set_used_buffer_ (ByteBuffer *buf)
        {
            used_.insert (buf);
        }
    }
}

