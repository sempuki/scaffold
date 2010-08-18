/* service.h -- abstraction for application services
 *
 *			Ryan McDougall
 */

#ifndef SERVICE_H_
#define SERVICE_H_

#include <QFuture>
#include <QtConcurrentRun>

namespace Scaffold
{
    namespace Service
    {
        // Generic Interface for service providers

        template <typename Request, typename Response>
        class Provider : public Tagged, public Framework::Plugin
        {
            public:
                typedef std::vector <Provider <Request,Response> *> List;
                typedef Response ResponseType;
                typedef typename rvalue <Request>::type RequestType;

                Provider (const Tag &t, int priority = 0)
                    : Tagged (t), priority_ (priority) {}

                int priority() const { return priority_; }

                virtual bool accepts (RequestType r) const = 0;
                virtual ResponseType retire (RequestType r) = 0;

                virtual void initialize () = 0;
                virtual void finalize () = 0;
                virtual void update () = 0;

            private:
                int priority_;
        };

        // Manager for multiple providers for a single service

        template <typename Request, typename Response>
        class Manager : public Framework::Worker
        {
            public:
                typedef Provider <Request, Response> ProviderType;
                typedef Response ResponseType;
                typedef typename rvalue <Request>::type RequestType;
                typedef std::vector <Manager <Request,Response> > List;

                virtual ~Manager()
                {
                    typename ProviderType::List::const_iterator i = providers_.begin();
                    typename ProviderType::List::const_iterator e = providers_.end();

                    for (; i != e; ++i) 
                    {
                        (*i)->finalize ();
                        delete *i;
                    }
                }

                virtual void attach (ProviderType *provider)
                {
                    provider->initialize ();
                    providers_.push_back (provider);
                }

                virtual void pump ()
                {
                    typename ProviderType::List::const_iterator i = providers_.begin();
                    typename ProviderType::List::const_iterator e = providers_.end();

                    for (; i != e; ++i) 
                        (*i)->update ();
                }

                virtual ProviderType *get (const Tag &t) const
                {
                    typename ProviderType::List::const_iterator i = providers_.begin();
                    typename ProviderType::List::const_iterator e = providers_.end();

                    for (; i != e; ++i) 
                        if ((*i)->tag() == t) 
                            return *i;
                }

                virtual ResponseType retire (RequestType r)
                {
                    typename ProviderType::List::iterator i = providers_.begin();
                    typename ProviderType::List::iterator e = providers_.end();

                    for (; i != e; ++i) 
                        if ((*i)->accepts (r))
                            return (*i)->retire (r);

                    return ResponseType ();
                }

            private:
                typename ProviderType::List providers_;
        };
    }
}

#endif //_MODULE_H_
