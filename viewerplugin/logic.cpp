/* logic.cpp -- defines the application's logic
 *
 *			Ryan McDougall
 */

#include "stdheaders.hpp"
#include "application.hpp"

#include "servicemanagers.hpp"
#include "llplugin/provider.hpp"
#include "uiplugin/provider.hpp"
#include "viewerplugin/logic.hpp"

#include <QtGui>

//=============================================================================

namespace ViewerPlugin
{
    Logic::Logic () : 
        state (1), scheduler (0), session (0), stream (0)
    {
    }

    void Logic::initialize (Framework::Scheduler *s)
    {
        cout << "module initialize" << endl;

        scheduler = s;
        service_session_manager->attach (new LLPlugin::SessionProvider);
        service_notification_manager->attach (new UIPlugin::NotificationProvider);
        service_action_manager->attach (new UIPlugin::ActionProvider);
        service_keybinding_manager->attach (new UIPlugin::KeyBindingProvider);
        service_settings_manager->attach (new UIPlugin::SettingsProvider);

        Model::Entity *logic = model_entities->get ("app-logic");
        if (logic->has ("app-control"))
        {
            Framework::Control *ctrl = logic->get <Framework::Control> ("app-control");

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
                    scheduler->enqueue (new Framework::Task 
                            (bind (&Logic::do_login, this, _1)));
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

                    cout << "quitting" << endl;
                    
                    stream->sendLogoutRequest ();
                    QApplication::exit ();
                }
                break;

            default:
                break;
        }
    }
    
    void Logic::do_login (frame_delta_t delta)
    {
        Connectivity::LoginParameters parms;

        //parms.insert ("first", "Test");
        //parms.insert ("last", "User");
        //parms.insert ("pass", "test");
        parms.insert ("first", "d");
        parms.insert ("last", "d");
        parms.insert ("pass", "d");
        parms.insert ("service", "http://localhost:8002");
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
}
