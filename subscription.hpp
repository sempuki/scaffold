/* subscription.hpp -- subscribe component of publish/subscribe architecture
 *
 *			Ryan McDougall
 */

#ifndef SUBSCRIPTION_H_
#define SUBSCRIPTION_H_

namespace Scaffold
{
    template <typename F>
        struct SubscriptionBase
        {
            typedef function <F> Function;
            typedef std::vector <Function> List;

            void operator+= (Function subscriber)
            {
                subscribers.push_back (subscriber);
            }

            List subscribers;
        };

    template <typename F>
        struct Subscription;

    template <typename T>
        struct Subscription <void (T)> : public SubscriptionBase <void (T)>
        {
            typedef SubscriptionBase <void (T)> BaseType;

            void operator() (T arg) const
            {
                typename BaseType::List::const_iterator i = BaseType::subscribers.begin();
                typename BaseType::List::const_iterator e = BaseType::subscribers.end();
                for (; i != e; ++i) (*i) (arg);
            }
        };

    template <typename T>
        struct Subscription <bool (T)> : public SubscriptionBase <bool (T)>
        {
            typedef SubscriptionBase <bool (T)> BaseType;

            void operator() (T arg) const
            {
                typename BaseType::List::const_iterator i = BaseType::subscribers.begin();
                typename BaseType::List::const_iterator e = BaseType::subscribers.end();
                for (; i != e; ++i) if ((*i) (arg)) break;
            }
        };

    template <typename T1, typename T2>
        struct Subscription <void (T1,T2)> : public SubscriptionBase <void (T1,T2)>
        {
            typedef SubscriptionBase <void (T1,T2)> BaseType;

            void operator() (T1 arg1, T2 arg2) const
            {
                typename BaseType::List::const_iterator i = BaseType::subscribers.begin();
                typename BaseType::List::const_iterator e = BaseType::subscribers.end();
                for (; i != e; ++i) (*i) (arg1, arg2);
            }
        };

    template <typename T1, typename T2>
        struct Subscription <bool (T1,T2)> : public SubscriptionBase <bool (T1,T2)>
        {
            typedef SubscriptionBase <bool (T1,T2)> BaseType;

            void operator() (T1 arg1, T2 arg2) const
            {
                typename BaseType::List::const_iterator i = BaseType::subscribers.begin();
                typename BaseType::List::const_iterator e = BaseType::subscribers.end();
                for (; i != e; ++i) if ((*i) (arg1, arg2)) break;
            }
        };
}

#endif //SUBSCRIPTION_H_
