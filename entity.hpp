/* model.hpp -- exported function header
 *
 *			Ryan McDougall
 */

#ifndef ENTITY_H_
#define ENTITY_H_

namespace Scaffold
{
    namespace Model
    {
        class Entity : public Tagged
        {
            public:
                typedef std::vector <Entity *> List;
                typedef std::map <Tag, Entity *> Map;
                typedef std::multimap <Tag, Entity *> MultiMap;

                Entity (const Tag &t) 
                    : Tagged (t)
                {}

                Entity (const Tag &t, const Tag &type) 
                    : Tagged (t), archetype_ (type)
                {}

                Tag type () 
                { 
                    return archetype_; 
                }

                bool has (const Tag &t) 
                { 
                    return components.count (t); 
                }

                Component *get (const Tag &t) 
                { 
                    return components [t]; 
                }

                template <typename T>
                T *get (const Tag &t) 
                { 
                    return static_cast <T *> (components [t]); 
                }

            public:
                void observe (Component &comp)
                {
                    using namespace std::tr1::placeholders;

                    comp.on_access += bind (&Entity::component_access_, this, _1);
                    comp.on_change += bind (&Entity::component_change_, this, _1);
                }

            public:
                Subscription <void(Entity*)> on_access;
                Subscription <void(Entity*)> on_change;

            public:
                Component::Map components;

            protected:
                void component_access_ (Component *comp)
                {
                    on_access (this);
                }

                void component_change_ (Component *comp)
                {
                    on_change (this);
                }

            private:
                Tag archetype_;
        };
    }
}

#endif //ENTITY_H_
