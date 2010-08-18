/* session.h -- session and stream abstraction
 *
 *			Ryan McDougall
 */

#include <QWidget>
#include <QAction>
#include <QKeySequence>

#include "stdheaders.hpp"
#include "application.hpp"
#include "service.hpp"
#include "userview.hpp"

namespace Scaffold
{
    namespace View
    {
        Notification::Notification (Notification::Type type, QString message, QWidget *widget) :
            type_ (type), message_ (message), widget_ (widget)
        {
        }

        Notification::Type Notification::type () const
        {
            return type_;
        }

        QString Notification::message () const
        {
            return message_;
        }

        QWidget *Notification::widget () const
        {
            return widget_;
        }
        
        Action::Action (const string &name, const string &location, QAction *action) :
            name_ (name), location_ (location), action_ (action)
        {
        }

        string Action::name () const
        {
            return name_;
        }

        string Action::location () const
        {
            return location_;
        }

        QAction *Action::widget () const
        {
            return action_;
        }
    }
}
