#ifndef __VISUALIZATIONMANAGER_H
#define __VISUALIZATIONMANAGER_H

#include <Ice/Logger.h>
#include <Ice/Properties.h>

#include <collada-view/Scene.h>
#include <collada-view/DefaultRenderer.h>

#include "TxtAreaPainter.h"
#include "VideoPainter.h"
#include "AnimationController.h"
#include "AnimationCmd.h"
#include "TileManager.h"


class VisualizationManager
{
 public:
  VisualizationManager();
  ~VisualizationManager();

 public:
  int init(Ice::LoggerPtr &log, Ice::PropertiesPtr props);
  void drawScene();
  void resizeWindow(int w, int h);

 private:
  TxtAreaPainter msg_painter;

 public:
  AnimationCmdMap sensor_animations_map;
  TileManager tile_manager;
  VideoPainter video_painter;

 private:
  DefaultRenderer* renderer;
  VideoPainter gps_painter;
  AnimationControllerList steeringWheelAnimations;
  AnimationControllerList accelAnimations;
  std::vector<AnimationControllerList> sonarAnimations;
  AnimationControllerList compassAnimations;
  Ice::LoggerPtr logger;
  Scene scene;
};


#endif // __VISUALIZATIONMANAGER_H
