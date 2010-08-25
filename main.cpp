/* main.cpp -- main module
 *
 *			Ryan McDougall
 */

#include "stdheaders.hpp"
#include "application.hpp"

#include "servicemanagers.hpp"
#include "llplugin/provider.hpp"
#include "uiplugin/provider.hpp"
#include "viewerplugin/logic.hpp"

//=============================================================================
// Framework Globals

Scaffold::Model::Scene                  *model_entities;
Scaffold::Model::EntityFactory          *model_entity_factory;
Scaffold::Connectivity::SessionManager  *service_session_manager;
Scaffold::View::NotificationManager     *service_notification_manager;
Scaffold::View::ActionManager           *service_action_manager;
Scaffold::View::KeyBindingManager       *service_keybinding_manager;
Scaffold::View::SettingsManager         *service_settings_manager;
Scaffold::View::ViewManager             *service_view_manager;

//=============================================================================
// Main entry point
int main (int argc, char** argv)
{
    using namespace Scaffold;

    // entities
    model_entities = new Model::Scene;
    model_entity_factory = new Model::EntityFactory;

    // application
    Application app (argc, argv);

    // services
    service_session_manager = new Connectivity::SessionManager;
    service_notification_manager = new View::NotificationManager;
    service_action_manager = new View::ActionManager;
    service_keybinding_manager = new View::KeyBindingManager;
    service_settings_manager = new View::SettingsManager;
    service_view_manager = new View::ViewManager;

    app.attach (service_session_manager);
    app.attach (service_notification_manager);
    app.attach (service_action_manager);
    app.attach (service_keybinding_manager);
    app.attach (service_settings_manager);
    app.attach (service_view_manager);

    // modules
    app.attach (new ViewerPlugin::Logic);

    return app.exec ();
}
