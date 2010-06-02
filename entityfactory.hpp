/* model.hpp -- exported function header
 *
 *			Ryan McDougall
 */

#ifndef ENTITY_FACTORY_H_
#define ENTITY_FACTORY_H_

namespace Scaffold
{
    namespace Model
    {
        class EntityFactory
        {
            public:
                ~EntityFactory ()
                {
                    for_each (pool_.begin(), pool_.end(),
                            safe_delete <Entity>);
                }

                void attach (ComponentFactoryBase *factory)
                {
                    factories_.push_back (factory);
                }

                Entity *create (const string &id, const string &archetype)
                {
                    Entity *entity = new Entity (id, archetype);
                    pool_.push_back (entity);

                    for_each (factories_.begin(), factories_.end(), 
                            bind (&ComponentFactoryBase::decorate, _1, entity));

                    return entity;
                }

            private:
                Entity::List                pool_;
                ComponentFactoryBase::List  factories_;
        };
    }
}

#endif //ENTITY_FACTORY_H_
