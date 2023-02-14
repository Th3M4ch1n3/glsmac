#pragma once

#include <cstdint>
#include <cstring>

namespace types {

class Color {
public:
	
	typedef float channel_t;
	
	struct color_t {
		channel_t red;
		channel_t green;
		channel_t blue;
		channel_t alpha;
		void operator= ( const Color& color ) {
			memcpy( this, &color.value, sizeof(color.value) );
		}
	};
	
	color_t value;
	
	typedef uint32_t rgba_t;
	
	Color();
	Color( const channel_t red, const channel_t green, const channel_t blue, const channel_t alpha );
	Color( const channel_t red, const channel_t green, const channel_t blue );
	Color( const color_t& color );
	
	void Set( channel_t red, channel_t green, channel_t blue, channel_t alpha );
	
	void operator= ( const color_t& color );
	bool operator== ( Color& other ) const;
	bool operator!= ( Color& other ) const;
	bool operator!= ( const Color& other );
	const Color operator* ( const float operand ) const;
	const Color operator/ ( const float operand ) const;

	const rgba_t GetRGBA() const;
	static Color FromRGBA( const rgba_t rgba );
	static Color FromRGBA( const uint8_t red, const uint8_t green, const uint8_t blue, const uint8_t alpha );
	static Color FromRGB( const uint8_t red, const uint8_t green, const uint8_t blue );
	static rgba_t RGBA( const uint8_t red, const uint8_t green, const uint8_t blue, const uint8_t alpha );
	static rgba_t RGB( const uint8_t red, const uint8_t green, const uint8_t blue );
};

} /* namespace types */
