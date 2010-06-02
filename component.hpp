/* model.hpp -- exported function header
 *
 *			Ryan McDougall
 */

#ifndef COMPONENT_H_
#define COMPONENT_H_

namespace Scaffold
{
    namespace Model
    {
        class PropertyBase : public Tagged
        {
            public:
                PropertyBase (const Tag &t) : Tagged (t) {}

            public:
                Subscription <void(PropertyBase*)> on_access;
                Subscription <void(PropertyBase*)> on_change;
        };

        template <typename T>
        class Property : public PropertyBase
        {
            public:
                Property (const Tag &t, T def = T()) 
                    : PropertyBase (t), default_ (def) 
                {
                    reset();
                }

                virtual ~Property () 
                {
                }

            public:
                operator T ()
                { 
                    return get(); 
                }

                Property operator= (const T &v) 
                { 
                    set (v); 
                    return *this; 
                }

            public:
                T get ()
                { 
                    on_access (this);
                    on_value_access (prop_);
                    return prop_; 
                }

                void set (const T &v) 
                { 
                    prop_ = v; 
                    on_access (this);
                    on_change (this);
                    on_value_access (prop_);
                    on_value_change (prop_);
                }

                void reset () 
                { 
                    set (default_); 
                }

            public:
                Subscription <void(T)> on_value_access;
                Subscription <void(T)> on_value_change;

            private:
                T   prop_;
                T   default_;

        };

        class Component : public Tagged
        {
            public:
                typedef std::vector <Component *> List;
                typedef std::map <Tag, Component *> Map;

                Component (const Tag &t) 
                    : Tagged (t)
                {}

                void observe (PropertyBase &prop)
                {
                    using namespace std::tr1::placeholders;

                    prop.on_access += bind (&Component::property_access_, this, _1);
                    prop.on_change += bind (&Component::property_change_, this, _1);
                }

            public:
                Subscription <void(Component*)> on_access;
                Subscription <void(Component*)> on_change;

            protected:
                void property_access_ (PropertyBase *prop)
                {
                    on_access (this);
                }

                void property_change_ (PropertyBase *prop)
                {
                    on_change (this);
                }
        };
    }
}

#endif //COMPONENT_H_
