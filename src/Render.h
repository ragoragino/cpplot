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


/*
Class interface that allows arbitrary scatter or line type.
*/

class RenderObjects
{
public:
	virtual void renderPoints(HDC hdc, const std::vector<double>& x, 
		const std::vector<double>& y, RECT rect, const std::vector<double>& range,
		COLORREF color, unsigned int size) {};

	virtual void renderLines(HDC hdc, const std::map<double, double>& data, RECT rect,
		const std::vector<double>& range, COLORREF color, unsigned int size) {};

	virtual void renderLegend(HDC hdc, unsigned int x, unsigned int y) {};
};

class RenderScatter : public RenderObjects
{
public:
	virtual void renderPoints(HDC hdc, const std::vector<double>& x, 
		const std::vector<double>& y,
		RECT rect, const std::vector<double>& range, COLORREF color, unsigned int size) {};

	virtual void renderLegend(HDC hdc, unsigned int x, unsigned int y) {};
};

class RenderScatterPoints : public RenderScatter
{
public:
	virtual void renderPoints(HDC hdc, const std::vector<double>& x, const std::vector<double>& y,
		RECT rect, const std::vector<double>& range, COLORREF color, unsigned int size);

	virtual void renderLegend(HDC hdc, unsigned int x, unsigned int y);
};

void RenderScatterPoints::renderPoints(HDC hdc, const std::vector<double>& x, const std::vector<double>& y,
	RECT rect, const std::vector<double>& range, COLORREF color, unsigned int size)
{
	// Set adjusted min and max values, adjusted for the 
	// free space before/after the first/last point
	double min_x = range[0];
	double max_x = range[1];
	double min_y = range[2];
	double max_y = range[3];

	// Pre-compute variables
	double win_length_x = rect.right - rect.left;
	double win_length_y = rect.bottom - rect.top;
	double length_x = max_x - min_x;
	double length_y = max_y - min_y;
	unsigned int x_coord, y_coord;

	// Set appropriate graph properties
	HPEN hGraphPen = CreatePen(PS_SOLID, size, color);
	HBRUSH hGraphBrush = CreateSolidBrush(color);
	HPEN hGraphPreviousPen = (HPEN)SelectObject(hdc, hGraphPen);
	HBRUSH hGraphPreviousBrush = (HBRUSH)SelectObject(hdc, hGraphBrush);

	// Draw the points
	unsigned int x_size = x.size();
	for (unsigned int i = 0; i != x_size; ++i)
	{
		x_coord = rect.left +
			(unsigned int)round((x[i] - min_x) * win_length_x / length_x);
		y_coord = rect.bottom -
			(unsigned int)round((y[i] - min_y) * win_length_y / length_y);

		Ellipse(hdc, x_coord - 1, y_coord - 1, x_coord + 1, y_coord + 1);
	}

	// Clean graphic objects
	DeleteObject(hGraphPen);
	DeleteObject(hGraphBrush);

	// Set previous graphic properties
	SelectObject(hdc, hGraphPreviousPen);
	SelectObject(hdc, hGraphPreviousBrush);
}

void RenderScatterPoints::renderLegend(HDC hdc, unsigned int x, unsigned int y)
{
	Ellipse(hdc, x - 1, y - 1, x + 1, y + 1);
}

class RenderScatterSquares : public RenderScatter
{
public:
	virtual void renderPoints(HDC hdc, const std::vector<double>& x, const std::vector<double>& y,
		RECT rect, const std::vector<double>& range, COLORREF color, unsigned int size);
};

void RenderScatterSquares::renderPoints(HDC hdc, const std::vector<double>& x, const std::vector<double>& y,
	RECT rect, const std::vector<double>& range, COLORREF color, unsigned int size)
{
	// Set adjusted min and max values, adjusted for the 
	// free space before/after the first/last point
	double min_x = range[0];
	double max_x = range[1];
	double min_y = range[2];
	double max_y = range[3];

	// Pre-compute variables
	double win_length_x = rect.right - rect.left;
	double win_length_y = rect.bottom - rect.top;
	double length_x = max_x - min_x;
	double length_y = max_y - min_y;
	unsigned int x_coord, y_coord;

	// Set appropriate graph properties
	HPEN hGraphPen = CreatePen(PS_SOLID, size, color);
	HBRUSH hGraphBrush = CreateSolidBrush(color);
	HPEN hGraphPreviousPen = (HPEN)SelectObject(hdc, hGraphPen);
	HBRUSH hGraphPreviousBrush = (HBRUSH)SelectObject(hdc, hGraphBrush);

	// Draw the points
	unsigned int x_size = x.size();
	for (unsigned int i = 0; i != x_size; ++i)
	{
		x_coord = rect.left +
			(unsigned int)round((x[i] - min_x) * win_length_x / length_x);
		y_coord = rect.bottom -
			(unsigned int)round((y[i] - min_y) * win_length_y / length_y);

		Rectangle(hdc, x_coord - 2, y_coord - 2, x_coord + 2, y_coord + 2);
	}

	// Clean graphic objects
	DeleteObject(hGraphPen);
	DeleteObject(hGraphBrush);

	// Set previous graphic properties
	SelectObject(hdc, hGraphPreviousPen);
	SelectObject(hdc, hGraphPreviousBrush);
}

class RenderLines : public RenderObjects
{
public:
	virtual void render(HDC hdc, const std::map<double, double>& data, RECT rect,
		const std::vector<double>& range, COLORREF color, unsigned int size) {};
};

class RenderLinesFull : public RenderLines
{
public:
	virtual void renderLines(HDC hdc, const std::map<double, double>& data, RECT rect,
		const std::vector<double>& range, COLORREF color, unsigned int size);
};

void RenderLinesFull::renderLines(HDC hdc, const std::map<double, double>& data, RECT rect,
	const std::vector<double>& range, COLORREF color, unsigned int size)
{
	double min_x = range[0];
	double max_x = range[1];
	double min_y = range[2];
	double max_y = range[3];

	// Pre-compute variables
	double win_length_x = rect.right - rect.left;
	double win_length_y = rect.bottom - rect.top;
	double length_x = max_x - min_x;
	double length_y = max_y - min_y;

	// Move to the starting point
	unsigned int x_coord = rect.left +
		(unsigned int)round((data.begin()->first - min_x) * win_length_x / length_x);
	unsigned int y_coord = rect.bottom -
		(unsigned int)round((data.begin()->second - min_y) * win_length_y / length_y);
	MoveToEx(hdc, x_coord, y_coord, NULL);

	// Set appropriate graph properties and draw the lines
	HPEN hGraphPen = CreatePen(PS_SOLID, size, color);
	HPEN hGraphPreviousPen = (HPEN)SelectObject(hdc, hGraphPen);
	for (std::map<double, double>::const_iterator it = data.begin();
		it != data.end(); ++it)
	{
		x_coord = rect.left +
			(unsigned int)round((it->first - min_x) * win_length_x / length_x);
		y_coord = rect.bottom -
			(unsigned int)round((it->second - min_y) * win_length_y / length_y);

		LineTo(hdc, x_coord, y_coord);
	}

	// Delete graphics objects
	DeleteObject(hGraphPen);

	// Set previous graphic properties
	SelectObject(hdc, hGraphPreviousPen);
}