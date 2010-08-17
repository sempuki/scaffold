/* task.hpp -- abstraction for task-based work queues
 *
 *			Ryan McDougall
 */

#ifndef TASK_H_
#define TASK_H_

namespace Scaffold
{
    namespace Framework
    {
        struct Task
        {
            typedef function <void(frame_delta_t)> Callable;
            typedef std::list <Task *> List;

            enum 
            {
                INITIAL,
                READY,
                RUNNING,
                COMPLETE,
                FINAL,
                ERROR
            };

            int state;
            int priority;

            Callable    work;
            List        dependants;

            Task (Callable t);
            Task *chain (Task *t);
        };

        class Scheduler
        {
            public:
                Scheduler ();

                void enqueue (Task *task);
                void enqueue (const Task::List &list);
                void dispatch (frame_delta_t delta);
                int length ();

            private:
                void enqueue_ (Task *task);
                void enqueue_ (const Task::List &list);
                void execute_ (Task *head, frame_delta_t delta);
                void dispose_ (Task *head);

            private:
                Task::List  queue_;
                Mutex       queue_lock_;
        };
    }
}

#endif //TASK_H_
