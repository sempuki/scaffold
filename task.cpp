/* task.cpp -- abstraction for task-based work queues
 *
 *			Ryan McDougall
 */

#include "stdheaders.hpp"
#include "task.hpp"

//=============================================================================
//
namespace Scaffold
{
    namespace Framework
    {
        Task::Task (Callable t) : 
            work (t) 
        {}

        Task *Task::chain (Task *t) 
        { 
            dependants.push_back (t); 
            return this;
        }
        

        Scheduler::Scheduler ()
        {
        }

        void Scheduler::enqueue (Task *task)
        {
            Locker mtx (queue_lock_);

            enqueue_ (task);
        }

        void Scheduler::enqueue (const Task::List &list)
        {
            Locker mtx (queue_lock_);

            enqueue_ (list);
        }

        void Scheduler::dispatch (frame_delta_t delta)
        {
            if (queue_.size())
            {
                Task *head = queue_.front();

                execute_ (head, delta);

                if (head->dependants.size())
                    enqueue_ (head->dependants);

                dispose_ (head);

                queue_.pop_front();
            }
        }

        int Scheduler::length ()
        {
            return queue_.size();
        }

        void Scheduler::enqueue_ (Task *task)
        {
            task->state = Task::READY;

            queue_.push_back (task);
        }

        void Scheduler::enqueue_ (const Task::List &list)
        {
            Task::List::const_iterator i = list.begin();
            Task::List::const_iterator e = list.end();
            for (; i != e; ++i) enqueue_ (*i);
        }

        void Scheduler::execute_ (Task *head, frame_delta_t delta)
        {
            head->state = Task::RUNNING;

            head->work (delta);

            head->state = Task::COMPLETE;
        }

        void Scheduler::dispose_ (Task *head) 
        {
            head->state = Task::FINAL;

            delete head;
        }
    }
}
