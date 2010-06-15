/* llprovider.cpp -- main module
 *
 *			Ryan McDougall
 */

#include "stdheaders.hpp"
#include "capabilities.hpp"
#include "xmlrpc.hpp"

#include "llstack/provider.hpp"

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QCryptographicHash>
#include <QNetworkReply>
#include <QtConcurrentRun>
#include <QCoreApplication>

//=========================================================================
// pretty printers

void variant_print (const QVariant &v, const QString &p);
void variant_print (const QVariantMap &m, const QString &p);
void variant_print (const QVariantList &l, const QString &p);
void variant_print (const QStringList &l, const QString &p);
void variant_print (const QString &s, const QString &p);
void variant_print (bool v, const QString &p);
void variant_print (int v, const QString &p);

void variant_print (const QVariant &v, const QString &p)
{
    QString pre (p + " ");
    switch (v.type())
    {
        case QVariant::Bool:
            variant_print (v.toBool(), pre);
            break;

        case QVariant::List:
            variant_print (v.toList(), pre);
            break;

        case QVariant::StringList:
            variant_print (v.toStringList(), pre);
            break;

        case QVariant::Map:
            variant_print (v.toMap(), pre);
            break;

        case QVariant::String:
            variant_print (v.toString(), pre);
            break;

        case QVariant::Int:
            variant_print (v.toInt(), pre);
            break;

        default:
            std::cout << "no handler for type: " << v.type() << std::endl;
    }
}

void variant_print (const QVariantMap &m, const QString &p)
{
    QString pre (p + " ");
    foreach (QString s, m.keys())
    {
        std::cout << qPrintable (pre) << "(M)" << qPrintable (s) << ": ";
        variant_print (m[s], pre);
        std::cout << "\n";
    }
}

void variant_print (const QVariantList &l, const QString &p)
{
    QString pre (p + " ");
    foreach (QVariant v, l)
    {
        std::cout << qPrintable (pre) << "(L)";
        variant_print (v, pre);
        std::cout << "\n";
    }
}

void variant_print (const QStringList &l, const QString &p)
{
    QString pre (p + " ");
    foreach (QString s, l)
    {
        std::cout << qPrintable (pre) << "(L)";
        variant_print (s, pre);
        std::cout << "\n";
    }
}

void variant_print (const QString &s, const QString &p)
{
    QString pre (p + " ");
    std::cout << qPrintable (pre) << "(S) " << qPrintable (s);
}

void variant_print (bool v, const QString &p)
{
    QString pre (p + " ");
    std::cout << qPrintable (pre) << "(B) " << v;
}

void variant_print (int v, const QString &p)
{
    QString pre (p + " ");
    std::cout << qPrintable (pre) << "(I) " << v;
}


//=============================================================================
//
namespace Scaffold
{
    namespace LLStack
    {
        //=========================================================================
        // parsers for LLLogin

        static AgentParameters parse_agent_params (const QVariantMap &m);
        static StreamParameters parse_stream_params (const QVariantMap &m);
        static SessionParameters parse_session_params (const QVariantMap &m);
        static Buddy::List parse_buddies (const QVariantMap &m);
        static Buddy::List parse_buddy_list (const QVariantList &l);
        static Buddy parse_buddy (const QVariantMap &m);
        static InventorySkeleton parse_inventory (const QVariantMap &m);
        static InventorySkeleton parse_inventory_skeleton (const QVariantMap &m);
        static FolderSkeleton parse_root_folder (const QVariantList &l);
        static FolderSkeleton parse_folder_skeleton (const QVariantMap &m);
        static LoginParameters parse_login_params (const Connectivity::LoginParameters &p);

        static AgentParameters parse_agent_params (const QVariantMap &m)
        {
            AgentParameters params;

            params.buddies = parse_buddies (m);
            params.inventory = parse_inventory (m);

            params.first_name = m["first_name"].toString().toStdString();
            params.last_name = m["last_name"].toString().toStdString();

            params.home = m["home"].toString().toStdString();
            params.look_at = m["look_at"].toString().toStdString();
            params.start_location = m["start_location"].toString().toStdString();
            params.region_x = m["region_x"].toInt();
            params.region_y = m["region_y"].toInt();

            return params;
        }

        static StreamParameters parse_stream_params (const QVariantMap &m)
        {
            StreamParameters params;

            params.circuit_code = m["circuit_code"].toInt();
            params.agent_id.fromString (m["agent_id"].toString().toStdString());
            params.session_id.fromString (m["session_id"].toString().toStdString());

            return params;
        }

        static SessionParameters parse_session_params (const QVariantMap &m)
        {
            SessionParameters params;

            params.seed_capabilities = m["seed_capability"].toString();
            params.sim_ip = m["sim_ip"].toString().toStdString();
            params.sim_port = m["sim_port"].toInt();
            params.message = m["message"].toString().toStdString();

            return params;
        }

        static Buddy::List parse_buddies (const QVariantMap &m)
        {
            return parse_buddy_list (m["buddy-list"].toList());
        }

        static Buddy::List parse_buddy_list (const QVariantList &l)
        {
            Buddy::List b;
            foreach (QVariant v, l)
                b.push_back (parse_buddy (v.toMap()));
            return b;
        }

        static Buddy parse_buddy (const QVariantMap &m)
        {
            Buddy b;
            b.buddy_id = m["buddy_id"].toString().toStdString();
            b.buddy_rights_given = m["buddy_rights_given"].toInt();
            b.buddy_rights_has = m["buddy_rights_has"].toInt();
            return b;
        }

        static InventorySkeleton parse_inventory (const QVariantMap &m)
        {
            return parse_inventory_skeleton (m);
            // skip "opensim library"
        }

        static InventorySkeleton parse_inventory_skeleton (const QVariantMap &m)
        {
            InventorySkeleton is;

            is.root = parse_root_folder (m ["inventory-root"].toList());
            foreach (QVariant v, m["inventory-skeleton"].toList())
            {
                is.folders.push_back (parse_folder_skeleton (v.toMap()));

                if (is.folders.back().folder_id == is.root.folder_id) 
                    is.root = is.folders.back();
            }

            return is;
        }

        static FolderSkeleton parse_root_folder (const QVariantList &l)
        {
            FolderSkeleton f;
            f.folder_id = l.front().toMap()["folder_id"].toString().toStdString();
            return f;
        }

        static FolderSkeleton parse_folder_skeleton (const QVariantMap &m)
        {
            FolderSkeleton f;
            f.name = m["name"].toString().toStdString();
            f.folder_id = m["folder_id"].toString().toStdString();
            f.parent_id = m["parent_id"].toString().toStdString();
            f.type_default = m["type_default"].toInt();
            f.version = m["version"].toInt();
            return f;
        }

        static LoginParameters parse_login_params (const Connectivity::LoginParameters &p)
        {
            LoginParameters params;
            params.first = p["first"].toStdString(); 
            params.last = p["last"].toStdString(); 
            params.pass = p["pass"].toStdString();
            params.service = p["service"];

            return params;
        }


        //=========================================================================
        // Login object for LLSession

        Login::Login (Session *session) : 
            session_ (session), login_ (0), caps_ (0),
            http_ (new QNetworkAccessManager (this)) 
        {}

        bool Login::operator() (const LoginParameters &params)
        {
            // compose formal arguments from parameters
            QVariantMap args;

            QString first (params.first.c_str());
            QString last (params.last.c_str());
            QString pass (params.pass.c_str());

            args["first"] = first; 
            args["last"] = last;
            args["passwd"] = QString::fromAscii
                ("$1$"+QCryptographicHash::hash
                 (pass.toAscii(), QCryptographicHash::Md5).toHex());

            args["mac"] = "00:00:00:00:00:00"; // TODO
            args["id0"] = "00:00:00:00:00:00"; // TODO
            args["options"] = (QStringList()
                    << "inventory-root"
                    << "inventory-skeleton"
                    << "inventory-lib-root"
                    << "inventory-lib-owner"
                    << "inventory-skel-lib"
                    << "initial-outfit"
                    << "gestures"
                    << "event_categories"
                    << "event_notifications"
                    << "classified_categories"
                    << "buddy-list"
                    << "ui-config"
                    << "tutorial_settings"
                    << "login-flags"
                    << "global-textures");
            args["read_critical"] = true;
            args["viewer_digest"] = "";
            args["channel"] = "channel"; // TODO
            args["version"] = "version"; // TODO
            args["platform"] = "platform"; // TODO
            args["agree_to_tos"] = true;
            args["last_exec_event"] = 0;
            args["start"] = "last";

            // make login call
            Connectivity::XmlRpc::Client client (params.service, http_);
            login_ = client.call ("login_to_simulator", (QVariantList() << args));

            connect (login_->reply, SIGNAL (finished()), this, SLOT (on_login_result()));

            return true;
        }

        void Login::on_login_result ()
        {
            if (login_->reply->error() == QNetworkReply::NoError)
            {
                QVariantMap response (login_->result.toMap ());

                variant_print (response, " ");

                if (response["login"] == "true")
                {
                    // parse login return parameters
                    session_->streamparam_ = parse_stream_params (response);
                    session_->sessionparam_ = parse_session_params (response);
                    session_->agentparam_ = parse_agent_params (response);

                    // sanity checking
                    if ((session_->streamparam_.circuit_code == 0) ||
                            (session_->streamparam_.session_id.isNull()) ||
                            (session_->streamparam_.agent_id.isNull()))
                        std::cout << "session login error: stream parameters incorrect" << std::endl;

                    // set up world stream 
                    session_->stream_.setStreamParameters (session_->streamparam_);
                    session_->stream_.setSessionParameters (session_->sessionparam_);
                    session_->stream_.connect ();

                        // get capabilities
                    QUrl seedcap = session_->sessionparam_.seed_capabilities;
                    Connectivity::Capabilities::Client client (seedcap, http_);
                    caps_ = client.request ();

                    connect (caps_->reply, SIGNAL (finished()), this, SLOT (on_caps_result()));
                }
            }
            else
                std::cout << "xmlrpc login error: " << login_->reply->error() << std::endl;

            delete login_; login_ = 0;
        }

        void Login::on_caps_result ()
        {
            if (caps_->reply->error() == QNetworkReply::NoError)
            {
                // get caps
                session_->sessionparam_.capabilities = caps_->result;

                // update future
                session_->connected_ = true;
            }
            else
                std::cout << "seed caps error: " << caps_->reply->error() << std::endl;

            delete caps_; caps_ = 0;
        }

        //=========================================================================
        // Logout object for LLSession

        Logout::Logout (Session *session) :
            session_ (session)
        {
        }

        bool Logout::operator() ()
        {
            session_->stream_.disconnect();
            return true;
        }

        //=========================================================================
        // LLStream

        Stream::Stream () : 
            Connectivity::Stream ("ll-stream"), 
            idmap_ (get_msg_id_map ()),
            names_ (get_msg_name_map ()),
            udp_ (this)
        {
            QObject::connect (&udp_, SIGNAL(hostFound()), this, SLOT(on_host_found()));
            QObject::connect (&udp_, SIGNAL(connected()), this, SLOT(on_connected()));
            QObject::connect (&udp_, SIGNAL(disconnected()), this, SLOT(on_disconnected()));
            QObject::connect (&udp_, SIGNAL(bytesWritten(qint64)), this, SLOT(on_bytes_written(qint64)));
            QObject::connect (&udp_, SIGNAL(readyRead()), this, SLOT(on_ready_read()));
            QObject::connect (&udp_, SIGNAL(stateChanged(QAbstractSocket::SocketState)), 
                    this, SLOT(on_state_changed(QAbstractSocket::SocketState)));
            QObject::connect (&udp_, SIGNAL(error(QAbstractSocket::SocketError)), 
                    this, SLOT(on_error(QAbstractSocket::SocketError)));
        }

        Stream::~Stream ()
        {
        }

        void Stream::setStreamParameters (const StreamParameters &params)
        {
            streamparam_ = params;
        }

        void Stream::setSessionParameters (const SessionParameters &params)
        {
            sessionparam_ = params;
        }

        bool Stream::isConnected () const
        {
            return connected_;
        }

        bool Stream::connect ()
        {
            udp_.connectToHost (QString (sessionparam_.sim_ip.c_str()), 
                    sessionparam_.sim_port);

            connected_ = udp_.waitForConnected (1000);

            return connected_;
        }

        bool Stream::disconnect ()
        {
            udp_.disconnectFromHost ();

            connected_ = (udp_.state() == QAbstractSocket::UnconnectedState || 
                    udp_.waitForDisconnected (1000));

            return connected_;
        }

        bool Stream::hasMessages ()
        {
            return udp_.hasPendingDatagrams ();
        }

        bool Stream::waitForConnect ()
        {
            return udp_.waitForConnected ();
        }

        bool Stream::waitForDisconnect ()
        {
            return udp_.waitForDisconnected ();
        }

        bool Stream::waitForWrite ()
        {
            return udp_.waitForBytesWritten ();
        }

        bool Stream::waitForRead ()
        {
            return udp_.waitForReadyRead ();
        }
        
        void Stream::listen (msg_id_t id, Message::Listener listen)
        {
            if (!signals_.count (id))
                signals_.insert (make_pair (id, Message::Signal ()));

            signals_[id] += listen;
        }

        void Stream::sendUseCircuitCodePacket ()
        {
            Message m (factory_.create (UseCircuitCode, RELIABLE_FLAG));

            m.pushHeader ();
            
            m.push (streamparam_.circuit_code);
            m.push (streamparam_.session_id);
            m.push (streamparam_.agent_id);

            send_message_ (m);
        }

        void Stream::sendCompleteAgentMovementPacket ()
        {
            Message m (factory_.create (CompleteAgentMovement, RELIABLE_FLAG));

            m.pushHeader ();
            
            m.push (streamparam_.agent_id);
            m.push (streamparam_.session_id);
            m.push (streamparam_.circuit_code);

            send_message_ (m);
        }

        void Stream::sendAgentThrottlePacket ()
        {
            Message m (factory_.create (AgentThrottle, RELIABLE_FLAG | ZERO_CODE_FLAG));

            m.pushHeader ();

            m.push (streamparam_.agent_id);
            m.push (streamparam_.session_id);
            m.push (streamparam_.circuit_code);

            m.push <uint32_t> (0); // generation counter

            m.pushVariableSize (7);
            m.push (MAX_BPS * 0.1f);  // resend
            m.push (MAX_BPS * 0.1f);  // land
            m.push (MAX_BPS * 0.02f); // wind
            m.push (MAX_BPS * 0.02f); // cloud
            m.push (MAX_BPS * 0.25f); // task
            m.push (MAX_BPS * 0.26f); // texture
            m.push (MAX_BPS * 0.25f); // asset
            
            send_message_ (m);
        }

        void Stream::sendAgentWearablesRequestPacket ()
        {
            Message m (factory_.create (AgentWearablesRequest, RELIABLE_FLAG));

            m.pushHeader ();
            
            m.push (streamparam_.agent_id);
            m.push (streamparam_.session_id);

            send_message_ (m);
        }

        void Stream::sendRexStartupPacket (const string &state)
        {
            Message::ParamList param;

            param.push_back (streamparam_.agent_id);
            param.push_back (state);

            sendGenericMessage ("RexStartup", param);
        }

        void Stream::sendGenericMessage (const string &method, const Message::ParamList &param)
        {
            Message m (factory_.create (GenericMessage, RELIABLE_FLAG | ZERO_CODE_FLAG));

            m.pushHeader ();
            
            m.push (streamparam_.agent_id);
            m.push (streamparam_.session_id);
            m.push (UUID::random ()); // TransactionID

            m.push (method);
            m.push (UUID::random ()); // InvoiceID

            m.pushBlock (param.size());
            for_each (param.begin(), param.end(), 
                    bind (&Message::push <string>, &m, _1));

            send_message_ (m);
        }

        void Stream::on_host_found ()
        {
            cout << "udp host found" << endl;
        }

        void Stream::on_connected ()
        {
            cout << "udp connected" << endl;
        }

        void Stream::on_disconnected ()
        {
            cout << "udp disconnected" << endl;
        }

        void Stream::on_ready_read ()
        {
            cout << "ready for reading" << endl;

            while (udp_.hasPendingDatagrams ())
            {
                Message m (factory_.create ());
                recv_message_ (m);
            }
        }

        void Stream::on_bytes_written (qint64 bytes)
        {
            cout << "udp bytes written " << bytes << endl;
        }

        void Stream::on_state_changed (QAbstractSocket::SocketState state)
        {
            cout << "udp state changed " << state << endl;
        }

        void Stream::on_error (QAbstractSocket::SocketError err)
        {
            cout << "udp error: " << err << endl;
        }
                
        void Stream::send_message_ (Message m)
        {
            if (m.getFlags() & RELIABLE_FLAG)
                // wait for ack for this packet
                waiting_.insert (m.getSequence());

            pair <const char *, size_t> buf = m.sendBuffer ();
            udp_.write (buf.first, buf.second);
        }
                
        void Stream::recv_message_ (Message m)
        {
            pair <char *, size_t> buf = m.recvBuffer ();
            qint64 size = udp_.readDatagram (buf.first, buf.second);

            if (size > MESSAGE_HEADER_SIZE)
            {
                m.popHeader ();
                msg_id_t id = m.getID();

                if (id == PacketAck)
                {
                    uint32_t seq;
                    uint8_t blocks;

                    m.pop (blocks);
                    for (int i=0; i < blocks; ++i)
                    {
                        m.pop (seq);

                        // stop waiting for acked packets
                        waiting_.erase (seq);
                    }
                }
                else
                    // notify listeners
                    signals_[id] (m);
            }

            m.seek (0, Message::Beg);
        }

        //=========================================================================
        // LLSession

        Session::Session () : 
            Connectivity::Session ("ll-session"), 
            connected_ (false), login_ (this), logout_ (this)
        {
        }

        Session::~Session ()
        {
        }

        void Session::setLoginParameters (const Connectivity::LoginParameters &params)
        {
            loginparam_ = parse_login_params (params);
        }

        AgentParameters Session::getAgentParameters () const
        {
            return agentparam_;
        }

        SessionParameters Session::getSessionParameters () const
        {
            return sessionparam_;
        }

        StreamParameters Session::getStreamParameters () const
        {
            return streamparam_;
        }

        bool Session::isConnected() const
        {
            return connected_;
        }

        Stream *Session::stream ()
        {
            return &stream_;
        }

        bool Session::connect ()
        {
            if (connected_) return true;
            return login_ (loginparam_);
        }

        bool Session::disconnect ()
        {
            if (!connected_) return true;
            return logout_ ();
        }

        //=========================================================================
        // LLSessionProvider

        SessionProvider::SessionProvider () : 
            Connectivity::SessionProvider ("ll-session-provider", 10) 
        {
        }

        SessionProvider::~SessionProvider ()
        {
            session_.disconnect ();
        }

        bool SessionProvider::accepts (RequestType request) const
        {
            bool accept = false;

            if (request.contains ("first") && request.contains ("last") &&
                request.contains ("pass") && request.contains ("service"))
                accept = true;

            return accept;
        }

        SessionProvider::ResponseType SessionProvider::retire (RequestType request)
        {
            request_ = request;

            return QtConcurrent::run (this, &SessionProvider::establish_session_blocking_); 
        }

        void SessionProvider::initialize ()
        {
        }

        void SessionProvider::finalize ()
        {
        }

        void SessionProvider::update ()
        {
            if (request_.count ())
            {
                session_.setLoginParameters (request_);
                session_.connect ();

                request_.clear ();
            }
        }

        Connectivity::Session *SessionProvider::session ()
        {
            return &session_;
        }
        
        void SessionProvider::timerEvent (QTimerEvent *e)
        {
            timeout_ = true;
        }

        Connectivity::Session *SessionProvider::establish_session_blocking_ ()
        { 
            // wait for connection
            timeout_ = false; startTimer (5000);
            while (!session_.isConnected () && !timeout_);

            return session ();
        }
    }
}
