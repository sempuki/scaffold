/* logic.cpp -- defines the application's logic
 *
 *			Ryan McDougall
 */

#include <QtGui>

#include "stdheaders.hpp"
#include "application.hpp"

#include "servicemanagers.hpp"
#include "llplugin/provider.hpp"
#include "uiplugin/provider.hpp"
#include "viewerplugin/logic.hpp"
#include "viewerplugin/ui_login.hpp"

//=============================================================================

namespace ViewerPlugin
{
    Logic::Logic () : 
        scheduler (0), session (0), stream (0), app (0), world (0)
    {
    }

    void Logic::initialize (Framework::Scheduler *s)
    {
        scheduler = s;
        
        cout << "module initialize" << endl;

        // add our custom providers to the service managers
        service_session_manager->attach (new LLPlugin::SessionProvider);
        service_notification_manager->attach (new UIPlugin::NotificationProvider);
        service_action_manager->attach (new UIPlugin::ActionProvider);
        service_keybinding_manager->attach (new UIPlugin::KeyBindingProvider);
        service_settings_manager->attach (new UIPlugin::SettingsProvider);
        service_view_manager->attach (new UIPlugin::MainViewProvider);
        service_view_manager->attach (new UIPlugin::InWorldViewProvider);

        // get the application entity
        Model::Entity *app_entity = model_entities->get ("application");

        if (app_entity->has ("application-state-component"))
        {
            app = app_entity->get <Framework::AppState> ("application-state-component");
            app->state.on_value_change += bind (&Logic::on_app_state_change, this, _1);
        }

        if (app_entity->has ("application-worldstate-component"))
        {
            world = app_entity->get <Framework::WorldState> ("application-worldstate-component");
            world->state.on_value_change += bind (&Logic::on_world_state_change, this, _1);
        }
    }

    void Logic::finalize ()
    {
        cout << "module finalize" << endl;
    }

    void Logic::on_app_state_change (int state)
    {
        cout << "app state changed: " << state << endl;

        switch (state)
        {
            case Framework::AppState::RUNNING:
                {
                    world->state = Framework::WorldState::OUT;
                }
                break;
        }
    }

    void Logic::on_world_state_change (int state)
    {
        cout << "world state changed: " << state << endl;

        switch (state)
        { 
            case Framework::WorldState::OUT:
                {
                    world->state = Framework::WorldState::LOGIN;
                }
                break;

            case Framework::WorldState::LOGIN:
                {
                    cout << "logging in" << endl;

                    // set up the main view with our login UI
                    QMainWindow *main = static_cast <QMainWindow *> 
                        (service_view_manager->retire ("main-view"));

                    LoginWidget *widget = new LoginWidget;
                    connect (widget, SIGNAL(exit()), this, SLOT(on_exit()));
                    connect (widget, SIGNAL(login(QMap<QString,QString>)), 
                            this, SLOT(on_login(QMap<QString,QString>)));

                    main->setWindowTitle ("Viewer");
                    main->setGeometry (100, 100, 400, 200);
                    main->setCentralWidget (widget);
                    main->show();
                }
                break;

            case Framework::WorldState::IN:
                {
                    cout << "streaming in-world!" << endl;
                }
                break;

            case Framework::WorldState::LOGOUT:
                {
                    cout << "logging out" << endl;
                    
                    do_logout ();
                }
                break;


            case Framework::WorldState::EXIT:
                {
                    cout << "exiting" << endl;

                    do_exit ();
                }
                break;
        }
    }

    void Logic::on_login (Connectivity::LoginParameters params)
    {
        Framework::Task *task;

        task = new Framework::Task (bind (&Logic::do_login, this, params));
        task->chain (new Framework::Task (bind (&Logic::do_start_world_stream, this)));
        task->chain (new Framework::Task (bind (&Logic::do_read_world_stream, this)));

        scheduler->enqueue (task);
    }

    void Logic::on_exit ()
    {
        world->state = Framework::WorldState::LOGOUT;
        world->state = Framework::WorldState::EXIT;
    }
    
    void Logic::do_login (Connectivity::LoginParameters parms)
    {
        //parms.insert ("first", "Test");
        //parms.insert ("last", "User");
        //parms.insert ("pass", "test");
        ////parms.insert ("first", "d");
        ////parms.insert ("last", "d");
        ////parms.insert ("pass", "d");
        ////parms.insert ("service", "http://localhost:8002");
        //parms.insert ("service", "http://home.hulkko.net:9007");
        //parms.insert ("service", "http://world.realxtend.org:9000");
        //parms.insert ("service", "http://world.evocativi.com:8002");
        
        QFuture <Connectivity::Session *> login = service_session_manager->retire (parms);

        if (!login.isCanceled()) 
        {
            Connectivity::Session *s = login.result();
            if (s->name() == "ll-session")
            {
                session = static_cast <LLPlugin::Session *> (s);
                stream = static_cast <LLPlugin::Stream *> (s->stream ());
            }
        }

        if (!session->isConnected() || !stream->isConnected())
            world->state = Framework::WorldState::EXIT;
    }

    void Logic::do_start_world_stream ()
    {
        stream->sendUseCircuitCodePacket ();
        stream->sendCompleteAgentMovementPacket ();
        stream->sendAgentThrottlePacket ();
        stream->sendAgentWearablesRequestPacket ();
        stream->sendRexStartupPacket ("started"); 

        service_notification_manager->retire 
            (View::Notification (View::Notification::MESSAGE, "logged in!"));
    }

    void Logic::do_read_world_stream ()
    {
        if (stream->waitForRead ())
            world->state = Framework::WorldState::IN;
    }

    void Logic::do_logout ()
    {
        if (stream && stream->isConnected())
            stream->sendLogoutRequest ();
    }

    void Logic::do_exit ()
    {
        QApplication::exit ();
    }
}
