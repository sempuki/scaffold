/* module.h -- exported function header
 *
 *			Ryan McDougall
 */

#ifndef SESSION_H_
#define SESSION_H_

namespace Scaffold
{
    namespace Connectivity
    {
        class Stream : public Tagged
        {
            public:
                Stream (const Tag &t) : Tagged (t) {}

                virtual bool isConnected () const = 0;

                virtual bool connect () = 0;
                virtual bool disconnect () = 0;

                virtual bool hasMessages () = 0;
                virtual bool waitForConnect () = 0;
                virtual bool waitForDisconnect () = 0;
                virtual bool waitForWrite () = 0;
                virtual bool waitForRead () = 0;
                
                virtual void pump () {}
        };

        class Session : public Tagged
        {
            public:
                Session (const Tag &t) : Tagged (t) {}

                virtual bool isConnected () const = 0;

                virtual bool connect () = 0;
                virtual bool disconnect () = 0;

                virtual Stream *stream () = 0;
        };

        typedef QMap <QString, QString> LoginParameters;
        typedef Service::Manager <LoginParameters, Session *> SessionManager;
        typedef SessionManager::ProviderType SessionProvider;
    }
}

#endif //_MODULE_H_
