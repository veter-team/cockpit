#include "AnimationController.h"
#include <collada-view/DefaultRenderer.h>


AnimationController::AnimationController(Animation::AnimationList::iterator &anim,
                                         SceneGraph *sg)
  : animation(anim), scene_graph(sg), animation_step(0)
{
  this->animation_step = this->animation->second.getMaxTime() / 100.0f;
}


void 
AnimationController::animate(short percentage)
{
  Animation::NodeIdSet touched_nodes;
  this->animation->second.animate(scene_graph->printStatusMessage,
                                  scene_graph->all_nodes,
                                  this->animation_step * percentage,
                                  touched_nodes);
  if(!touched_nodes.empty())
      UpdateLocalMatricies(scene_graph->all_nodes, touched_nodes);
}


void 
AddAnimationController(AnimationControllerList &list, 
                       const std::string &anim_id,
                       SceneGraph *scene_graph)
{
  Animation::AnimationID id = {anim_id, scene_graph->all_animations.begin()->first.doc_uri};
  Animation::AnimationList::iterator a = scene_graph->all_animations.find(id);
  if(a != scene_graph->all_animations.end())
  {
    AnimationController c(a, scene_graph);
    list.push_back(c);
  }
}


void 
AnimateList(AnimationControllerList &list, short percentage)
{
  for(AnimationControllerList::iterator a = list.begin();
    a != list.end(); ++a)
  {
    a->animate(percentage);
  }
}
