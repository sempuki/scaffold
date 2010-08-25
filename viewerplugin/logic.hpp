/* logic.hpp -- defines the application's logic
 *
 *			Ryan McDougall
 */

#ifndef LOGIC_H_
#define LOGIC_H_

namespace Scaffold
{
    namespace Framework
    {
        class Scheduler;
    }
}

namespace LLPlugin
{
    class Session;
    class Stream;
}

namespace ViewerPlugin
{
    using namespace Scaffold;

    struct Logic : public Framework::Module
    {
        int state;

        Framework::Scheduler    *scheduler;
        LLPlugin::Session       *session;
        LLPlugin::Stream        *stream;

        Logic ();

        // module functions
        void initialize (Framework::Scheduler *s);
        void finalize ();

        // updated from the control entity
        void on_app_state_change (int state);
        void on_frame_update (frame_delta_t delta);

        // logic functions
        void do_login (frame_delta_t delta, Connectivity::LoginParameters parms);
        void do_start_world_stream (frame_delta_t delta);
        void do_read_world_stream (frame_delta_t delta);
        void do_logout ();
    };
}

#endif
