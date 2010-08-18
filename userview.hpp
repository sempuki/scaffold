/* session.h -- session and stream abstraction
 *
 *			Ryan McDougall
 */

#ifndef USERVIEW_H_
#define USERVIEW_H_

class QWidget;
class QAction;
class QKeySequence;

namespace Scaffold
{
    namespace View
    {
        class Notification
        {
            public:
                typedef std::vector <Notification> List;

                // depending on type, QWidget is expected to be 
                // QProgressBar, QGroupBox, QLineEdit
                enum Type { MESSAGE, PROGRESS, CHOICE, INPUT };

                Notification (Type type, QString message, QWidget *widget = 0);

                Type type () const;
                QString message () const;
                QWidget *widget () const;

            private:
                Type    type_;
                QString message_;
                QWidget *widget_;
        };

        class Action
        {
            public:
                typedef std::vector <Action> List;

                // name is used in keybinding, location gives a placement hint
                Action (const string &name, const string &location, QAction *action);

                string name () const;
                string location () const;
                QAction *widget () const;

            private:
                string  name_;
                string  location_;
                QAction *action_;
        };

        typedef std::list <string, QKeySequence> KeyBinding;
        
        // Notify the user of important events
        typedef Service::Manager <Notification, void> NotificationManager;
        typedef NotificationManager::ProviderType NotificationProvider;

        // Display invokable actions in menus or docks
        typedef Service::Manager <Action, void> ActionManager;
        typedef ActionManager::ProviderType ActionProvider;

        // Change key bindings for invokable actions
        typedef Service::Manager <KeyBinding, void> KeyBindingManager;
        typedef KeyBindingManager::ProviderType KeyBindingProvider;

        // Display custom settings widgets for user configuration
        typedef Service::Manager <QWidget *, void> SettingsManager;
        typedef SettingsManager::ProviderType SettingsProvider;
    }
}

#endif //_MODULE_H_
