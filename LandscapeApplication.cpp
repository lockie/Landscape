/*
-----------------------------------------------------------------------------
Filename:    LandscapeApplication.cpp
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
#include "LandscapeApplication.hpp"

using namespace Ogre;
using namespace Caelum;

//-------------------------------------------------------------------------------------
LandscapeApplication::LandscapeApplication(void)
{
}
//-------------------------------------------------------------------------------------
LandscapeApplication::~LandscapeApplication(void)
{
}

//-------------------------------------------------------------------------------------
void LandscapeApplication::createScene(void)
{
	// Небо
	mCaelumSystem = new CaelumSystem (mRoot, mSceneMgr,
		CaelumSystem::CAELUM_COMPONENTS_DEFAULT);
	mCaelumSystem->autoConfigure(CaelumSystem::CAELUM_COMPONENTS_DEFAULT);
	mRoot->addFrameListener(mCaelumSystem);
	mWindow->addListener(mCaelumSystem);

	mCaelumSystem->getUniversalClock()->setTimeScale(100);							// ускорим смену времени суток
	FlatCloudLayer* cloudLayer = mCaelumSystem->getCloudSystem()->createLayer();	// добавим ещё один слой облаков, зададим:
	cloudLayer->setCloudCover(0.8f);												//  плотность
	cloudLayer->setCloudSpeed(Vector2(0.0001f, 0.0001f));							//  скорость
	cloudLayer->setHeight(5000);													//  и высоту

	// Земля
	mCamera->setPosition(Vector3(1683, 50, 2116));									// направляем камеру
	mCamera->lookAt(Vector3(1963, 50, 1660));

	Vector3 lightdir(0.55f, -0.3f, 0.75f);											// свет для статического освещения местности
	lightdir.normalise();
	Light* light = mSceneMgr->createLight("tstLight");
	light->setType(Light::LT_DIRECTIONAL);
	light->setDirection(lightdir);
	light->setDiffuseColour(ColourValue::White);
	light->setSpecularColour(ColourValue(0.4f, 0.4f, 0.4f));

	mTerrainGlobals = OGRE_NEW TerrainGlobalOptions;								// глобальные настройки местности
	mTerrainGroup = OGRE_NEW TerrainGroup(mSceneMgr,								// группа страниц местности
		Terrain::ALIGN_X_Z, 513, 12000);
	mTerrainGroup->setOrigin(Vector3::ZERO);
	mTerrainGlobals->setLightMapDirection(light->getDerivedDirection());
	mTerrainGlobals->setCompositeMapAmbient(mSceneMgr->getAmbientLight());
	mTerrainGlobals->setCompositeMapDiffuse(light->getDiffuseColour());
	mSceneMgr->destroyLight("tstLight");

	mTerrainGroup->defineTerrain(0, 0);												// обозначаем наше намерение загрузить страницу с координатой (0, 0)
	mTerrainGroup->loadAllTerrains(true);											// собственно, загружаем все запрошенные страницы в синхронном режиме
	mTerrainGroup->freeTemporaryResources();										// прибираемся

}


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
	int main(int argc, char** argv)
#endif
	{
		// Create application object
		LandscapeApplication app;

		try
		{
			app.go();
		} catch( Ogre::Exception& e )
		{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			MessageBox( NULL, e.getFullDescription().c_str(), 
				"An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
			std::cerr << "An exception has occured: " <<
				e.getFullDescription().c_str() << std::endl;
#endif
		}

	return 0;
	}
