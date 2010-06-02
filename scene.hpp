/* scene.hpp -- 
 *
 *			Ryan McDougall
 */

#ifndef SCENE_H_
#define SCENE_H_

namespace Scaffold
{
    namespace Model
    {
        class Scene
        {
            public:
                void insert (Entity *ent)
                {
                    entities_.insert (make_pair (ent->tag(), ent));
                }

                Entity *get (const Tag &id)
                {
                    Entity::Map::iterator i = entities_.find (id);
                    return (i != entities_.end())? i->second : 0;
                }

            private:
                Entity::Map entities_;
        };
    }
}

#endif //SCENE_H_
