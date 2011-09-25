/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

// OpenGL Extension "autoloader"
#include <GL/glew.h>

// There are conflicting definitions in SDL_opengl.h
// with glew.h. So prefere glew.h.
#define GL_SGIX_fragment_lighting

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include <stdio.h>
#include "TxtAreaPainter.h"
#include "VideoPainter.h"
#include "AbstractCommand.h"
#include "IceApp.h"
#include "ServerThread.h"
#include "BufferQueue.h"

#include <collada-view/Scene.h>
#include <collada-view/DefaultRenderer.h>
#include "PrintStatusMsg.h"
#include "AnimationController.h"
#include "AnimationCmd.h"

#include "MapDownloadThread.h"
#include "TileManager.h"

/* screen width, height, and bit depth */
#define SCREEN_WIDTH  1000
#define SCREEN_HEIGHT 800
#define SCREEN_BPP     16

#define STEERING_TIMER_INTERVAL ((40/10)*10)
#define ACCEL_TIMER_INTERVAL ((50/10)*10)

#define WHEEL_ROTATION_STEP 5

#define TACHO_ROTATION_STEP 3

TxtAreaPainter msg_painter;
VideoPainter video_painter(&msg_painter);
VideoPainter gps_painter(&msg_painter, 256, 256);
BufferQueue buffer_queue;
SDL_TimerID steering_timer = NULL;
SDL_TimerID accel_timer = NULL;
AnimationControllerList steeringWheelAnimations;
AnimationControllerList accelAnimations;
AnimationControllerList sonar1Animations;
AnimationControllerList compassAnimations;
AnimationCmdMap sensor_animations_map;
MapDownloadThread map_download_thread;
TileManager tile_manager(&map_download_thread);
IceApp app(&msg_painter, 
           &video_painter,
           &tile_manager, 
           &map_download_thread,
           &buffer_queue, 
           &sensor_animations_map);
SDL_Joystick *joystick = NULL;
bool reverse_accel = false;


void 
myInitGL(void)
{
  glShadeModel(GL_SMOOTH);							// Enable Smooth Shading

  glClearColor(0.0f, 0.0f, 0.0f, 0.5f); // Black Background
  glClearDepth(1.0f);                   // Depth Buffer Setup
  glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
  glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do

  glEnable(GL_TEXTURE_2D);              // Enable Texture Mapping

  glEnable(GL_MULTISAMPLE);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  glEnable(GL_BLEND);
  //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glBlendFunc(GL_SRC_ALPHA, GL_DST_COLOR);

  glCullFace(GL_BACK);
}


void 
drawScene(DefaultRenderer *renderer)   // Create The Display Function
{
  video_painter.paint();
  gps_painter.paint();
  renderer->render();
  SDL_GL_SwapBuffers(); // Swap the buffers to not be left with a clear screen
}


// The Reshape Function (the viewport)
void 
resizeWindow(DefaultRenderer *renderer, int w, int h)
{
  renderer->setupCamera(w, h);
}


void 
sendUserEvent()
{
  SDL_Event event;
  event.user.type = SDL_USEREVENT;
  event.user.code = 0;
  SDL_PushEvent(&event);
}


static short wheel_angle = 0;
static short accel_angle = 0;
static short camera_angle = 50;

Uint32 
steeringLeft(Uint32 interval, void *param)
{
  if(wheel_angle > 0 + WHEEL_ROTATION_STEP)
    wheel_angle -= WHEEL_ROTATION_STEP;
  AnimateList(steeringWheelAnimations, wheel_angle);
  app.setSteering(wheel_angle);
  sendUserEvent();
  return interval;
}


Uint32 
steeringRight(Uint32 interval, void *param)
{
  if(wheel_angle < 100 - WHEEL_ROTATION_STEP)
    wheel_angle += WHEEL_ROTATION_STEP;
  AnimateList(steeringWheelAnimations, wheel_angle);
  app.setSteering(wheel_angle);
  sendUserEvent();
  return interval;
}


Uint32 
accelMinus(Uint32 interval, void *param)
{
  if(accel_angle > 0 + TACHO_ROTATION_STEP)
    accel_angle -= TACHO_ROTATION_STEP;
  AnimateList(accelAnimations, accel_angle);
  app.setAccel(accel_angle);
  sendUserEvent();
  return interval;
}

Uint32 
accelPlus(Uint32 interval, void *param)
{
  if(accel_angle < 100 - TACHO_ROTATION_STEP)
    accel_angle += TACHO_ROTATION_STEP;
  AnimateList(accelAnimations, accel_angle);
  app.setAccel(accel_angle);
  sendUserEvent();
  return interval;
}


/* function to handle key press events */
void 
handleKeyPress(SDL_keysym *keysym, bool *done, SceneGraph *scene_graph)
{
  switch(keysym->sym)
    {
    case SDLK_ESCAPE:
      /* ESC key was pressed */
      *done = true;
      break;

    case SDLK_LEFT:
      if(steering_timer != NULL)
        SDL_RemoveTimer(steering_timer);
      steering_timer = 
        SDL_AddTimer(STEERING_TIMER_INTERVAL, steeringLeft, scene_graph);
      break;

    case SDLK_RIGHT:
      if(steering_timer != NULL)
        SDL_RemoveTimer(steering_timer);
      steering_timer = 
        SDL_AddTimer(STEERING_TIMER_INTERVAL, steeringRight, scene_graph);
      break;

    case SDLK_UP:
      if(accel_timer != NULL)
        SDL_RemoveTimer(accel_timer);
      accel_timer = 
        SDL_AddTimer(ACCEL_TIMER_INTERVAL, 
                     accelPlus, 
                     (void*)(keysym->mod & KMOD_LCTRL));
      break;

    case SDLK_DOWN:
      if(accel_timer != NULL)
        SDL_RemoveTimer(accel_timer);
      accel_timer = 
        SDL_AddTimer(ACCEL_TIMER_INTERVAL, 
                     accelMinus, 
                     (void*)(keysym->mod & KMOD_LCTRL));
      break;

    case SDLK_a:
      if(camera_angle < 95)
        {
          camera_angle += 5;
          app.setRotateCamera(camera_angle);
        }
      break;

    case SDLK_d:
      if(camera_angle > 5)
        {
          camera_angle -= 5;
          app.setRotateCamera(camera_angle);
        }
      break;

    default:
      break;
    }

  return;
}


/* function to handle key release events */
void 
handleKeyRelease(SDL_keysym *keysym)
{
  switch(keysym->sym)
    {
    case SDLK_LEFT:
    case SDLK_RIGHT:
      if(steering_timer != NULL)
        {
          SDL_RemoveTimer(steering_timer);
          steering_timer = NULL;
        }
      break;

    case SDLK_UP:
    case SDLK_DOWN:
      if(accel_timer != NULL)
        {
          SDL_RemoveTimer(accel_timer);
          accel_timer = NULL;
        }
      break;

    default:
      break;
    }

  return;
}


void 
handleJoystickAxisMotion(SDL_JoyAxisEvent *event)
{
  /*  static int prevsteering = 0;
  static int prevaccel = 0;
  int v;

  if(event->axis == app.getJoystickSteeringAxis())
    { // steering
      wheel_painter.rotateAbs(range_map(32767, -32767, 
                                        short(-WHEEL_MAX_ANGLE), 
                                        short(WHEEL_MAX_ANGLE), 
                                        event->value));
      v = range_map(32767, -32767, 0, 100, event->value);
      if(v != prevsteering)
        {
          app.setSteering(v);
          prevsteering = v;
        }
    }
  else if(event->axis == app.getJoystickAccelAxis())
    { // acceleration
      tacho_painter.rotateAbs(range_map(32767, -32767, 
                                        0, -2*short(TACHO_MAX_ANGLE), 
                                        event->value));
      if(reverse_accel == false)
        v = range_map(32767, -32767, 50, 100, event->value);
      else
        v = range_map(32767, -32767, 50, 0, event->value);
      printf("Axis value: %i, accel duty: %i, reverse = %c\n", 
             event->value,
             v, 
             reverse_accel ? 'y' : 'n');
      if(v != prevaccel)
        {
          app.setAccel(v);
          prevaccel = v;
        }
    }
  */
}


void 
executeCommand(AbstractCommand *cmd)
{
  cmd->execute();
  if(cmd->shouldDelete())
    delete cmd;
}


int  
mainLoop(DefaultRenderer *renderer, 
         int video_flags, 
         SDL_Surface **surface, 
         bool *done)
{
  /* used to collect events */
  SDL_Event event;

  drawScene(renderer);

  /* wait for events */
  while(!(*done))
    {
      /* handle the events in the queue */
      while(SDL_WaitEvent(&event))
        {
          switch(event.type)
            {
            case SDL_VIDEORESIZE:
              /* handle resize event */
              resizeWindow(renderer, event.resize.w, event.resize.h);
              break;

            case SDL_KEYDOWN:
              /* handle key presses */
              handleKeyPress(&event.key.keysym, done, renderer->scene_graph);
              break;

            case SDL_KEYUP:
              /* handle key releases */
              handleKeyRelease(&event.key.keysym);
              break;

            case SDL_JOYAXISMOTION:
              /* Handle Joystick Motion */
              handleJoystickAxisMotion(&event.jaxis);
              break;
          
            case SDL_JOYBUTTONDOWN:
              /* Handle Joystick Button Presses */
              printf("Button pressed: %i\n", event.jbutton.button);
              if(event.jbutton.button == app.getJoystickReverseButton()) 
                reverse_accel = true;
              break;

            case SDL_JOYBUTTONUP:
              /* Handle Joystick Button releases */
              printf("Button released: %i\n", event.jbutton.button);
              if(event.jbutton.button == app.getJoystickReverseButton()) 
                reverse_accel = false;
              break;

            case SDL_USEREVENT:
              if(event.user.code && event.user.data1)
                executeCommand((AbstractCommand*)event.user.data1);
              break;

            case SDL_QUIT:
              /* handle quit requests */
              *done = true;
              break;

            default:
              drawScene(renderer);
              break;
            }

          if(!(*done))
            drawScene(renderer);
          else
            break;
        }
    }

  if(joystick)
    SDL_JoystickClose(joystick);
  /* clean ourselves up and exit */
  SDL_Quit();

  return 0;
}


int  
main(int argc, char** argv)
{
  Scene scene;
  SDL_Surface *surface;
  int video_flags; // Flags to pass to SDL_SetVideoMode
  const SDL_VideoInfo *videoInfo; // this holds some info about our display
  bool done = false;
  int res = 0;

  if(argc == 1)
    {
      printf("No configuration file specified.\n");
      printf("Which is equivalent to invoking:\n");
      printf("cockpit --Ice.Config=cockpit.config\n");
    }
  else if(argc != 2)
    {
      fprintf(stderr, "Usage: cockpit --Ice.Config=<config_file>\n");
      fprintf(stderr, "If no --Ice.Config option specified, then cockpit.config will be used.\n");
      return -1;
    }

  // initialize SDL
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER) < 0)
    {
      fprintf(stderr, 
              "Video initialization failed: %s\n",
              SDL_GetError());
      SDL_Quit();
      return -2;
    }

  // Fetch the video info
  videoInfo = SDL_GetVideoInfo();

  if(!videoInfo)
    {
      fprintf(stderr, 
              "Video query failed: %s\n",
              SDL_GetError( ) );
      SDL_Quit();
      return -3;
    }

  // Sets up OpenGL double buffering
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  // Enable multisampling
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

  // Flags to pass to SDL_SetVideoMode
  video_flags  = SDL_OPENGL;          // Enable OpenGL in SDL
  video_flags |= SDL_GL_DOUBLEBUFFER; // Enable double buffering
  video_flags |= SDL_HWPALETTE;       // Store the palette in hardware
  video_flags |= SDL_RESIZABLE;       // Enable window resizing
  // Get a SDL surface
  surface = SDL_SetVideoMode(SCREEN_WIDTH, 
                             SCREEN_HEIGHT, 
                             SCREEN_BPP,
                             video_flags);

  // Verify there is a surface
  if(!surface)
    {
      fprintf(stderr,
              "Video mode set failed: %s\n",
              SDL_GetError());
      SDL_Quit();
      return -3;
    }

  SDL_WM_SetCaption("Cockpit", "Cockpit");
  SDL_WM_SetIcon(SDL_LoadBMP("icon.bmp"), NULL);

  SDL_version compile_version;
  const SDL_version *link_version = IMG_Linked_Version();
  SDL_IMAGE_VERSION(&compile_version);
  printf("compiled with SDL_image version: %d.%d.%d\n", 
         compile_version.major,
         compile_version.minor,
         compile_version.patch);
  printf("running with SDL_image version: %d.%d.%d\n", 
         link_version->major,
         link_version->minor,
         link_version->patch);

  // load support for the JPG, PNG and TIFF image formats
  const int flags = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF;
  int initted = IMG_Init(flags);
  if((initted & flags) != flags)
    {
      fprintf(stderr, "IMG_Init: Failed to init required jpg, png and tiff support!\n");
      fprintf(stderr, "IMG_Init: %s\n", IMG_GetError());
      return -4;
    }

  GLenum err = glewInit();
  if(GLEW_OK != err)
    {
      fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
      return -5;
    }

  int num_joysticks = SDL_NumJoysticks();
  printf("%i joysticks were found:\n", num_joysticks);
  for(int i = 0; i < num_joysticks; ++i) 
      printf("  %s\n", SDL_JoystickName(i));

  if(num_joysticks)
    {
      printf("Using the first one.\n");
      SDL_JoystickEventState(SDL_ENABLE);
      joystick = SDL_JoystickOpen(0);      
    }

  // initialize OpenGL
  myInitGL();

  ServerThreadPtr ct = new ServerThread(&app, argc, argv);
  IceUtil::ThreadControl ctc = ct->start();

  try
    {
      scene.getSceneGraph()->printStatusMessage = printStatusMessage1;
      std::string str_prop = ct->getStringProperty("UI.Model");
      if(str_prop.empty())
      {
        printStatusMessage("Property UI.Model is not set.", NULL);
        printStatusMessage("Do not know where to find UI model. Exiting...", NULL);
      }
      else
      {
        scene.load(str_prop);
        DefaultRenderer renderer(printStatusMessage1);
        renderer.setScene(scene.getSceneGraph());

        Ice::StringSeq anim_ids = ct->getStringListProperty("UI.Animations.SteeringWheel");
        if(anim_ids.empty())
        {
          printStatusMessage("Property UI.Animations.SteeringWheel is not specified.", NULL);
          printStatusMessage("There will be no animation for steering control.", NULL);
        }
        for(Ice::StringSeq::const_iterator aid = anim_ids.begin();
            aid != anim_ids.end(); ++aid)
        {
          AddAnimationController(steeringWheelAnimations, *aid, scene.getSceneGraph());
        }

        anim_ids = ct->getStringListProperty("UI.Animations.Acceleration");
        if(anim_ids.empty())
        {
          printStatusMessage("Property UI.Animations.Acceleration is not specified.", NULL);
          printStatusMessage("There will be no animation for acceleration control.", NULL);
        }
        for(Ice::StringSeq::const_iterator aid = anim_ids.begin();
            aid != anim_ids.end(); ++aid)
        {
          AddAnimationController(accelAnimations, *aid, scene.getSceneGraph());
        }

        anim_ids = ct->getStringListProperty("UI.Animations.Compass");
        if(anim_ids.empty())
        {
          printStatusMessage("Property UI.Animations.Compass is not specified.", NULL);
          printStatusMessage("There will be no animation for compass data.", NULL);
        }
        for(Ice::StringSeq::const_iterator aid = anim_ids.begin();
            aid != anim_ids.end(); ++aid)
        {
          AddAnimationController(compassAnimations, *aid, scene.getSceneGraph());
        }
        if(!compassAnimations.empty())
          sensor_animations_map.insert(std::make_pair(3, AnimationCmd(&compassAnimations)));

        anim_ids = ct->getStringListProperty("UI.Animations.Sonar1");
        if(anim_ids.empty())
        {
          printStatusMessage("Property UI.Animations.Sonar1 is not specified.", NULL);
          printStatusMessage("There will be no animation for sonar1 data.", NULL);
        }
        for(Ice::StringSeq::const_iterator aid = anim_ids.begin();
            aid != anim_ids.end(); ++aid)
        {
          AddAnimationController(sonar1Animations, *aid, scene.getSceneGraph());
        }
        if(!sonar1Animations.empty())
          sensor_animations_map.insert(std::make_pair(2, AnimationCmd(&sonar1Animations)));

        str_prop = ct->getStringProperty("UI.VideoImageId");
        if(str_prop.empty())
        {
          printStatusMessage("Property UI.VideoImageId is not specified.", NULL);
          printStatusMessage("There will be no video displayed.", NULL);
        }
        else
        {
          if(renderer.texture_names.empty())
          {
            printStatusMessage("Texture list is empty.", NULL);
            printStatusMessage("There will be no video displayed.", NULL);
          }
          else
          {
            DefaultRenderer::TextureNameMap::const_iterator tex 
              = renderer.texture_names.begin();
            Image::ImageID id = {str_prop, tex->first.doc_uri};
            tex = renderer.texture_names.find(id);
            if(tex == renderer.texture_names.end())
            {
              printStatusMessage(std::string("Video texture with specified name not found: ") + str_prop, NULL);
              printStatusMessage("There will be no video displayed.", NULL);
            }
            else
              video_painter.setTextureId(tex->second);
          }
        }

        str_prop = ct->getStringProperty("UI.GPSImageId");
        if(str_prop.empty())
        {
          printStatusMessage("Property UI.GPSImageId is not specified.", NULL);
          printStatusMessage("There will be no map data displayed.", NULL);
        }
        else
        {
          if(renderer.texture_names.empty())
          {
            printStatusMessage("Texture list is empty.", NULL);
            printStatusMessage("There will be no map data displayed.", NULL);
          }
          else
          {
            DefaultRenderer::TextureNameMap::const_iterator tex 
              = renderer.texture_names.begin();
            Image::ImageID id = {str_prop, tex->first.doc_uri};
            tex = renderer.texture_names.find(id);
            if(tex == renderer.texture_names.end())
            {
              printStatusMessage(std::string("GPS texture with specified name not found: ") + str_prop, NULL);
              printStatusMessage("There will be no map data displayed.", NULL);
            }
            else
              {
                Image img = renderer.scene_graph->all_images.find(id)->second;
                tile_manager.width = img.image->w;
                tile_manager.height = img.image->h;
                tile_manager.texture_id = tex->second;
              }
          }
        }

        // resize the initial window
        resizeWindow(&renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
        res = mainLoop(&renderer, video_flags, &surface, &done);
      }
    }
  catch(const std::exception &ex)
    {
      printStatusMessage(ex.what(), NULL);
      res = 101;
    }

  app.requestShutdown();
  printf("Waiting for communication thread to shutdown\n");
  ctc.join();

  printf("Waiting for map download thread to shutdown\n");
  map_download_thread.requestShutdownAndWait();

  printf("Unload the dynamically loaded image libraries\n");
  IMG_Quit();

  return res;
}
