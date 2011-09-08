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

#include <WindBatchPage.h>
#include <ImpostorPage.h>
#include <TreeLoader2D.h>

using namespace Ogre;
using namespace Caelum;
using namespace Forests;


//-------------------------------------------------------------------------------------
static TerrainGroup* terrainGroup = NULL;

static float getTerrainHeight(float x, float z, void* userData)
{
	OgreAssert(terrainGroup, "Terrain isn't initialized");
	return terrainGroup->getHeightAtWorldPosition(x, 0, z);
}

//-------------------------------------------------------------------------------------
LandscapeApplication::LandscapeApplication(void) : grass(NULL), trees(NULL), bushes(NULL)
{
}
//-------------------------------------------------------------------------------------
LandscapeApplication::~LandscapeApplication(void)
{
	if(grass)
	{
		delete grass->getPageLoader();
		delete grass;
		grass = NULL;
	}
	if(trees)
	{
		delete trees->getPageLoader();
		delete trees;
		trees = NULL;
	}
	if(bushes)
	{
		delete bushes->getPageLoader();
		delete bushes;
		bushes = NULL;
	}

	mSceneMgr->destroyEntity("Tree1");
	mSceneMgr->destroyEntity("Tree2");
	mSceneMgr->destroyEntity("Fern");
	mSceneMgr->destroyEntity("Plant");
	mSceneMgr->destroyEntity("Mushroom");
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
	mCamera->setPosition(Vector3(1628, 85, 2563));									// направляем камеру
	mCamera->lookAt(Vector3(1963, 0, 1660));

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
	terrainGroup = mTerrainGroup;

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

	// Растительность

	// трава
	//
	grass = new PagedGeometry(mCamera);
	grass->addDetailLevel<GrassPage>(160);											// уровень детализации: не рендерить траву дальше 60 единиц от камеры
	grassLoader = new GrassLoader(grass);
	grass->setPageLoader(grassLoader);
	grassLoader->setHeightFunction(getTerrainHeight);								// функция, возвращающая высоту ландшафта в заданной точке
	GrassLayer* l = grassLoader->addLayer("3D-Diggers/plant1sprite");				// добавить слой травы
	l->setMinimumSize(0.9f, 0.9f);													// максимальный...
	l->setMaximumSize(2, 2);														//  ... и минимальный размер
	l->setAnimationEnabled(true);													// включить анимацию
	l->setSwayDistribution(7.0f);													// колебания травы от ветра
	l->setSwayLength(0.1f);															// амплитуда колебаний - 0.1 единиц
	l->setSwaySpeed(0.4f);															// скорость колебаний
	l->setDensity(3.0f);															// плотность травы
	l->setRenderTechnique(GRASSTECH_SPRITE);										// рендерим с помощью спрайтов
	l->setFadeTechnique(FADETECH_GROW);												// при движении камеры трава должна медленно подниматься
	l->setColorMap("terrain_texture2.jpg");											// карта распределения цвета
	l->setDensityMap("densitymap.png");												// карта плотности
	l->setMapBounds(TBounds(0, 0, 3000, 3000));										// границы слоя
	// деревья
	//
	trees = new PagedGeometry(mCamera);
	trees->addDetailLevel<WindBatchPage>(150, 30);									// использовать батчинг на расстоянии до 150 единиц и затухание на 30 единиц
	trees->addDetailLevel<ImpostorPage>(900, 50);									// заменять модели спрайтами на расстоянии между 900 и 950 единицами
	TreeLoader2D *treeLoader = new TreeLoader2D(trees, TBounds(0, 0, 5000, 5000));
	trees->setPageLoader(treeLoader);
	treeLoader->setHeightFunction(getTerrainHeight);								// функция, возвращающая высоту ландшафта в заданной точке
	treeLoader->setColorMap("terrain_lightmap.jpg");								// карта распределения цвета - в данном случае совпадает с картой распределения света на ландшафте
	Entity *tree1 = mSceneMgr->createEntity("Tree1", "fir05_30.mesh");				// загрузить модели деревьев
	Entity *tree2 = mSceneMgr->createEntity("Tree2", "fir14_25.mesh");
	trees->setCustomParam(tree1->getName(), "windFactorX", 15);						// параметры ветра
	trees->setCustomParam(tree1->getName(), "windFactorY", 0.01f);
	trees->setCustomParam(tree2->getName(), "windFactorX", 22);
	trees->setCustomParam(tree2->getName(), "windFactorY", 0.013f);
	// распределяем случайным образом 5000 копий деревьев
	Vector3 position = Vector3::ZERO;
	Radian yaw;
	Real scale;
	for (int i = 0; i < 5000; i++)
	{
		yaw = Degree(Math::RangeRandom(0, 360));
		position.x = Math::RangeRandom(0, 2000);									// координата Y не требуется, т.к. будет вычислена
		position.z = Math::RangeRandom(2300, 4000);									//  с помощью ф-ии getTerrainHeight, для быстродействия
		scale = Math::RangeRandom(0.07f, 0.12f);
		if (Math::UnitRandom() < 0.5f)
		{
			if (Math::UnitRandom() < 0.5f)
				treeLoader->addTree(tree1, position, yaw, scale);
		}
		else
			treeLoader->addTree(tree2, position, yaw, scale);
	}

	// кусты/грибы
	//
	bushes = new PagedGeometry(mCamera);
	bushes->addDetailLevel<WindBatchPage>(80, 50);
	TreeLoader2D *bushLoader = new TreeLoader2D(bushes, TBounds(0, 0, 5000, 5000));
	bushes->setPageLoader(bushLoader);
	bushLoader->setHeightFunction(getTerrainHeight);
	bushLoader->setColorMap("terrain_lightmap.jpg");
	Entity *fern = mSceneMgr->createEntity("Fern", "farn1.mesh");					// загрузить модель папоротника
	Entity *plant = mSceneMgr->createEntity("Plant", "plant2.mesh");				// загрузить модель цветка
	Entity *mushroom = mSceneMgr->createEntity("Mushroom", "shroom1_1.mesh");		// загрузить модель гриба
	bushes->setCustomParam(fern->getName(), "factorX", 1);							// параметры ветра
	bushes->setCustomParam(fern->getName(), "factorY", 0.01f);
	bushes->setCustomParam(plant->getName(), "factorX", 0.6f);
	bushes->setCustomParam(plant->getName(), "factorY", 0.02f);
	// распределяем случайным образом 20000 копий кустов и грибов
	for (int i = 0; i < 20000; i++)
	{
		yaw = Degree(Math::RangeRandom(0, 360));
		position.x = Math::RangeRandom(0, 2000);
		position.z = Math::RangeRandom(2300, 4000);
		if (Math::UnitRandom() < 0.8f) {
			scale = Math::RangeRandom(0.3f, 0.4f);
			bushLoader->addTree(fern, position, yaw, scale);
		} else if (rnd < 0.9) {
			scale = Math::RangeRandom(0.2f, 0.6f);
			bushLoader->addTree(mushroom, position, yaw, scale);
		} else {
			scale = Math::RangeRandom(0.3f, 0.5f);
			bushLoader->addTree(plant, position, yaw, scale);
		}
	}
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

	grass->update();
	trees->update();
	bushes->update();

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
