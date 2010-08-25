/* provider.cpp -- UI services using Qt
 *
 *			Ryan McDougall
 */

#include "stdheaders.hpp"
#include "uiplugin/provider.hpp"

#include <QMainWindow>
#include <QGraphicsView>

//=============================================================================
//
namespace UIPlugin
{
    //=========================================================================

    NotificationProvider::NotificationProvider () :
        View::NotificationProvider ("qt-notification-provider", 10)
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
    }

    void NotificationProvider::finalize ()
    {
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
    
    //=========================================================================

    MainViewProvider::MainViewProvider () :
        View::ViewProvider ("qt-view-provider", 10), main_ (0)
    {
    }

    MainViewProvider::~MainViewProvider ()
    {
    }

    bool MainViewProvider::accepts (RequestType request) const
    {
        return (request == "main-view");
    }

    MainViewProvider::ResponseType MainViewProvider::retire (RequestType request)
    {
        return main_;
    }

    void MainViewProvider::initialize ()
    {
        main_ = new QMainWindow;
    }

    void MainViewProvider::finalize ()
    {
        delete main_;
    }

    void MainViewProvider::update ()
    {
    }

    //=========================================================================

    InWorldViewProvider::InWorldViewProvider () :
        View::ViewProvider ("qt-view-provider", 10), view_ (0)
    {
    }

    InWorldViewProvider::~InWorldViewProvider ()
    {
    }

    bool InWorldViewProvider::accepts (RequestType request) const
    {
        return (request == "inworld-graphics-view");
    }

    InWorldViewProvider::ResponseType InWorldViewProvider::retire (RequestType request)
    {
        return view_;
    }

    void InWorldViewProvider::initialize ()
    {
        view_ = new QGraphicsView (new QGraphicsScene);
    }

    void InWorldViewProvider::finalize ()
    {
        delete view_;
    }

    void InWorldViewProvider::update ()
    {
    }
}
