#pragma once
#include "Header.h"

/* 
Class interface that allows using unified rendering for ticks.
RenderAxisX flips back flipped coordinates pushed to the 
show_ticks_internal fuction in the Axis class for x axis.
*/
class RenderAxis
{
public:
	virtual void render_text(HDC hdc, unsigned int x, unsigned int y, 
		wchar_t* format, unsigned int length) = 0;

	virtual void render_tick(HDC hdc, unsigned int x, unsigned int y, 
		unsigned int stick) = 0;
};

class RenderAxisX: public RenderAxis
{
public:
	RenderAxisX(RECT in_rect) : rect(in_rect) {};

	virtual void render_text(HDC hdc, unsigned int x, unsigned int y, 
		wchar_t* format, unsigned int length);

	virtual void render_tick(HDC hdc, unsigned int x, unsigned int y, 
		unsigned int stick);

private:
	RECT rect;
};

class RenderAxisY : public RenderAxis
{
public:
	virtual void render_text(HDC hdc, unsigned int x, unsigned int y, 
		wchar_t* format, unsigned int length);

	virtual void render_tick(HDC hdc, unsigned int x, unsigned int y, 
		unsigned int stick);
};

inline void RenderAxisX::render_text(HDC hdc, unsigned int x, 
	unsigned int y, wchar_t* text, unsigned int length)
{
	unsigned int adj_x = rect.right - (y - rect.left);
	unsigned int adj_y = rect.bottom - (x - rect.top);

	TextOut(hdc, adj_x, adj_y, text, length);
}

inline void RenderAxisX::render_tick(HDC hdc, unsigned int x, 
	unsigned int y, unsigned int stick)
{
	unsigned int adj_x = rect.right - (y - rect.left);
	unsigned int adj_y = rect.bottom - (x - rect.top);

	MoveToEx(hdc, adj_x, adj_y, NULL);
	LineTo(hdc, adj_x, adj_y + stick);
}

inline void RenderAxisY::render_text(HDC hdc, unsigned int x,
	unsigned int y, wchar_t* text, unsigned int length)
{
	TextOut(hdc, x, y, text, length);
}

inline void RenderAxisY::render_tick(HDC hdc, unsigned int x, 
	unsigned int y, unsigned int stick)
{
	MoveToEx(hdc, x, y, NULL);
	LineTo(hdc, x - stick, y);
}

