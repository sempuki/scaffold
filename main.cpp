/* main.cpp -- main module
 *
 *			Ryan McDougall
 */

#include "stdheaders.hpp"
#include "application.hpp"
#include "session.hpp"
#include "llstack/provider.hpp"


//=============================================================================
// Framework

using namespace std;
using namespace std::tr1;

using namespace Scaffold;
using namespace Scaffold::Model;
using namespace Scaffold::Framework;
using namespace Scaffold::Connectivity;

Scene           entities;
EntityFactory   factory;

SessionManager  *service_session_manager;

//=============================================================================
// My stuff

struct Logic : public Module
{
    int         state;
    Scheduler   *scheduler;

    LLStack::Session    *session;
    LLStack::Stream     *stream;

    Logic () : 
        state (1), scheduler (0), session (0), stream (0)
    {}

    void initialize (Scheduler *s)
    {
        cout << "module initialize" << endl;

        scheduler = s;
        service_session_manager->attach (new LLStack::SessionProvider);

        Entity *logic = entities.get ("app-logic");
        if (logic->has ("app-control"))
        {
            Control *ctrl = logic->get <Control> ("app-control");

            ctrl->state.on_value_change += bind (&Logic::on_app_state_change, this, _1);
            ctrl->delta.on_value_change += bind (&Logic::on_frame_update, this, _1);
        }
    }

    void finalize ()
    {
        cout << "module finalize" << endl;
    }

    void on_app_state_change (int state)
    {
        cout << "app state changed: " << state << endl;
    }

    void on_frame_update (frame_delta_t delta)
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
                    scheduler->enqueue (new Task 
                            (bind (&Logic::do_login, this, _1)));
                }
                break;

            case 2:
                {
                    state = 0;

                    cout << "begin streaming world" << endl;
                    scheduler->enqueue (new Task 
                            (bind (&Logic::do_start_world_stream, this, _1)));
                }
                break;

            case 3:
                {
                    state = 0;

                    cout << "waiting for world" << endl;
                    scheduler->enqueue (new Task 
                            (bind (&Logic::do_read_world_stream, this, _1)));
                }
                break;
                
            case 4:
                {
                    state = 0;

                    cout << "quitting" << endl;
                    QApplication::exit();
                }
                break;

            default:
                break;
        }
    }
    
    void do_login (frame_delta_t delta)
    {
        LoginParameters parms;
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
        
        QFuture <Session *> login = service_session_manager->retire (parms);

        if (!login.isCanceled()) 
        {
            Session *s = login.result();
            if (s->name() == "ll-session")
            {
                session = static_cast <LLStack::Session *> (s);
                stream = static_cast <LLStack::Stream *> (s->stream ());
            }
        }

        if (session->isConnected() && stream->isConnected())
            state = 2;
        else
            state = 4;
    }

    void do_start_world_stream (frame_delta_t delta)
    {
        stream->sendUseCircuitCodePacket ();
        stream->sendCompleteAgentMovementPacket ();
        stream->sendAgentThrottlePacket ();
        stream->sendAgentWearablesRequestPacket ();
        stream->sendRexStartupPacket ("started"); 

        state = 3;
    }

    void do_read_world_stream (frame_delta_t delta)
    {
        if (stream->waitForRead ())
        {
            while (true); // we have no idea when to quit atm

            state = 4;
        }
    }
};

// A sample component
struct MyComponent : public Component
{
    MyComponent (const Tag &id) : 
        Component (id), 
        lartiness ("lartiness", -1),
        foolevel ("foo-level", 1.0)
    {
        observe (lartiness);
        observe (foolevel);
    }

    Property <int> lartiness;
    Property <float> foolevel;
};

//=============================================================================
// Main entry point
int main (int argc, char** argv)
{
    // components
    const char *mycomp_types[] = { "my-type", "other-type" };
    const char *ctrl_types[] = { "app-control-type" };

    factory.attach (new ComponentFactory <Control> ("app-control", ctrl_types, 1));
    factory.attach (new ComponentFactory <MyComponent> ("my-component", mycomp_types, 2));

    // entities
    entities.insert (factory.create ("test", "my-type"));
    entities.insert (factory.create ("app-logic", "app-control-type"));

    // application
    Control *ctrl = entities.get ("app-logic")->get <Control> ("app-control");
    Application app (argc, argv, ctrl);

    // services
    service_session_manager = new SessionManager;
    app.attach (service_session_manager);

    // modules
    app.attach (new Logic);

    return app.exec ();
}
