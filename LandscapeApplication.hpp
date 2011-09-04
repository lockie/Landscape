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

#include <Caelum.h>

#include "BaseApplication.hpp"

class LandscapeApplication : public BaseApplication
{
public:
	LandscapeApplication(void);
	virtual ~LandscapeApplication(void);

protected:
	virtual void createScene(void);

	Caelum::CaelumSystem* mCaelumSystem;
};

#endif // #ifndef __LandscapeApplication_hpp__
