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
        state (1), scheduler (0), session (0), stream (0)
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

        // set up the main view with our login UI
        QMainWindow *main = static_cast <QMainWindow *> (service_view_manager->retire ("main-view"));
        main->setWindowTitle ("Viewer");
        main->setGeometry (100, 100, 400, 200);
        main->setCentralWidget (new LoginWidget);
        main->show();

        // get the application entity
        Model::Entity *app = model_entities->get ("application");
        if (app->has ("application-state-component"))
        {
            Framework::AppState *ctrl = app->get
                <Framework::AppState> ("application-state-component");

            ctrl->state.on_value_change += bind (&Logic::on_app_state_change, this, _1);
            ctrl->delta.on_value_change += bind (&Logic::on_frame_update, this, _1);
        }
    }

    void Logic::finalize ()
    {
        cout << "module finalize" << endl;
    }

    void Logic::on_app_state_change (int state)
    {
        cout << "app state changed: " << state << endl;
    }

    void Logic::on_frame_update (frame_delta_t delta)
    {
        // app logic state machine

        switch (state)
        {
            case 0:
                // null state
                break;

            case 1:
                {
                    state = 0;

                    cout << "attempting login" << endl;
                    //scheduler->enqueue (new Framework::Task 
                    //        (bind (&Logic::do_login, this, _1)));
                }
                break;

            case 2:
                {
                    state = 0;

                    cout << "begin streaming world" << endl;
                    scheduler->enqueue (new Framework::Task 
                            (bind (&Logic::do_start_world_stream, this, _1)));
                }
                break;

            case 3:
                {
                    state = 0;

                    cout << "waiting for world" << endl;
                    scheduler->enqueue (new Framework::Task 
                            (bind (&Logic::do_read_world_stream, this, _1)));
                }
                break;
                
            case 4:
                {
                    state = 0;

                    //scheduler->enqueue (new Framework::Task 
                    //        (bind (&Logic::do_logout, this, _1)));
                }
                break;

            default:
                break;
        }
    }
    
    void Logic::do_login (frame_delta_t delta, Connectivity::LoginParameters parms)
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

        if (session->isConnected() && stream->isConnected())
            state = 2;
        else
            state = 4;
    }

    void Logic::do_start_world_stream (frame_delta_t delta)
    {
        stream->sendUseCircuitCodePacket ();
        stream->sendCompleteAgentMovementPacket ();
        stream->sendAgentThrottlePacket ();
        stream->sendAgentWearablesRequestPacket ();
        stream->sendRexStartupPacket ("started"); 

        service_notification_manager->retire 
            (View::Notification (View::Notification::MESSAGE, "logged in!"));

        state = 3;
    }

    void Logic::do_read_world_stream (frame_delta_t delta)
    {
        if (stream->waitForRead ())
        {
            while (true); // we have no idea when to quit atm

            state = 4;
        }
    }

    void Logic::do_logout ()
    {
        cout << "quitting" << endl;

        stream->sendLogoutRequest ();
        QApplication::exit ();
    }
}
