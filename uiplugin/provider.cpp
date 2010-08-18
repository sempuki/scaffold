/* provider.cpp -- UI services using Qt
 *
 *			Ryan McDougall
 */

#include <QDialog>
#include <QLayout>
#include <QLabel>

#include "stdheaders.hpp"
#include "uiplugin/provider.hpp"

//=============================================================================
//
namespace Scaffold
{
    namespace UIPlugin
    {
        //=========================================================================

        NotificationProvider::NotificationProvider () :
            View::NotificationProvider ("qt-notification-provider", 10),
            parent_ (0)
        {
        }

        NotificationProvider::~NotificationProvider ()
        {
        }

        bool NotificationProvider::accepts (RequestType request) const
        {
            return true;
        }

        NotificationProvider::ResponseType NotificationProvider::retire (RequestType request)
        {
            notices_.push_back (request);
        }

        void NotificationProvider::initialize ()
        {
            parent_ = new QWidget;
        }

        void NotificationProvider::finalize ()
        {
            delete parent_;
        }

        void NotificationProvider::update ()
        {
            for_each (notices_.begin(), notices_.end(),
                    bind (&NotificationProvider::dispatch_notice_, this, _1));

            notices_.clear ();
        }
        
        void NotificationProvider::dispatch_notice_ (const View::Notification &note)
        {
            switch (note.type ())
            {
                case View::Notification::MESSAGE:
                case View::Notification::PROGRESS:
                case View::Notification::CHOICE:
                case View::Notification::INPUT:
                    cout << "notification: " << qPrintable (note.message()) << endl;
                    break;

            }
        }
        
        //=========================================================================

        ActionProvider::ActionProvider () :
            View::ActionProvider ("qt-action-provider", 10)
        {
        }

        ActionProvider::~ActionProvider ()
        {
        }

        bool ActionProvider::accepts (RequestType request) const
        {
            return true;
        }

        ActionProvider::ResponseType ActionProvider::retire (RequestType request)
        {
        }

        void ActionProvider::initialize ()
        {
        }

        void ActionProvider::finalize ()
        {
        }

        void ActionProvider::update ()
        {
        }

        //=========================================================================

        KeyBindingProvider::KeyBindingProvider () :
            View::KeyBindingProvider ("qt-keybinding-provider", 10)
        {
        }

        KeyBindingProvider::~KeyBindingProvider ()
        {
        }

        bool KeyBindingProvider::accepts (RequestType request) const
        {
            return true;
        }

        KeyBindingProvider::ResponseType KeyBindingProvider::retire (RequestType request)
        {
        }

        void KeyBindingProvider::initialize ()
        {
        }

        void KeyBindingProvider::finalize ()
        {
        }

        void KeyBindingProvider::update ()
        {
        }

        //=========================================================================

        SettingsProvider::SettingsProvider () :
            View::SettingsProvider ("qt-settings-provider", 10)
        {
        }
        
        SettingsProvider::~SettingsProvider ()
        {
        }

        bool SettingsProvider::accepts (RequestType request) const
        {
            return true;
        }

        SettingsProvider::ResponseType SettingsProvider::retire (RequestType request)
        {
        }

        void SettingsProvider::initialize ()
        {
        }

        void SettingsProvider::finalize ()
        {
        }

        void SettingsProvider::update ()
        {
        }
    }
}
