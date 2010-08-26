/* logic.hpp -- defines the application's logic
 *
 *			Ryan McDougall
 */

#ifndef LOGIC_H_
#define LOGIC_H_

#include <QObject>

#include "application.hpp"
#include "session.hpp"

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

    class Logic : public QObject, public Framework::Module
    {
        Q_OBJECT

        public:
            Logic ();

            // module functions
            void initialize (Framework::Scheduler *s);
            void finalize ();

        public:
            // updated from the application entity
            void on_app_state_change (int state);
            void on_world_state_change (int state);

        public slots:
            // updated from the login UI
            void on_login (Connectivity::LoginParameters);
            void on_exit ();

        protected:
            // logic functions
            void do_login (Connectivity::LoginParameters);
            void do_start_world_stream ();
            void do_read_world_stream ();
            void do_logout ();
            void do_exit ();

        private:
            Framework::Scheduler    *scheduler;
            LLPlugin::Session       *session;
            LLPlugin::Stream        *stream;

            Framework::AppState     *app;
            Framework::WorldState   *world;
    };
}

#endif
