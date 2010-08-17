/* module.hpp -- interfaces for plugable application modules
 *
 *			Ryan McDougall
 */

#ifndef MODULE_H_
#define MODULE_H_

namespace Scaffold
{
    namespace Framework
    {
        class Module
        {
            public:
                typedef std::vector <Module *> List;

                virtual ~Module () {}
                virtual void initialize (Scheduler *) = 0;
                virtual void finalize () = 0;
        };

        class Plugin
        {
            public:
                typedef std::vector <Plugin *> List;

                virtual ~Plugin () {}
                virtual void initialize () = 0;
                virtual void finalize () = 0;
                virtual void update () = 0;
        };

        class Worker
        {
            public:
                typedef std::vector <Worker *> List;

                virtual ~Worker () {}
                virtual void pump () = 0;
        };
    }
}

#endif
