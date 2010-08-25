/* provider.hpp -- UI services using Qt
 *
 */

#ifndef UI_PROVIDER_H_
#define UI_PROVIDER_H_

#include "stdheaders.hpp"
#include "application.hpp"
#include "service.hpp"
#include "userview.hpp"

class QMainWindow;
class QGraphicsView;

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

    //=========================================================================

    class MainViewProvider : public QObject, public View::ViewProvider
    {
        public:
            MainViewProvider ();
            ~MainViewProvider ();

            bool accepts (RequestType request) const;
            ResponseType retire (RequestType request);

            void initialize ();
            void finalize ();
            void update ();

        private:
            QMainWindow     *main_;
    };

    //=========================================================================

    class InWorldViewProvider : public QObject, public View::ViewProvider
    {
        public:
            InWorldViewProvider ();
            ~InWorldViewProvider ();

            bool accepts (RequestType request) const;
            ResponseType retire (RequestType request);

            void initialize ();
            void finalize ();
            void update ();

        private:
            QGraphicsView   *view_;
    };
}

#endif
