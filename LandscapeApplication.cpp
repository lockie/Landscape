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

#include <Noise/Perlin/Perlin.h>
#include <Modules/ProjectedGrid/ProjectedGrid.h>

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
	mCaelumSystem->setManageSceneFog(Ogre::FOG_NONE);

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

	// Вода
	mHydrax = new Hydrax::Hydrax(mSceneMgr, mCamera, mWindow->getViewport(0));
	Hydrax::Module::ProjectedGrid* mModule = new Hydrax::Module::ProjectedGrid(
			mHydrax,																// указатель на главный класс Hydrax
			new Hydrax::Noise::Perlin(/* без особых параметров */),					// модуль для создания ряби
			Ogre::Plane(Ogre::Vector3(0,1,0), Ogre::Vector3(0,0,0)),				// водная поверхность
			Hydrax::MaterialManager::NM_VERTEX,										// режим карты нормалей
			Hydrax::Module::ProjectedGrid::Options(64));							// опции сетки
	mHydrax->setModule(mModule);
	mHydrax->loadCfg("HydraxDemo.hdx");
	mOriginalWaterColor = mHydrax->getWaterColor();
	mHydrax->create();
	mHydrax->getMaterialManager()->addDepthTechnique(
		mTerrainGroup->getTerrain(0, 0)->getMaterial()->createTechnique());			// добавить технику глубины в материал страницы ландшафта (0, 0)
	mCamera->setFarClipDistance(1000000);

}

//-------------------------------------------------------------------------------------
bool LandscapeApplication::frameEnded(const Ogre::FrameEvent& evt)
{
	Vector3 value = mCaelumSystem->getSun()->getSceneNode()->_getDerivedPosition();
	ColourValue cval = mCaelumSystem->getSun()->getBodyColour();
	mHydrax->setSunPosition(value);
	mHydrax->setSunColor(Vector3(cval.r,cval.g,cval.b));

	Caelum::LongReal mJulian = mCaelumSystem->getUniversalClock()->getJulianDay();
	cval = mCaelumSystem->getSunLightColour(mJulian, mCaelumSystem->getSunDirection(mJulian));
	mHydrax->setWaterColor(Vector3(cval.r - 0.3, cval.g - 0.2, cval.b));

	Vector3 col = mHydrax->getWaterColor();
	float height = mHydrax->getSunPosition().y / 10.0f;

	Hydrax::HydraxComponent c = mHydrax->getComponents();
	if(height < 0)
	{
		if(mHydrax->isComponent(Hydrax::HYDRAX_COMPONENT_CAUSTICS))
			mHydrax->setComponents(Hydrax::HydraxComponent(c ^ Hydrax::HYDRAX_COMPONENT_CAUSTICS));
	} else
	{
		if(!mHydrax->isComponent(Hydrax::HYDRAX_COMPONENT_CAUSTICS))
			mHydrax->setComponents(Hydrax::HydraxComponent(c | Hydrax::HYDRAX_COMPONENT_CAUSTICS));
	}

	if(height < -99.0f)
	{
		col = mOriginalWaterColor * 0.1f;
		height = 9999.0f;
	}
	else if(height < 1.0f)
	{
		col = mOriginalWaterColor * (0.1f + (0.009f * (height + 99.0f)));
		height = 100.0f / (height + 99.001f);
	}
	else if(height < 2.0f)
	{
		col += mOriginalWaterColor;
		col /= 2.0f;
		float percent = (height - 1.0f);
		col = (col * percent) + (mOriginalWaterColor * (1.0f - percent));
	}
	else
	{
		col += mOriginalWaterColor;
		col	/= 2.0f;
	}
	mHydrax->setWaterColor(col);
	mHydrax->setSunArea(height);

	mHydrax->update(evt.timeSinceLastFrame);

	return true;
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
