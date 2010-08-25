/* application.cpp -- application level abstraction
 *
 *			Ryan McDougall
 */

#include "application.hpp"

namespace Scaffold
{
    //=========================================================================

    DispatchThread::DispatchThread() : 
        scheduler_ (0), stop_ (false) 
    {}

    void DispatchThread::setScheduler (Framework::Scheduler *s) 
    { 
        scheduler_ = s; 
    }

    void DispatchThread::setFrameDelta (frame_delta_t d) 
    { 
        delta_ = d; 
    }

    void DispatchThread::run ()
    {
        int num;

        while (!stop_) 
        {
            num = scheduler_->length ();
            while (num --) scheduler_->dispatch (delta_);

            QThread::yieldCurrentThread ();
        }
    }

    void DispatchThread::stop ()
    {
        stop_ = true;
    }
    
    //=========================================================================

    Application::Application (int &argc, char **argv) :
        QApplication (argc, argv), app_ (0), world_ (0)
    {
        // set up components for application entity
        do_entity_initialize ();

        // set thread's scheduler
        thread_.setScheduler (&scheduler_);

        // set up main loop real-time timer
        connect (&frame_timer_, SIGNAL (timeout()), this, SLOT (update()));
        frame_timer_.setSingleShot (true);
        frame_timer_.start (0);
        time_.start ();

        // set thread's real-time delta values
        app_->state = Framework::AppState::READY;
        app_->delta.on_value_change += bind
            (&DispatchThread::setFrameDelta, &thread_, _1);
    }

    Application::~Application ()
    {
        thread_.stop ();
        thread_.wait (1000);

        do_module_finalize ();
        do_module_delete ();
        do_worker_delete ();
    }

    void Application::attach (Framework::Worker *worker) 
    {
        workers_.push_back (worker);
    }

    void Application::attach (Framework::Module *module) 
    {
        modules_.push_back (module);
    }

    int Application::exec ()
    {
        do_module_initialize ();

        // set up shared state
        app_->state = Framework::AppState::RUNNING;

        // set up dispatching thread
        thread_.start ();

        return QApplication::exec ();
    }

    void Application::update ()
    {
        // get real-time delta value
        frame_delta_t delta = time_.elapsed();

        app_->delta = delta;   // update subscribers
        do_worker_pump ();      // update workers

        // restart timer for as soon as possible
        frame_timer_.start (0); 
        time_.restart ();
    }

    void Application::do_worker_pump ()
    {
        for_each (workers_.begin(), workers_.end(), 
                mem_fn (&Framework::Worker::pump));
    }

    void Application::do_worker_delete ()
    {
        for_each (workers_.begin(), workers_.end(), 
                safe_delete <Framework::Worker>);
    }

    void Application::do_module_initialize ()
    {
        for_each (modules_.begin(), modules_.end(), 
                bind (&Framework::Module::initialize, _1, &scheduler_));
    }

    void Application::do_module_finalize ()
    {
        for_each (modules_.begin(), modules_.end(), 
                mem_fn (&Framework::Module::finalize));
    }

    void Application::do_module_delete ()
    {
        for_each (modules_.begin(), modules_.end(), 
                safe_delete <Framework::Module>);
    }

    void Application::do_entity_initialize ()
    {
        using namespace Model;
        using namespace Framework;

        const char *app_archetypes[] = { "application-archetype" };

        model_entity_factory->attach
            (new ComponentFactory <AppState>
             ("application-state-component", app_archetypes, 1));

        model_entity_factory->attach
            (new ComponentFactory <WorldState>
             ("application-worldstate-component", app_archetypes, 1));

        Entity *app = model_entity_factory->create ("application", "application-archetype");
        app_ = app->get <AppState> ("application-state-component");
        world_ = app->get <WorldState> ("application-worldstate-component");
        model_entities->insert (app);
    }
}
