
//{{BLOCK(cave2)

//======================================================================
//
//	cave2, 1024x256@8, 
//	+ palette 16 entries, not compressed
//	+ 20 tiles (t|f|p reduced) not compressed
//	+ regular map (in SBBs), not compressed, 128x32 
//	Total size: 32 + 1280 + 8192 = 9504
//
//	Time-stamp: 2017-08-03, 13:58:20
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.3
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_CAVE2_H
#define GRIT_CAVE2_H

#define cave2TilesLen 640
extern const unsigned short cave2Tiles[320];

#define cave2MapLen 8192
extern const unsigned short cave2Map[4096];

#define cave2colMapLen 8192
extern const unsigned short cave2colMap[4096];

#define cave2PalLen 32
extern const unsigned short cave2Pal[16];

#endif // GRIT_CAVE2_H

//}}BLOCK(cave2)
