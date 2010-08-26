/* application.hpp -- application level abstraction
 *
 *			Ryan McDougall
 */


#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <QApplication>
#include <QThread>
#include <QTimer>
#include <QTime>

#include "stdheaders.hpp"
#include "task.hpp"
#include "module.hpp"
#include "model.hpp"
#include "service.hpp"

namespace Scaffold
{
    namespace Framework
    {
        // share application state through application entity
        struct AppState : public Model::Component
        {
            enum 
            {
                INITIAL,
                READY,
                RUNNING,
                FINISHED,
                FINAL,
                ERROR
            };

            AppState (const Tag &id) :
                Model::Component (id), 
                state ("application-state", INITIAL),
                delta ("frame-duration")
            {}

            Model::Property <int> state;
            Model::Property <frame_delta_t> delta;
        };

        // share in-world state through application entity
        struct WorldState : public Model::Component
        {
            enum
            {
                IN,
                OUT,
                LOGIN,
                LOGOUT,
                ETHER,
                EXIT,
                ERROR
            };

            WorldState (const Tag &id) :
                Model::Component (id),
                state ("world-state", OUT)
            {}

            Model::Property <int> state;
        };

        class Module;
    }

    // run blocking module code one a separate thread
    class DispatchThread : public QThread
    {
        Q_OBJECT

        public:
            DispatchThread ();

            void setScheduler (Framework::Scheduler *s);
            void setFrameDelta (frame_delta_t t);

            void run ();
            void stop ();

        private:
            Framework::Scheduler  *scheduler_;

            frame_delta_t   delta_;
            bool            stop_;
    };

    // combine main-loop, modules, workers, scheduler, and application entity
    // application entity is named "application"
    class Application : public QApplication
    {
        Q_OBJECT

        public:
            Application (int &argc, char **argv);
            ~Application ();

        public:
            void attach (Framework::Module *module);
            void attach (Framework::Worker *worker);

            int exec ();

            protected slots:
                void update ();

        private:
            void do_worker_pump ();
            void do_worker_delete ();

            void do_module_initialize ();
            void do_module_finalize ();
            void do_module_delete ();

            void do_entity_initialize ();

        private:
            Framework::Worker::List workers_;
            Framework::Module::List modules_;

            Framework::AppState     *app_;
            Framework::WorldState   *world_;
            Framework::Scheduler    scheduler_;
            DispatchThread          thread_;

            QTimer  frame_timer_;
            QTime   time_;
    };
}

#endif
