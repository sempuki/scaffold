/* servicemanagers.hpp -- link information for global service manager variables
 *
 *			Ryan McDougall
 */

#include "session.hpp"
#include "userview.hpp"

extern Scaffold::Connectivity::SessionManager  *service_session_manager;
extern Scaffold::View::NotificationManager     *service_notification_manager;
extern Scaffold::View::ActionManager           *service_action_manager;
extern Scaffold::View::KeyBindingManager       *service_keybinding_manager;
extern Scaffold::View::SettingsManager         *service_settings_manager;
extern Scaffold::View::ViewManager             *service_view_manager;
