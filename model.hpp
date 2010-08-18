/* model.hpp -- all Data Model headers
 *
 *			Ryan McDougall
 */

#ifndef MODEL_H_
#define MODEL_H_

#include "tag.hpp"
#include "subscription.hpp"
#include "component.hpp"
#include "entity.hpp"
#include "componentfactory.hpp"
#include "entityfactory.hpp"
#include "scene.hpp"

extern Scaffold::Model::Scene           *model_entities;
extern Scaffold::Model::EntityFactory   *model_entity_factory;

#endif //MODEL_H_
