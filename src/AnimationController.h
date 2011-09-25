#ifndef __ANIMATIONCONTROLLER_H
#define __ANIMATIONCONTROLLER_H

#include <vector>
#include <string>
#include <collada-view/Scene.h>


class AnimationController
{
 public:
  AnimationController(Animation::AnimationList::iterator &anim, 
                      SceneGraph *sg);

 public:
  void animate(short percentage);

 protected:
  Animation::AnimationList::iterator animation;
  SceneGraph *scene_graph;
  float animation_step;
};

typedef std::vector<AnimationController> AnimationControllerList;

void AddAnimationController(AnimationControllerList &list, 
                            const std::string &anim_id,
                            SceneGraph *scene_graph);

void AnimateList(AnimationControllerList &list, short percentage);

#endif // __ANIMATIONCONTROLLER_H
