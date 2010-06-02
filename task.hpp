/* task.hpp -- 
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

            Task (Callable t) 
                : work (t) 
            {}

            Task *chain (Task *t) 
            { 
                dependants.push_back (t); 
                return this;
            }
        };

        class Scheduler
        {
            public:
                Scheduler ()
                {
                }

                void enqueue (Task *task)
                {
                    Locker mtx (queue_lock_);

                    enqueue_ (task);
                }

                void enqueue (const Task::List &list)
                {
                    Locker mtx (queue_lock_);

                    enqueue_ (list);
                }

                int length ()
                {
                    return queue_.size();
                }

                void dispatch (frame_delta_t delta)
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

            private:
                void enqueue_ (Task *task)
                {
                    task->state = Task::READY;

                    queue_.push_back (task);
                }

                void enqueue_ (const Task::List &list)
                {
                    Task::List::const_iterator i = list.begin();
                    Task::List::const_iterator e = list.end();
                    for (; i != e; ++i) enqueue_ (*i);
                }

                void execute_ (Task *head, frame_delta_t delta)
                {
                    head->state = Task::RUNNING;

                    head->work (delta);

                    head->state = Task::COMPLETE;
                }

                void dispose_ (Task *head) 
                {
                    head->state = Task::FINAL;

                    delete head;
                }

            private:
                Task::List  queue_;
                Mutex       queue_lock_;
        };
    }
}

#endif //TASK_H_
