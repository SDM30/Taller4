// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================

#include "Application.h"

#include <algorithm>
#include <filesystem>
#include <queue>
#include <iostream>
#include <sstream>

#include <Ogre.h>

// -------------------------------------------------------------------------
namespace fs = std::filesystem;

// -------------------------------------------------------------------------
Application::Application(int &ac, char **av)
    : TContext(fs::path(av[0]).filename().stem().string()), TListener() {
  this->m_ExecutionDir = fs::canonical(fs::path(av[0]).parent_path()).string();
}

// -------------------------------------------------------------------------
Application::~Application() {}

// -------------------------------------------------------------------------
void Application::loadResources() {
  this->enableShaderCache();
  auto rgm = Ogre::ResourceGroupManager::getSingletonPtr();
  try {
    rgm->initialiseAllResourceGroups();
    rgm->loadResourceGroup(Ogre::RGN_DEFAULT);
  } catch (...) {
    // Do nothing
  }
}

// -------------------------------------------------------------------------
void Application::locateResources() {
  // Required by Ogre itself
  const Ogre::String ogreMedia = this->getDefaultMediaDir();
  auto rgm = Ogre::ResourceGroupManager::getSingletonPtr();
  rgm->addResourceLocation(ogreMedia + "/Main", "FileSystem",
                           Ogre::RGN_INTERNAL);
  rgm->addResourceLocation(ogreMedia + "/RTShaderLib", "FileSystem",
                           Ogre::RGN_INTERNAL);

  std::string media_dir =
      std::filesystem::path(this->m_ExecutionDir) / "SimpleFPS_resources";
  rgm->addResourceLocation(media_dir, "FileSystem", Ogre::RGN_DEFAULT);
}

// -------------------------------------------------------------------------
void Application::setup() {
  this->_loadExtraPlugins();
  this->_initSceneManager();
  this->_loadScene();
  this->_configureCamera();
}

OgreBites::NativeWindowPair Application::createWindow(
    const Ogre::String& name, uint32_t w, uint32_t h,
    Ogre::NameValuePairList miscParams) {
  return this->TContext::createWindow(name, 1280, 720, miscParams);
}

// -------------------------------------------------------------------------
void Application::go() {
  this->initApp();
  this->getRoot()->startRendering();
  this->closeApp();
}

// -------------------------------------------------------------------------
bool Application::keyPressed(const OgreBites::KeyboardEvent &evt) {
  if (evt.keysym.sym == OgreBites::SDLK_ESCAPE) {
    this->getRoot()->queueEndRendering();
    return (true);
  } 
  else if (evt.keysym.sym == '1') { 
    this->m_CurrentProjectile = 0; 
    std::cout << "Proyectil: " << this->m_Projectiles[ this->m_CurrentProjectile ] << std::endl;
    return (true); 
  }
  else if (evt.keysym.sym == '2') { 
    this->m_CurrentProjectile = 1; 
    std::cout << "Proyectil: " << this->m_Projectiles[ this->m_CurrentProjectile ] << std::endl;
    return (true); 
  }
  else if (evt.keysym.sym == '3') { 
    this->m_CurrentProjectile = 2; 
    std::cout << "Proyectil: " << this->m_Projectiles[ this->m_CurrentProjectile ] << std::endl;
    return (true); 
  }
  else
    return (false);
}

// -------------------------------------------------------------------------
bool Application::mousePressed(const OgreBites::MouseButtonEvent &evt) {
  if (evt.button == OgreBites::BUTTON_LEFT) {
    this->m_ClickTimer.reset();
    this->m_Charging = true;
    return (true);
  } else
    return (false);
}

// -------------------------------------------------------------------------
bool Application::mouseReleased(const OgreBites::MouseButtonEvent &evt) {
  if (evt.button == OgreBites::BUTTON_LEFT && this->m_Charging) {
    this->m_Charging = false;

    const float seconds = this->m_ClickTimer.getMilliseconds() / 1000.0f;
    const float speedMin = 5.0f;
    const float speedMax = 50.0f;
    const float speedPerSecond = 20.0f;
    const float speed = std::min(speedMin + seconds * speedPerSecond, speedMax);

    const float muzzleDistance = 0.3f;
    Ogre::Vector3 dir = this->m_CameraMan->getAimDirection();
    Ogre::Vector3 cannonPosition = this->getRoot()
                                       ->getSceneManager("scnMgr")
                                       ->getSceneNode("Cannon")
                                       ->_getDerivedPosition();

    this->m_Model.shoot( this->m_Projectiles[ this->m_CurrentProjectile ] , cannonPosition + dir * muzzleDistance,
                        dir, speed);
            
    return (true);
  } else
    return (false);
}

// -------------------------------------------------------------------------
void Application::frameRendered(const Ogre::FrameEvent &evt) {
  this->m_Model.update(evt.timeSinceLastFrame);
}

// -------------------------------------------------------------------------
void Application::_loadExtraPlugins() {
  auto *root = this->getRoot();
  std::istringstream in(pujOgre_PLUGINS);
  std::string plugin;
  while (std::getline(in, plugin, '|')) {
    std::string plg_name = std::filesystem::path(this->m_ExecutionDir) / plugin;
    plg_name += pujOgre_SHARED_EXT;
    root->loadPlugin(plg_name);
  } // end while
}

// -------------------------------------------------------------------------
void Application::_initSceneManager() {
  this->TContext::setup();
  this->addInputListener(this);

  auto *root = this->getRoot();
  auto mgr = root->createSceneManager("DefaultSceneManager", "scnMgr");
  mgr->setAmbientLight(Ogre::ColourValue(0.9, 0.9, 0.9));

  // register our scene with the RTSS
  auto *shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
  shadergen->addSceneManager(mgr);
}

// -------------------------------------------------------------------------
void Application::_configureCamera() {
  auto *scnMgr = this->getRoot()->getSceneManager("scnMgr");
  Ogre::Camera *cam = scnMgr->getCameras().begin()->second;
  cam->setAutoAspectRatio(true);

  Ogre::SceneNode *floor = scnMgr->getSceneNode("Ground", false);
  this->m_CameraMan = new pujOgre::FPSCamera(cam, floor);
  this->m_CameraMan->setWalkSpeed(3);
  this->addInputListener(this->m_CameraMan);

  this->setWindowGrab(true);
  Ogre::Viewport *vp = this->getRenderWindow()->addViewport(cam);
  vp->setBackgroundColour(Ogre::ColourValue(0.5f, 0.5f, 0.52f));
}

// -------------------------------------------------------------------------
void Application::_loadScene() {
  auto *root = this->getRoot()->getSceneManager("scnMgr")->getRootSceneNode();
  root->loadChildren("fps.scenepp");
  this->m_Model.build(root);
}

// -------------------------------------------------------------------------
void Application::_bounds(Ogre::SceneNode *node, Ogre::AxisAlignedBox *box) {
  for (Ogre::MovableObject *obj : node->getAttachedObjects()) {
    const Ogre::String &type = obj->getMovableType();
    if (obj->isVisible() && type != "Camera" && type != "Light")
      box->merge(obj->getWorldBoundingBox(true));
  } // end for

  for (Ogre::Node *child : node->getChildren())
    Self::_bounds(static_cast<Ogre::SceneNode *>(child), box);
}

// eof - Application.cxx
