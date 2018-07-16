#include "font_symbols.h"

// 
//  Font data for Open Sans Condensed 32pt
// 

// Character bitmaps for Open Sans Condensed 32pt
const uint8_t symbolsBitmaps[] = 
{
	// @0 Batt-0 (10 pixels wide)
	//           
	//           
	// ########  
	// #      ## 
	// #       # 
	// #      ## 
	// ########  
	//           
	0x7C, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x6C, 0x38, 0x00,
		
	// @10 Batt-1 (10 pixels wide)
	//           
	//           
	// ########  
	// ##     ## 
	// ##      # 
	// ##     ## 
	// ########  
	//           
	0x7C, 0x7C, 0x44, 0x44, 0x44, 0x44, 0x44, 0x6C, 0x38, 0x00,
		
	// @20 Batt-2 (10 pixels wide)
	//           
	//           
	// ########  
	// ###    ## 
	// ###     # 
	// ###    ## 
	// ########  
	//           
	0x7C, 0x7C, 0x7C, 0x44, 0x44, 0x44, 0x44, 0x6C, 0x38, 0x00,
		
	// @30 Batt-3 (10 pixels wide)
	//           
	//           
	// ########  
	// ####   ## 
	// ####    # 
	// ####   ## 
	// ########  
	//           
	0x7C, 0x7C, 0x7C, 0x7C, 0x44, 0x44, 0x44, 0x6C, 0x38, 0x00,
		
	// @40 Batt-4 (10 pixels wide)
	//           
	//           
	// ########  
	// #####  ## 
	// #####   # 
	// #####  ## 
	// ########  
	//           
	0x7C, 0x7C, 0x7C, 0x7C, 0x7C, 0x44, 0x44, 0x6C, 0x38, 0x00,
		
	// @50 Batt-5 (10 pixels wide)
	//           
	//           
	// ########  
	// ###### ## 
	// ######  # 
	// ###### ## 
	// ########  
	//           
	0x7C, 0x7C, 0x7C, 0x7C, 0x7C, 0x7C, 0x44, 0x6C, 0x38, 0x00,
		
	// @60 Batt-6 (10 pixels wide)
	//           
	//           
	// ########  
	// ######### 
	// ####### # 
	// ######### 
	// ########  
	//           
	0x7C, 0x7C, 0x7C, 0x7C, 0x7C, 0x7C, 0x7C, 0x6C, 0x38, 0x00,
	
	// @70 Batt-7 (10 pixels wide)
	//           
	//           
	// ########  
	// ######### 
	// ######### 
	// ######### 
	// ########  
	//           
	0x7C, 0x7C, 0x7C, 0x7C, 0x7C, 0x7C, 0x7C, 0x7C, 0x38, 0x00,
	
	// @80 Signal-0 (10 pixels wide)
	//           
	//           
	//           
	//           
	//           
	//           
	// # # # # # 
	//           
	0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00,
	
	// @90 Signal-1 (10 pixels wide)
	//           
	//           
	//           
	//           
	//           
	//   #       
	// # # # # # 
	//           
	0x40, 0x00, 0x60, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00,
	
	// @100 Signal-2 (10 pixels wide)
	//           
	//           
	//           
	//           
	//     #     
	//   # #     
	// # # # # # 
	//           
	0x40, 0x00, 0x60, 0x00, 0x70, 0x00, 0x40, 0x00, 0x40, 0x00,
	
	// @110 Signal-3 (10 pixels wide)
	//           
	//           
	//           
	//       #   
	//     # #   
	//   # # #   
	// # # # # # 
	//           
	0x40, 0x00, 0x60, 0x00, 0x70, 0x00, 0x78, 0x00, 0x40, 0x00,
	
	// @120 Signal-4 (10 pixels wide)
	//           
	//           
	//         # 
	//       # # 
	//     # # # 
	//   # # # # 
	// # # # # # 
	//           
	0x40, 0x00, 0x60, 0x00, 0x70, 0x00, 0x78, 0x00, 0x7C, 0x00,
};

// Character descriptors for Open Sans Condensed 32pt
// { [Char width in bits], [Offset into symbolsCharBitmaps in bytes] }
const FONT_CHAR_INFO symbolsDescriptors[] = 
{
	{10, 0}, 		// b Batt-0
	{10, 10}, 		// c Batt-1 
	{10, 20}, 		// d Batt-2 
	{10, 30}, 		// e Batt-3 
	{10, 40}, 		// f Batt-4 
	{10, 50}, 		// g Batt-5 
	{10, 60}, 		// h Batt-6 
	{10, 70}, 		// i Batt-7 
	{0, 0}, 		// j
	{0, 0}, 		// k
	{0, 0}, 		// l
	{0, 0}, 		// m
	{0, 0}, 		// n
	{0, 0}, 		// o
	{0, 0}, 		// p
	{0, 0}, 		// q
	{0, 0}, 		// r
	{10, 80}, 		// s Signal-0
	{10, 90}, 		// t Signal-1
	{10, 100}, 		// u Signal-2
	{10, 110}, 		// v Signal-3
	{10, 120}, 		// w Signal-4
};

// Font information for Open Sans Condensed 32pt
const FONT_INFO symbols_FontInfo =
{
	1, //  Character height
	'b', //  Start character
	'w', //  End character
	2, //  Width, in pixels, of space character
	symbolsDescriptors, //  Character descriptor array
	symbolsBitmaps, //  Character bitmap array
};

