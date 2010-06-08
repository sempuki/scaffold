/* main.cpp -- main module
 *
 *			Ryan McDougall
 */

#include "stdheaders.hpp"
#include "application.hpp"
#include "session.hpp"
#include "llstack/provider.hpp"
#include "llstack/message.hpp"


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
        state (0), scheduler (0), session (0), stream (0)
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
        if (stream)
            stream->pump();

        switch (state)
        {
            case 0:
                {
                    cout << "attempting login" << endl;
                    state = 1;

                    scheduler->enqueue 
                        (new Task (bind (&Logic::do_lllogin_test, this, _1)));
                }
                break;

            case 1:
                {
                    if (session) 
                    {
                        cout << "got session" << endl;

                        if (session->isConnected() && stream->isConnected())
                        {
                            cout << "logged in" << endl;

                            stream->SendUseCircuitCodePacket ();
                            stream->SendCompleteAgentMovementPacket ();
                            stream->SendAgentThrottlePacket ();
                            stream->SendAgentWearablesRequestPacket ();
                            stream->SendRexStartupPacket ("started"); 
                        }
                        else
                            cout << "login failed" << endl;

                        state = 2;
                    }
                }
                break;

            case 2:
                {
                    cout << "quitting" << endl;
                    state = 3;

                    QApplication::exit();
                }
                break;
                
            default:
                break;
        }
    }
    
    void do_lllogin_test (frame_delta_t delta)
    {
        LoginParameters parms;
        parms.insert ("first", "r");
        parms.insert ("last", "r");
        parms.insert ("pass", "r");
        //parms.insert ("service", "http://localhost:8002");
        parms.insert ("service", "http://home.hulkko.net:9007");
        
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
