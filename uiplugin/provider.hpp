/* provider.hpp -- UI services using Qt
 *
 */

#ifndef UI_PROVIDER_H_
#define UI_PROVIDER_H_

#include "stdheaders.hpp"
#include "application.hpp"
#include "service.hpp"
#include "userview.hpp"

namespace UIPlugin
{
    using namespace Scaffold;

    //=========================================================================

    class NotificationProvider : public QObject, public View::NotificationProvider
    {
        public:
            NotificationProvider ();
            ~NotificationProvider ();

            bool accepts (RequestType request) const;
            ResponseType retire (RequestType request);

            void initialize ();
            void finalize ();
            void update ();

        private:
            void dispatch_notice_ (const View::Notification &note);

        private:
            View::Notification::List  notices_;
    };

    //=========================================================================

    class ActionProvider : public QObject, public View::ActionProvider
    {
        public:
            ActionProvider ();
            ~ActionProvider ();

            bool accepts (RequestType request) const;
            ResponseType retire (RequestType request);

            void initialize ();
            void finalize ();
            void update ();
    };

    //=========================================================================

    class KeyBindingProvider : public QObject, public View::KeyBindingProvider
    {
        public:
            KeyBindingProvider ();
            ~KeyBindingProvider ();

            bool accepts (RequestType request) const;
            ResponseType retire (RequestType request);

            void initialize ();
            void finalize ();
            void update ();
    };

    //=========================================================================

    class SettingsProvider : public QObject, public View::SettingsProvider
    {
        public:
            SettingsProvider ();
            ~SettingsProvider ();

            bool accepts (RequestType request) const;
            ResponseType retire (RequestType request);

            void initialize ();
            void finalize ();
            void update ();
    };
}

#endif
