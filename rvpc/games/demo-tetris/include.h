
// ****************************************************************************
//                                 
//                              Includes
//
// ****************************************************************************
#include INCLUDES_H     // all includes

#if USE_RCA
#include "../../../rca-library/include.h"
#endif

#include "../../src/_include.h"
#include "src/main.h"		// main code

#include "src/keyboard.h"
#include "src/print.h"
#include "src/vga.h"
#include "src/mini-printf.h"

#if USE_RCA
extern const font_desc_t TextModeFontRvpcTetris;
#else
extern const ALIGNED u8 TextModeFontRvpcTetris[4096];
#endif


