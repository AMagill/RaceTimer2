#pragma once
#include <stdint.h>

// This structure describes a single character's display information
struct FONT_CHAR_INFO
{
	const uint8_t widthBits; 					// width, in bits (or pixels), of the character
	const uint16_t offset; 					// offset of the character's bitmap, in bytes, into the the FONT_INFO's data array
};	

// Describes a single font
struct FONT_INFO
{
	const uint8_t 			heightPages; 	// height, in pages (8 pixels), of the font's characters
	const uint8_t 			startChar; 		// the first character in the font (e.g. in charInfo and data)
	const uint8_t 			endChar; 		// the last character in the font
	const uint8_t			spacePixels; 	// number of pixels that a space character takes up
	const FONT_CHAR_INFO*	charInfo; 		// pointer to array of char information
	const uint8_t*			data; 			// pointer to generated array of character visual representation
};
