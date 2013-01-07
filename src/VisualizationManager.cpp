#include <string>

// OpenGL Extension "autoloader"
#include <GL/glew.h>

// There are conflicting definitions in SDL_opengl.h
// with glew.h. So prefere glew.h.
#define GL_SGIX_fragment_lighting

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include "VisualizationManager.h"
#include "PrintStatusMsg.h"


VisualizationManager::VisualizationManager()
  : video_painter(&msg_painter),
    edge_video_painter(&msg_painter),
    renderer(NULL),
    gps_painter(&msg_painter, 256, 256)
{
  this->video_painters.push_back(&video_painter);
  this->video_painters.push_back(&edge_video_painter);
}


VisualizationManager::~VisualizationManager()
{
  if(this->renderer)
    delete this->renderer;
}


int 
VisualizationManager::init(Ice::LoggerPtr &log, Ice::PropertiesPtr props)
{
  this->logger = log;
  
  // initialize SDL
  int res = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
  if(res < 0)
    {
      std::string msg = "Video initialization failed: ";
      msg += SDL_GetError();
      this->logger->error(msg);
      SDL_Quit();
      return res;
    }

  // Sets up OpenGL double buffering
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  // Enabling multisampling
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

  // Flags to pass to SDL_SetVideoMode
  int video_flags  = SDL_OPENGL;      // Enable OpenGL in SDL
  video_flags |= SDL_GL_DOUBLEBUFFER; // Enable double buffering
  video_flags |= SDL_HWPALETTE;       // Store the palette in hardware
  video_flags |= SDL_RESIZABLE;       // Enable window resizing

  const SDL_VideoInfo *video_info = SDL_GetVideoInfo();

  int screen_width = video_info->current_w / 10 * 9;
  int screen_height = video_info->current_h / 10 * 9;
  // Get a SDL surface
  SDL_Surface *surface = SDL_SetVideoMode(screen_width, 
					  screen_height,
					  video_info->vfmt->BitsPerPixel,
					  video_flags);
  // Verify there is a surface
  if(!surface)
    {
      // Try once more without multisampling
      this->logger->print("Disabling multisampling");
      SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
      SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
      surface = SDL_SetVideoMode(screen_width,
				 screen_height,
				 video_info->vfmt->BitsPerPixel,
				 video_flags);
      if(!surface)
        {
	  std::string msg = "Video mode set failed: ";
	  msg += SDL_GetError();
	  this->logger->error(msg);
          return -3;
        }
    }

  SDL_WM_SetCaption("Cockpit", "Cockpit");
  SDL_WM_SetIcon(SDL_LoadBMP("cockpit.bmp"), NULL);

  SDL_version compile_version;
  const SDL_version *link_version = IMG_Linked_Version();
  SDL_IMAGE_VERSION(&compile_version);
  char buff[1024];
  sprintf(buff, "compiled with SDL_image version: %d.%d.%d\n", 
	  compile_version.major,
	  compile_version.minor,
	  compile_version.patch);
  logger->print(buff);
  sprintf(buff, "running with SDL_image version: %d.%d.%d\n", 
	  link_version->major,
	  link_version->minor,
	  link_version->patch);
  logger->print(buff);

  // Load support for the JPG, PNG and TIFF image formats
  // for textures
  const int flags = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF;
  int initted = IMG_Init(flags);
  if((initted & flags) != flags)
    {
      this->logger->print("IMG_Init: Failed to init required jpg, png and tiff support.");
      std::string msg = "IMG_Init: ";
      msg += IMG_GetError();
      this->logger->error(msg);
      return -4;
    }

  GLenum err = glewInit();
  if(GLEW_OK != err)
    {
      std::string msg = "GLEW initialization failed: ";
      msg += (const char*)glewGetErrorString(err);
      this->logger->error(msg);
      return -5;
    }

  // Tune OpenGL
  glShadeModel(GL_SMOOTH); // Enable Smooth Shading
  glClearColor(0.0f, 0.0f, 0.0f, 0.5f); // Black Background
  glClearDepth(1.0f);  // Depth Buffer Setup
  glEnable(GL_DEPTH_TEST); // Enables Depth Testing
  glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
  glEnable(GL_TEXTURE_2D); // Enable Texture Mapping
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glEnable(GL_BLEND);
  //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  //glBlendFunc(GL_SRC_ALPHA, GL_DST_COLOR);
  glCullFace(GL_BACK);

  this->renderer = new DefaultRenderer(printStatusMessage1);

  scene.getSceneGraph()->printStatusMessage = printStatusMessage1;
  std::string str_prop = props->getProperty("UI.Model");
  if(str_prop.empty())
    {
      this->logger->print("Property UI.Model is not set.");
      this->logger->print("Do not know where to find UI model. Exiting...");
      return -6;
    }
  else
    {
      scene.load(str_prop);
      renderer->setScene(scene.getSceneGraph());

      Ice::StringSeq anim_ids = props->getPropertyAsList("UI.Animations.SteeringWheel");
      if(anim_ids.empty())
        {
          this->logger->print("Property UI.Animations.SteeringWheel is not specified.");
          this->logger->print("There will be no animation for steering control.");
        }
      for(Ice::StringSeq::const_iterator aid = anim_ids.begin();
	  aid != anim_ids.end(); ++aid)
        {
          AddAnimationController(steeringWheelAnimations, *aid, scene.getSceneGraph());
        }
      if(!steeringWheelAnimations.empty())
	{
	  sensorid_t id(0, sensors::Joystick, 0);
	  sensor_animations_map.insert(std::make_pair(id, AnimationCmd(&steeringWheelAnimations)));
	  id.id = 0;
	  id.type = sensors::Keyboard;
	  id.data_idx = 0;
	  sensor_animations_map.insert(std::make_pair(id, AnimationCmd(&steeringWheelAnimations)));
	}

      anim_ids = props->getPropertyAsList("UI.Animations.Acceleration");
      if(anim_ids.empty())
        {
          this->logger->print("Property UI.Animations.Acceleration is not specified.");
          this->logger->print("There will be no animation for acceleration control.");
        }
      for(Ice::StringSeq::const_iterator aid = anim_ids.begin();
	  aid != anim_ids.end(); ++aid)
        {
          AddAnimationController(accelAnimations, *aid, scene.getSceneGraph());
        }
      if(!accelAnimations.empty())
	{
	  sensorid_t id(0, sensors::Joystick, 1);
	  sensor_animations_map.insert(std::make_pair(id, AnimationCmd(&accelAnimations)));
	  id.id = 0;
	  id.type = sensors::Keyboard;
	  id.data_idx = 1;
	  sensor_animations_map.insert(std::make_pair(id, AnimationCmd(&accelAnimations)));
	}

      anim_ids = props->getPropertyAsList("UI.Animations.Compass");
      if(anim_ids.empty())
        {
          this->logger->print("Property UI.Animations.Compass is not specified.");
          this->logger->print("There will be no animation for compass data.");
        }
      for(Ice::StringSeq::const_iterator aid = anim_ids.begin();
	  aid != anim_ids.end(); ++aid)
        {
          AddAnimationController(compassAnimations, *aid, scene.getSceneGraph());
        }
      if(!compassAnimations.empty())
	{
	  sensorid_t id(0xC0, sensors::Compass, 0);
	  sensor_animations_map.insert(std::make_pair(id, AnimationCmd(&compassAnimations)));
	}

      int sonar_ids[] = {0xE0, 0xE2, 0xE4, 0xE6};
      sonarAnimations.resize(sizeof(sonar_ids) / sizeof(sonar_ids[0]));
      char sonar_prop_name[32];
      for(size_t i = 0; i < sizeof(sonar_ids) / sizeof(sonar_ids[0]); ++i)
	{
	  sprintf(sonar_prop_name, "UI.Animations.Sonar%0X", sonar_ids[i]);
	  anim_ids = props->getPropertyAsList(sonar_prop_name);
	  if(anim_ids.empty())
	    {
	      std::string msg;
	      msg = "Property ";
	      msg += sonar_prop_name;
	      msg += " is not specified.";
	      this->logger->print(msg);
	      this->logger->print("There will be no animation for this sonar data.");
	      continue;
	    }
	  for(Ice::StringSeq::const_iterator aid = anim_ids.begin();
	      aid != anim_ids.end(); ++aid)
	    {
	      AddAnimationController(sonarAnimations[i], 
				     *aid, 
				     scene.getSceneGraph());
	    }
	  if(!sonarAnimations[i].empty())
	    {
	      sensorid_t id(sonar_ids[i], sensors::Range, 0);
	      sensor_animations_map.insert(std::make_pair(id, AnimationCmd(&sonarAnimations[i])));
	    }
	}

      std::string video_img_props[] = {
	std::string("UI.VideoImage1Id"), 
	std::string("UI.VideoImage2Id")
      };

      for(size_t i = 0; i < sizeof(video_img_props) / sizeof(video_img_props[0]); ++i)
	{
	  str_prop = props->getProperty(video_img_props[i]);
	  if(str_prop.empty())
	    {
	      this->logger->print(std::string("Property ") + video_img_props[i] + " is not specified.");
	      this->logger->print("There will be no video displayed.");
	    }
	  else
	    {
	      if(renderer->texture_names.empty())
		{
		  this->logger->print("Texture list is empty.");
		  this->logger->print("There will be no video displayed.");
		}
	      else
		{
		  DefaultRenderer::TextureNameMap::const_iterator tex 
		    = renderer->texture_names.begin();
		  Image::ImageID id = {str_prop, tex->first.doc_uri};
		  tex = renderer->texture_names.find(id);
		  if(tex == renderer->texture_names.end())
		    {
		      this->logger->print(std::string("Video texture with specified name not found: ") + str_prop);
		      this->logger->print("There will be no video displayed.");
		    }
		  else
		    this->video_painters[i]->setTextureId(tex->second);
		}
	    }
	}

      str_prop = props->getProperty("UI.GPSImageId");
      if(str_prop.empty())
        {
          this->logger->print("Property UI.GPSImageId is not specified.");
          this->logger->print("There will be no map data displayed.");
        }
      else
        {
          if(renderer->texture_names.empty())
	    {
	      this->logger->print("Texture list is empty.");
	      this->logger->print("There will be no map data displayed.");
	    }
          else
	    {
	      DefaultRenderer::TextureNameMap::const_iterator tex 
		= renderer->texture_names.begin();
	      Image::ImageID id = {str_prop, tex->first.doc_uri};
	      tex = renderer->texture_names.find(id);
	      if(tex == renderer->texture_names.end())
		{
		  this->logger->print(std::string("GPS texture with specified name not found: ") + str_prop);
		  this->logger->print("There will be no map data displayed.");
		}
	      else
		{
		  Image img = renderer->scene_graph->all_images.find(id)->second;
		  tile_manager.width = img.image->w;
		  tile_manager.height = img.image->h;
		  tile_manager.texture_id = tex->second;
		}
	    }
        }

      // resize the initial window
      this->resizeWindow(screen_width, screen_height);
    }
  
  return 0;
}


void 
VisualizationManager::drawScene()
{
  for(VideoPainterList::iterator vp = this->video_painters.begin();
      vp != this->video_painters.end(); ++vp)
    (*vp)->paint();

  this->gps_painter.paint();
  this->renderer->render();
  SDL_GL_SwapBuffers(); // Swap the buffers to not be left with a clear screen
}


// The Reshape Function (the viewport)
void 
VisualizationManager::resizeWindow(int w, int h)
{
  this->renderer->setupCamera(w, h);
}
