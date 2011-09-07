/*
-----------------------------------------------------------------------------
Filename:    LandscapeApplication.h
-----------------------------------------------------------------------------

This source file is part of the
   ___                 __    __ _ _    _ 
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/                              
      Tutorial Framework
      http://www.ogre3d.org/tikiwiki/
-----------------------------------------------------------------------------
*/
#ifndef __LandscapeApplication_hpp__
#define __LandscapeApplication_hpp__

#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>

#include <Caelum.h>
#include <Hydrax.h>

#include "BaseApplication.hpp"

class LandscapeApplication : public BaseApplication
{
public:
	LandscapeApplication(void);
	virtual ~LandscapeApplication(void);

protected:
	virtual void createScene(void);
	virtual bool frameEnded(const Ogre::FrameEvent& evt);

	Caelum::CaelumSystem* mCaelumSystem;
	Ogre::TerrainGlobalOptions* mTerrainGlobals;
	Ogre::TerrainGroup* mTerrainGroup;
	Hydrax::Hydrax* mHydrax;
	Ogre::Vector3 mOriginalWaterColor;
};

#endif // #ifndef __LandscapeApplication_hpp__
