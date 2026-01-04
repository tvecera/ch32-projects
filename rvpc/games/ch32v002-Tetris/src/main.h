
#ifndef _MAIN_H
#define _MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

// format: 1-bit pixel graphics
// image width: 140 pixels
// image height: 103 lines
// image pitch: 18 bytes
extern const u8 ImgIntro[1854];

// format: 1-bit pixel graphics
// image width: 6 pixels
// image height: 6 lines
// image pitch: 1 bytes
extern const u8 ImgTile[6];

// format: 1-bit pixel graphics
// image width: 6 pixels
// image height: 6 lines
// image pitch: 1 bytes
extern const u8 ImgBrick[6];

#ifdef __cplusplus
}
#endif

#endif // _MAIN_H
