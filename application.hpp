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
        class Module; 

        // share application state through a control component
        struct Control : public Model::Component
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

            Control (const Tag &id) : 
                Model::Component (id), 
                state ("application-state", INITIAL),
                delta ("frame-duration")
            {}

            Model::Property <int> state;
            Model::Property <frame_delta_t> delta;
        };
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

    // combine main-loop, modules, workers, scheduler, and control component
    class Application : public QApplication
    {
        Q_OBJECT

        public:
            Application (int &argc, char **argv, Framework::Control *ctrl);
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

        private:
            Framework::Worker::List workers_;
            Framework::Module::List modules_;

            Framework::Scheduler    scheduler_;
            Framework::Control      *ctrl_;

            DispatchThread  thread_;

            QTimer  frame_timer_;
            QTime   time_;
    };
}

#endif
