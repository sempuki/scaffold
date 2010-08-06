/* model.hpp -- exported function header
 *
 *			Ryan McDougall
 */

#ifndef COMPONENT_FACTORY_H_
#define COMPONENT_FACTORY_H_

namespace Scaffold
{
    namespace Model
    {
        class ComponentFactoryBase
        {
            public:
                typedef std::vector <ComponentFactoryBase *> List;

                virtual ~ComponentFactoryBase () {};
                virtual Component::List components () = 0;
                virtual void decorate (Entity *entity) = 0;
        };

        template <typename ComponentType>
        class ComponentFactory : public ComponentFactoryBase
        {
            public:
                ComponentFactory (const Tag &t, const Tag::Set &archetypes)
                    : type_ (t), supported_ (archetypes)
                {}

                ComponentFactory (const Tag &t, const char *archetypes[], size_t n)
                    : type_ (t)
                {
                    supported_.insert (archetypes, archetypes + n);
                }

                ~ComponentFactory ()
                {
                    for_each (pool_.begin(), pool_.end(),
                            safe_delete <Component>);
                }

                Component::List components () 
                {
                    return pool_;
                }

                void decorate (Entity *entity)
                {
                    if (supported_.count (entity->type()))
                    {
                        ComponentType *comp = new ComponentType (type_);
                        pool_.push_back (comp);

                        entity->components.insert (make_pair (type_, comp));
                        entity->observe (*comp);
                    }
                }

            private:
                Tag             type_;
                Tag::Set        supported_;
                Component::List pool_;
        };
    }
}

#endif //COMPONENT_FACTORY_H_
