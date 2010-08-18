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

//=============================================================================
// Main entry point
int main (int argc, char** argv)
{
    using namespace Scaffold;

    // entities
    model_entities = new Model::Scene;
    model_entity_factory = new Model::EntityFactory;

    // control entity
    const char *ctrl_types[] = { "app-control-type" };

    model_entity_factory->attach 
        (new Model::ComponentFactory <Framework::Control> 
         ("app-control", ctrl_types, 1));

    model_entities->insert 
        (model_entity_factory->create 
         ("app-logic", "app-control-type"));

    Framework::Control *ctrl = model_entities->get 
        ("app-logic")->get <Framework::Control> ("app-control");

    // application
    Application app (argc, argv, ctrl);

    // services
    service_session_manager = new Connectivity::SessionManager;
    service_notification_manager = new View::NotificationManager;
    service_action_manager = new View::ActionManager;
    service_keybinding_manager = new View::KeyBindingManager;
    service_settings_manager = new View::SettingsManager;
    
    app.attach (service_session_manager);
    app.attach (service_notification_manager);
    app.attach (service_action_manager);
    app.attach (service_keybinding_manager);
    app.attach (service_settings_manager);

    // modules
    app.attach (new ViewerPlugin::Logic);

    return app.exec ();
}
