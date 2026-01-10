
// ****************************************************************************
//                                 
//                              Includes
//
// ****************************************************************************

#include INCLUDES_H		// all includes

#if USE_RCA
#include "../../../rca-library/include.h"
#endif

#define RUSTICAL_STYLE_WALLS
#define DUNGEON_MASTER_STYLE_WALLS
//#define _USE_FIELD_OF_VIEW_
//#define _ENABLE_SHADING_

#include "../../src/_include.h"
#include "src/main.h"		// main code

#include "src/dungeonTypes.h"

#include "src/bitTables.h"
#include "src/dungeon.h"
#include "src/externBitmaps.h"
#include "src/LevelDefinitions.h"
#include "src/monsterBitmaps.h"
#include "src/objectBitmaps.h"
#include "src/soundFX.h"
#include "src/TinyDungeon.h"
#include "src/TinyJoypadUtils.h"
#include "src/wallBitmaps.h"
