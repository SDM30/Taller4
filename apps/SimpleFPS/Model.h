// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================
#ifndef __SolarSystem__Model__h__
#define __SolarSystem__Model__h__

#include <Ogre.h>
#include <OgreBullet.h>
#include <memory>
#include <unordered_map>

/**
 */
class Model {
public:
  using Self = Model;

public:
  Model();
  virtual ~Model();

  void toggle_simulation();

  void build(Ogre::SceneNode *root);
  void update(double t);
  void shoot(const Ogre::String &nodeName, const Ogre::Vector3 &pos,
             const Ogre::Vector3 &dir, float speed);

protected:
  void _configure(Ogre::Entity *e, const Ogre::UserObjectBindings &b);

protected:
  std::unique_ptr<Ogre::Bullet::DynamicsWorld> m_DynWorld;
  // std::unique_ptr< Bullet::DebugDrawer > m_DbgDraw;
  std::unordered_map<Ogre::String, btRigidBody *> m_Bodies;

  bool m_Simulating{false};
};

#endif // __SolarSystem__Model__h__

// eof - Model.h
