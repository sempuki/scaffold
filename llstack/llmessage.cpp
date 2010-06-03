/* main.cpp -- main module
 *
 *			Ryan McDougall
 */

#include "stdheaders.hpp"
#include "llstack/llmessage.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/io.hpp>


//=============================================================================

namespace Scaffold
{
    namespace LLStack 
    {
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

        Message *MessageFactory::create (const string &name, size_t size)
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

