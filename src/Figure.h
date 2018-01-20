#pragma once
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Gdi32.lib")

#include "Header.h"
#include "Bitmap.h"

#include <windows.h>

/*
TODO:
-scientific notation in writing strings
-plot for x, y
-exceptions
-finish assymetric Figure constructor
*/

// define floating-point error comparison allowance
#define FP_ERROR 0.0001

// COLORS
#define BLACK RGB(0, 0, 0)
#define BLUE RGB(0, 0, 255)
#define GREEN RGB(0, 255, 0)
#define GREY RGB(128, 128, 128)
#define LIGHT_GREY RGB(211, 211, 211)
#define ORANGE RGB(255, 128, 0)
#define PINK RGB(255, 0, 255)
#define RED RGB(255, 0, 0)
#define YELLOW RGB(255, 255, 0)
#define WHITE RGB(255, 255, 255)

// VARIABLES
// defines number of graphs handled in a single window
#ifndef MAX_GRAPHS
#define MAX_GRAPHS 10
#endif

// defines adjustment of left, right, up and bottom regions
#ifndef ADJUSTMENT_GRAPH
#define ADJUSTMENT_GRAPH 0.05
#endif

// define space between the window and graph = usable rectangle
#ifndef ADJUSTMENT_WINDOW
#define ADJUSTMENT_WINDOW 10
#endif

// define ratio of attribute to the width of the whole usable rectangle
#ifndef ATTRIBUTE_DISTANCE
#define ATTRIBUTE_DISTANCE 0.12
#endif

// define ratio of tick stick w.r.t. the abslute attribute distance
#ifndef TICK_RATIO
#define TICK_RATIO 0.2
#endif

int InitializeWindow(int width, int height);

namespace cpplot {

	// Necessary declarations
	class Figure;
	inline unsigned int cumulative_sum(const std::vector<unsigned int>& container, size_t index);

	// Global variables
	namespace Globals
	{
		static constexpr unsigned int size = 10;
		static wchar_t FigureName[size];
		static unsigned int counter = 0;
		static cpplot::Figure *figure = nullptr;
		static wchar_t *dir = nullptr;
	};

	// Abstract Base Class
	class Graph
	{
	public:
		virtual void prepare(const std::vector<double>& in_x, const std::vector<double>& in_y, 
			unsigned int in_size, COLORREF in_color, std::vector<double>& range) = 0;

		virtual void show(HDC hdc, HWND hwnd, RECT rect, const std::vector<double>& range) const = 0;

		virtual ~Graph() = default;

	protected:
		COLORREF color;
		unsigned int size;
	};

	// Derived class for scatterplots
	class Scatter : public Graph
	{
	public:
		Scatter() = default;

		virtual void prepare(const std::vector<double>& in_x, const std::vector<double>& in_y, 
			unsigned int in_size, COLORREF in_color, std::vector<double>& range);

		virtual void show(HDC hdc, HWND hwnd, RECT rect, const std::vector<double>& range) const;

		virtual ~Scatter() = default;

	private:
		std::vector<double> x, y;
	};

	void Scatter::prepare(const std::vector<double>& in_x, const std::vector<double>& in_y, 
		unsigned int in_size, COLORREF in_color, std::vector<double>& range)
	{
		x = in_x;
		y = in_y;
		size = in_size;
		color = in_color;

		// Find min and max of x and y 
		double max_x = *std::max_element(x.begin(), x.end());
		double min_x = *std::min_element(x.begin(), x.end());
		double max_y = *std::max_element(y.begin(), y.end());
		double min_y = *std::min_element(y.begin(), y.end());

		// Set x and y range for Window data member range
		range[0] = range[0] < min_x ? range[0] : min_x;
		range[1] = range[1] > max_x ? range[1] : max_x;
		range[2] = range[2] < min_y ? range[2] : min_y;
		range[3] = range[3] > max_y ? range[3] : max_y;
	}

	void Scatter::show(HDC hdc, HWND hwnd, RECT rect, const std::vector<double>& range) const
	{
		// Set the adjusted min and max values, adjusted for the free space before/after 
		// first/last point
		double adj_min_x = range[0];
		double adj_max_x = range[1];
		double adj_min_y = range[2];
		double adj_max_y = range[3];

		// Pre-compute variables
		double win_length_x = rect.right - rect.left;
		double win_length_y = rect.bottom - rect.top;
		double length_x = adj_max_x - adj_min_x;
		double length_y = adj_max_y - adj_min_y;
		unsigned int x_coord, y_coord;

		// Set appropriate graph properties and coordinates
		HPEN hGraphPen = CreatePen(PS_SOLID, size, color);
		HBRUSH hGraphBrush = CreateSolidBrush(color);
		SelectObject(hdc, hGraphPen);
		SelectObject(hdc, hGraphBrush);

		// Draw the circles
		unsigned int x_size = x.size();
		for (unsigned int i = 0; i != x_size; ++i)
		{
			x_coord = rect.left + 
				(unsigned int)round((x[i] - adj_min_x) * win_length_x / length_x);
			y_coord = rect.bottom - 
				(unsigned int)round((y[i] - adj_min_y) * win_length_y / length_y);

			Ellipse(hdc, x_coord - 1, y_coord - 1, x_coord + 1, y_coord + 1);
		}
		
		// Clean the graphic objects
		DeleteObject(hGraphPen);
		DeleteObject(hGraphBrush);
	}

	// Derived class for line plots
	class Line : public Graph
	{
	public:
		Line() = default;

		virtual void prepare(const std::vector<double>& in_x, const std::vector<double>& in_y, 
			unsigned int in_size, COLORREF in_color, std::vector<double>& range);

		virtual void show(HDC hdc, HWND hwnd, RECT rect, const std::vector<double>& range) const;

		virtual ~Line() = default;

	private:
		std::map<double, double> data;
	};

	void Line::prepare(const std::vector<double>& in_x, const std::vector<double>& in_y, 
		unsigned int in_size, COLORREF in_color, std::vector<double>& range)
	{
		for (unsigned int i = 0; i != in_x.size(); ++i)
		{
			data[in_x[i]] = in_y[i]; // TODO : EMPLACE ?
		}

		size = in_size;
		color = in_color;

		// Find min and max of x and y 
		double min_x = data.begin()->first;
		double max_x = (--data.end())->first;
		double max_y = std::max_element(data.begin(), data.end(),
			[](const std::pair<double, double>& a, const std::pair<double, double>&b)
		{ return a.second < b.second;  })->second;
		double min_y = std::min_element(data.begin(), data.end(),
			[](const std::pair<double, double>& a, const std::pair<double, double>&b)
		{ return a.second < b.second;  })->second;

		// Set x and y range for Window data member range
		range[0] = range[0] < min_x ? range[0] : min_x;
		range[1] = range[1] > max_x ? range[1] : max_x;
		range[2] = range[2] < min_y ? range[2] : min_y;
		range[3] = range[3] > max_y ? range[3] : max_y;
	}

	void Line::show(HDC hdc, HWND hwnd, RECT rect, const std::vector<double>& range) const
	{
		double adj_min_x = range[0];
		double adj_max_x = range[1];
		double adj_min_y = range[2];
		double adj_max_y = range[3];

		// Pre-compute variables
		double win_length_x = rect.right - rect.left;
		double win_length_y = rect.bottom - rect.top;
		double length_x = adj_max_x - adj_min_x;
		double length_y = adj_max_y - adj_min_y;

		// Move to the starting point
		unsigned int x_coord = rect.left + 
			(unsigned int)round((data.begin()->first - adj_min_x) * win_length_x / length_x);
		unsigned int y_coord = rect.bottom - 
			(unsigned int)round((data.begin()->second - adj_min_y) * win_length_y / length_y);
		MoveToEx(hdc, x_coord, y_coord, NULL);

		// Set appropriate graph coordinates and draw the lines
		HPEN hGraphPen = CreatePen(PS_SOLID, size, color);
		SelectObject(hdc, hGraphPen);
		for (std::map<double, double>::const_iterator it = data.begin(); it != data.end(); ++it)
		{
			x_coord = rect.left + 
				(unsigned int)round((it->first - adj_min_x) * win_length_x / length_x);
			y_coord = rect.bottom - 
				(unsigned int)round((it->second - adj_min_y) * win_length_y / length_y);

			LineTo(hdc, x_coord, y_coord);
		}

		// Delete graphics objects
		DeleteObject(hGraphPen);
	}

	
	class Axis
	{
	public:
		void show(HDC hdc, HWND hwnd, RECT x_rect, RECT y_rect, 
			const std::vector<double>& range, HFONT font) const;

		bool get_xlabel() 
		{
			if (!xlabel.empty())
			{
				return true;
			}

			return false;
		}

		bool get_ylabel()
		{
			if (!ylabel.empty())
			{
				return true;
			}

			return false;
		}

		void set_xlabel(std::string xlab) {	xlabel = xlab; }

		void set_ylabel(std::string ylab) {	ylabel = ylab; }

	private:
		std::string xlabel;
		std::string ylabel;
	};

	void Axis::show(HDC hdc, HWND hwnd, RECT x_rect, RECT y_rect, 
		const std::vector<double>& range, HFONT font) const
	{
		// Set default text alignment
		unsigned int prev_text_align = SetTextAlign(hdc, TA_CENTER | TA_TOP);

		// Get parameters of current font -> for proper rendering of axis values
		TEXTMETRIC textMetric;
		GetTextMetrics(hdc, &textMetric);

		// Set length of axis ticks
		unsigned int x_stick = (unsigned int)(((x_rect.bottom - x_rect.top) - 
			2.0 * textMetric.tmHeight) / 3.0);
		unsigned int y_stick = (unsigned int)(((y_rect.right - y_rect.left) - 
			2.0 * textMetric.tmHeight) / 3.0);

		/* *********************************
		// X axis ticks and values rendering
		********************************* */

		double x_factor = 0.0;
		double x_diff = range[1] - range[0];
		unsigned int x_op_sign = 0; // nothing, mult or div

		// Find the multiplicative/divisive factor
		if ((x_diff + FP_ERROR) < 10.0)
		{
			x_op_sign = 1;

			x_factor += 1.0;
			x_diff *= 10.0;

			while ((x_diff + FP_ERROR) < 10.0)
			{
				x_factor += 1.0;
				x_diff *= 10.0;
			}
		}
		else if ((x_diff - FP_ERROR) >= 100.0)
		{
			x_op_sign = 2;

			x_factor += 1.0;
			x_diff /= 10.0;

			while ((x_diff - FP_ERROR) >= 100.0)
			{
				x_factor += 1.0;
				x_diff /= 10.0;
			}
		}

		// Find the appropriate tick and value dispersion 
		double x_value_period = 2.0;
		double x_tick_period = 1.0;
		if (x_diff >= 20.0 && x_diff < 40.0)
		{
			x_value_period = 5.0;
			x_tick_period = 2.5;
		}
		else if (x_diff >= 40.0)
		{
			x_value_period = 10.0;
			x_tick_period = 5.0;
		}
		
		// Set the graph-specific tick and value dispersion
		double x_value_period_adj = x_value_period;
		double x_tick_period_adj = x_tick_period;
		switch (x_op_sign)
		{
		case 1 :
		{
			x_value_period_adj /= pow(10.0, x_factor);
			x_tick_period_adj /= pow(10.0, x_factor);
		}
		break;
		case 2 :
		{
			x_value_period_adj *= pow(10.0, x_factor);
			x_tick_period_adj *= pow(10.0, x_factor);
		}
		break;
		}

		// Find the starting values for the first tick and value
		unsigned int x_value_divisor = (int)ceil(range[0] / x_value_period_adj);
		unsigned int x_tick_divisor = (int)ceil(range[0] / x_tick_period_adj);

		double x_value = x_value_divisor * x_value_period_adj;
		double x_tick = x_tick_divisor * x_tick_period_adj;

		// Pre-compute variables
		double win_length_x = x_rect.right - x_rect.left;
		double length_x = range[1] - range[0];

		unsigned int x_coord, y_coord = x_rect.top;
		
		// Set graphics attributes
		HPEN hBoxPen = CreatePen(PS_SOLID, 1, BLACK);
		SelectObject(hdc, hBoxPen);

		// Render ticks
		while (x_tick < range[1])
		{
			x_coord = x_rect.left +
				(unsigned int)round((x_tick - range[0]) * win_length_x / length_x);

			MoveToEx(hdc, x_coord, y_coord, NULL);
			LineTo(hdc, x_coord, y_coord + x_stick);

			x_tick += x_tick_period_adj;
		}

		// Render values
		int text_factor = 2; // adjustment of the value below the bottom of the graph
		y_coord += text_factor * x_stick;
		std::wstring x_text;

		// Test needed due to correct rendering of the notation of the number
		if (x_op_sign == 0 || x_op_sign == 2)
		{
			while (x_value < range[1])
			{
				x_coord = x_rect.left +
					(unsigned int)round((x_value - range[0]) * win_length_x / length_x);


				x_text = std::to_wstring((int)x_value);
				TextOut(hdc, x_coord, y_coord, x_text.c_str(), x_text.size());

				x_value += x_value_period_adj;
			}
		}
		else
		{
			while (x_value < range[1])
			{
				x_coord = x_rect.left +
					(unsigned int)round((x_value - range[0]) * win_length_x / length_x);

				x_text = std::to_wstring(x_value);
				TextOut(hdc, x_coord, y_coord, x_text.c_str(), x_text.size());

				x_value += x_value_period_adj;
			}
		}

		// Write the label of x axis
		unsigned int x_coord_xlabel = (x_rect.right + x_rect.left) / 2;
		unsigned int y_coord_xlabel = x_rect.top + 3 * x_stick + textMetric.tmHeight;
		wchar_t * wide_xlabel = new wchar_t[xlabel.size() + 1];
		MultiByteToWideChar(CP_UTF8, 0, xlabel.c_str(), -1, wide_xlabel, xlabel.size() + 1);
		TextOut(hdc, x_coord_xlabel, y_coord_xlabel, wide_xlabel, xlabel.size());
		
		delete[] wide_xlabel;

		/* *********************************
		// Y axis ticks and values rendering
		********************************* */
		
		double y_factor = 0.0;
		double y_diff = range[3] - range[2];

		unsigned int y_op_sign = 0; // nothing, mult or div

		// Find the multiplicative/divisive factor
		if ((y_diff + FP_ERROR) < 10.0)
		{
			y_op_sign = 1;

			y_factor += 1.0;
			y_diff *= 10.0;

			while ((y_diff + FP_ERROR) < 10.0)
			{
				y_factor += 1.0;
				y_diff *= 10.0;
			}
		}
		else if ((y_diff - FP_ERROR) >= 100.0)
		{
			y_op_sign = 2;

			y_factor += 1.0;
			y_diff /= 10.0;

			while ((y_diff - FP_ERROR) >= 100.0)
			{
				y_factor += 1.0;
				y_diff /= 10.0;
			}
		}

		// Find the appropriate tick and value dispersion 
		double y_value_period = 2.0;
		double y_tick_period = 1.0;
		if (y_diff >= 20.0 && y_diff < 40.0)
		{
			y_value_period = 5.0;
			y_tick_period = 2.5;
		}
		else if (y_diff >= 40.0)
		{
			y_value_period = 10.0;
			y_tick_period = 5.0;
		}

		// Set the graph-specific tick and value dispersion
		double y_value_period_adj = y_value_period;
		double y_tick_period_adj = y_tick_period;
		switch (y_op_sign)
		{
		case 1:
		{
			y_value_period_adj /= pow(10.0, y_factor);
			y_tick_period_adj /= pow(10.0, y_factor);
		}
		break;
		case 2:
		{
			y_value_period_adj *= pow(10.0, y_factor);
			y_tick_period_adj *= pow(10.0, y_factor);
		}
		break;
		}

		// Find the starting values for the first tick and value
		int y_value_divisor = (int)ceil(range[2] / y_value_period_adj);
		int y_tick_divisor = (int)ceil(range[2] / y_tick_period_adj);

		double y_value = y_value_divisor * y_value_period_adj;
		double y_tick = y_tick_divisor * y_tick_period_adj;

		// Pre-compute variables
		double win_length_y = y_rect.bottom - y_rect.top;
		double length_y = range[3] - range[2];

		x_coord = y_rect.right;

		// Render ticks
		while (y_tick < range[3])
		{
			y_coord = y_rect.bottom -
				(unsigned int)round((y_tick - range[2]) * win_length_y / length_y);

			MoveToEx(hdc, x_coord, y_coord, NULL);
			LineTo(hdc, x_coord - y_stick, y_coord);

			y_tick += y_tick_period_adj;
		}

		// Render values
		x_coord -= (unsigned int)(text_factor * y_stick);
		std::wstring y_text;

		// Set bottom and center text alignment
		SetTextAlign(hdc, TA_CENTER | TA_BOTTOM);

		// Set font rotated by 90 degrees
		LOGFONT lf;
		GetObject(font, sizeof(LOGFONT), &lf);
		lf.lfEscapement = 900;
		HFONT lfont = CreateFontIndirect(&lf);
		HFONT h_prev_font = (HFONT)SelectObject(hdc, (HGDIOBJ)(HFONT)(lfont));

		// Test needed due to correct rendering of the notation of the number
		if (y_op_sign == 0 || y_op_sign == 2)
		{
			while (y_value < range[3])
			{
				y_coord = y_rect.bottom -
					(unsigned int)round((y_value - range[2]) * win_length_y / length_y);

				y_text = std::to_wstring((int)y_value);
				TextOut(hdc, x_coord, y_coord, y_text.c_str(), y_text.size());

				y_value += y_value_period_adj;
			}
		}
		else
		{
			while (y_value < range[3])
			{
				y_coord = y_rect.bottom -
					(unsigned int)round((y_value - range[2]) * win_length_y / length_y);

				y_text = std::to_wstring(y_value);
				TextOut(hdc, x_coord, y_coord, y_text.c_str(), y_text.size());

				y_value += y_value_period_adj;
			}
		}

		// Write the label of y axis
		unsigned int x_coord_ylabel = y_rect.right - 3 * y_stick - textMetric.tmHeight;
		unsigned int y_coord_ylabel = (y_rect.bottom + y_rect.top) / 2;
		wchar_t * wide_ylabel = new wchar_t[ylabel.size() + 1];
		MultiByteToWideChar(CP_UTF8, 0, ylabel.c_str(), -1, wide_ylabel, ylabel.size() + 1);
		TextOut(hdc, x_coord_ylabel, y_coord_ylabel, wide_ylabel, ylabel.size());

		delete[] wide_ylabel;

		// Set text alignment and font that was in place before rendering axis attributes
		SetTextAlign(hdc, prev_text_align);
		SelectObject(hdc, h_prev_font);

		// Set previous graphics properties and delete graphic objects
		DeleteObject(hBoxPen);
	}

	// Window class
	class Window
	{
	public:
		Window() = delete;

		Window(COLORREF in_color) :
			background_color(in_color), initialized{ false }, first_show{ false },
			active_graph { 0 }, max_graphs{ MAX_GRAPHS }, 
			xy_range{ INFINITY, -INFINITY, INFINITY, -INFINITY }, axis{ new Axis() }
		{
			graph = alloc.allocate(max_graphs);
		};

		void prepare(const std::vector<double>& in_x, const std::vector<double>& in_y, 
			std::string in_type, unsigned int in_size, COLORREF in_color);

		void prepare(const std::vector<double>& in_y, std::string in_type, unsigned int in_size, 
			COLORREF in_color);

		void show(HDC hdc, HWND hwnd, RECT rect, HFONT font);

		bool init_test() const { return initialized; }

		void resize();

		void set_xlabel(std::string xlab);

		void set_ylabel(std::string ylab);

		~Window();

	private:
		Graph **graph;
		Axis * axis;
		std::allocator<Graph*> alloc;

		COLORREF background_color;
		bool initialized, first_show; /* indicator whether window is initialized 
									  and whether this is the first showing */
		unsigned int active_graph, max_graphs;
		std::vector<double> xy_range; // min_x, max_x, min_y, max_y
	};

	inline void Window::show(HDC hdc, HWND hwnd, RECT rect, HFONT font)
	{
		/*
		// Fill the client area with a brush TODO
		HRGN BackgroundRegion = CreateRectRgnIndirect(&rect);
		HBRUSH hBackgroundBrush = CreateSolidBrush(background_color);
		FillRgn(hdc, BackgroundRegion, hBackgroundBrush);
		*/

		// Set the adjusted min and max values, adjusted for the free space before/after 
		// first/last point at the initialization point
		if (!first_show)
		{
			xy_range[1] += (xy_range[1] - xy_range[0]) * ADJUSTMENT_GRAPH;
			xy_range[0] -= (xy_range[1] - xy_range[0]) * ADJUSTMENT_GRAPH;
			xy_range[3] += (xy_range[3] - xy_range[2]) * ADJUSTMENT_GRAPH;
			xy_range[2] -= (xy_range[3] - xy_range[2]) * ADJUSTMENT_GRAPH;

			first_show = true;
		}

		// Set rectangle for graph space
		RECT graph_rect = { 
			rect.left + ADJUSTMENT_WINDOW, 
			rect.top + ADJUSTMENT_WINDOW,
			rect.right - ADJUSTMENT_WINDOW,
			rect.bottom - ADJUSTMENT_WINDOW 
		};

		// Get parameters of current font and get the offset of 
		TEXTMETRIC textMetric;
		GetTextMetrics(hdc, &textMetric);

		// TODO : PARAMETERS AS COMPILER CONSTANTS
		double ratio = 1.5 / 5.0;
		double x_offset = textMetric.tmHeight * 
			(axis->get_xlabel() ? 5 : (4 / 1.5));
		double y_offset = textMetric.tmHeight * 
			(axis->get_ylabel() ? 5 : (4 / 1.5));
		
		HPEN hBoxPen = CreatePen(PS_SOLID, 1, BLACK);
		HGDIOBJ prev_hBoxPen = SelectObject(hdc, hBoxPen);
		// Rectangle(hdc, graph_rect.left, graph_rect.top, graph_rect.right, graph_rect.bottom);

		// Set rectangle for axis and values on axis
		RECT x_ticks, y_ticks;

		x_ticks.bottom = graph_rect.bottom;
		x_ticks.top = (unsigned int)(x_ticks.bottom - x_offset);

		y_ticks.left = graph_rect.left;
		y_ticks.right = (unsigned int)(y_ticks.left + y_offset);

		// Adjust the graph rectangle
		graph_rect.bottom = x_ticks.top;
		graph_rect.left = y_ticks.right;

		// Adjust the x and y tick rectangles
		x_ticks.right = graph_rect.right;
		x_ticks.left = graph_rect.left;

		y_ticks.top = graph_rect.top;
		y_ticks.bottom = graph_rect.bottom;
		
		// Draw and fill the enclosing rectangle
		hBoxPen = CreatePen(PS_SOLID, 1, BLACK);
		HBRUSH hGraphBrush = CreateSolidBrush(background_color);
		SelectObject(hdc, hBoxPen);
		HGDIOBJ prev_hGraphBrush = SelectObject(hdc, hGraphBrush);
		Rectangle(hdc, graph_rect.left, graph_rect.top, graph_rect.right, graph_rect.bottom);

		// Paint individual graphs
		for (unsigned int i = 0; i != active_graph; ++i)
		{
			graph[i]->show(hdc, hwnd, graph_rect, xy_range);
		}

		// Set previous options and delete graphics objects
		SelectObject(hdc, prev_hBoxPen);
		SelectObject(hdc, prev_hGraphBrush);
		DeleteObject(hBoxPen);
		DeleteObject(hGraphBrush);

		// Call the rendering of axis ticks and value signs
		axis->show(hdc, hwnd, x_ticks, y_ticks, xy_range, font);
	}

	inline void Window::prepare(const std::vector<double>& in_x, const std::vector<double>& in_y, 
		std::string in_type, unsigned int in_size, COLORREF in_color)
	{
		// Check for number of plotted graphs in the window and resize the buffer if needed
		if (active_graph >= max_graphs)
		{
			this->resize();
		}

		// Insert adequate Graph pointer 
		if (in_type == "scatter")
		{
			graph[active_graph] = new Scatter();
		}
		else
		{
			graph[active_graph] = new Line();
		}	

		// Send the variables to the Graph object
		graph[active_graph++]->prepare(in_x, in_y, in_size, in_color, xy_range);

		// Initialization of the window succedded
		initialized = true;
	}

	inline void Window::prepare(const std::vector<double>& in_y, std::string in_type, 
		unsigned int in_size, COLORREF in_color)
	{
		// Create points on the x-axis
		std::vector<double> x(in_y.size());
		for (unsigned int i = 1; i <= x.size(); ++i)
		{
			x[i - 1] = i;
		}

		// Check for number of plotted graphs in the window and resize the buffer if needed
		if (active_graph >= max_graphs)
		{
			this->resize();
		}

		// Insert adequate Graph pointer 
		if (in_type == "scatter")
		{
			graph[active_graph] = new Scatter();
		}
		else
		{
			graph[active_graph] = new Line();
		}

		// Send the variables to the Graph object
		graph[active_graph++]->prepare(x, in_y, in_size, in_color, xy_range);

		// Initialization of the window succedded
		initialized = true;
	}

	inline void Window::set_xlabel(std::string xlab)
	{
		axis->set_xlabel(xlab);
	}

	inline void Window::set_ylabel(std::string ylab)
	{
		axis->set_ylabel(ylab);
	}

	inline void Window::resize() 
	{ 
		// Allocate new, larger, storage
		max_graphs *= 2;
		Graph **new_graph = alloc.allocate(max_graphs);

		// Copy values from the original to the new storage
		for (int i = 0; i != active_graph; ++i)
		{
			new_graph[i] = graph[i];
		}

		// Deallocate original storage
		alloc.deallocate(graph, max_graphs / 2);
		
		graph = new_graph;
	};

	Window::~Window()
	{
		if (initialized)
		{
			// Deallocate individual Graph objects
			for (unsigned int i = 0; i != active_graph; ++i)
			{
				delete graph[i];
			}

			// Deallocate the storage of Graph pointers
			alloc.deallocate(graph, max_graphs);
		}
		else
		{
			printf("WARNING: There should be no reason to call the destructor at this point!");
		}

		// Delete Axis object
		delete axis;
	}

	class Figure
	{
	public:
		Figure(const std::vector<unsigned int>& width, const std::vector<unsigned int>& height, 
			const std::vector<COLORREF>& colors = std::vector<COLORREF>{});
		Figure(unsigned int width, unsigned int height, COLORREF colors = WHITE);

		// No copy constructor
		Figure(const Figure& figure) = delete;

		// No assignment operator
		Figure& operator=(const Figure& figure) = delete;
		
		// Destructor
		~Figure();

		void plot(const std::vector<double>& x, const std::vector<double>& y, 
			std::string type = "line", unsigned int width = 1, COLORREF color = WHITE,
			std::vector<unsigned int> position = std::vector<unsigned int>{});

		void plot(const std::vector<double>& y, std::string type = "line", 
			unsigned int width = 1, COLORREF color = WHITE, std::vector<unsigned int> position = 
			std::vector<unsigned int>{});

		void legend(std::string leg) {};

		void xlabel(std::string xlab);

		void ylabel(std::string ylab);

		void title(std::string title) {};

		void show()
		{
			// Initialize the window with adjusted window coordinates
			::InitializeWindow(win_width, win_height);
		};

		void set_font(int nHeight = 20, int nWidth = 0, int nEscapement = 0, int nOrientation = 0, 
			int fnWeight = FW_DONTCARE, DWORD fdwItalic = FALSE, DWORD fdwUnderline = FALSE,
			DWORD fdwStrikeOut = FALSE, DWORD fdwCharSet = ANSI_CHARSET, DWORD fdwOututPrecision =
			OUT_DEFAULT_PRECIS, DWORD fdwClipPrecision = CLIP_DEFAULT_PRECIS, DWORD fdwQuality = 
			DEFAULT_QUALITY, DWORD fdwPitchAndFamily = DEFAULT_PITCH | FF_SWISS, LPCTSTR lpszFace = 
			L"Arial");

		void paint(HDC hdc, HWND hwnd, RECT client_area);

		void save(std::string file);

		wchar_t *file_dir; // file directory -> public because needs to be seen by callback 

	private:
		unsigned int x_dim, y_dim; // dimensionality of the plotting area
		std::allocator<Window> alloc;
		Window * windows; // array of individual plots
		unsigned int win_width, win_height; // Width and Height of the GUI window
		std::vector<unsigned int> width, height; // user specified width and height
		std::vector<COLORREF> colors; // user specified colors of the windows
		HFONT font;

		int active_window;
	};

	Figure::Figure(const std::vector<unsigned int>& in_width, const std::vector<unsigned int>& in_height, 
		const std::vector<COLORREF>& in_colors) : x_dim{ in_width.size() }, y_dim{ in_height.size() }, 
		width{ in_width }, height{ in_height }, colors{ in_colors }, win_height{ 0 }, win_width{ 0 }, 
		active_window { -1 }, file_dir{ nullptr }
	{
		// Set default font
		LOGFONT lf;
		SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);
		font = CreateFontIndirect(&lf);

		// Check if dimensions of individual vectors are nonzero
		if ((x_dim == 0) || (y_dim == 0))
		{
			throw std::exception();
		}
		
		// Check if colors are proplerly set, otherwise set them to default WHITE
		if (colors.size() != (x_dim * y_dim))
		{
			for (unsigned int i = 0; i != (x_dim * y_dim); ++i)
			{
				colors.emplace_back(WHITE);
			}
		}

		// Set the cpplot::Globals::figure to current Figure instance
		Globals::figure = this;

		// Compute the overall width and height
		for (unsigned int i = 0; i != x_dim; ++i)
		{
			win_width += width[i];
		}

		for (unsigned int i = 0; i != y_dim; ++i)
		{
			win_height += height[i];
		}

		// Allocate space and construct individual Window objects
		windows = alloc.allocate(x_dim * y_dim);

		for (unsigned int i = 0; i != (x_dim * y_dim); ++i)
		{
			new (&windows[i]) Window(colors[i]);
		}
	}

	Figure::Figure(unsigned int in_width, unsigned int in_height, COLORREF colors) : 
		x_dim{ 1 }, y_dim{ 1 }, width(1, in_width), height(1, in_height), active_window{ -1 },
		file_dir{ nullptr }
	{
		// Set default font
		LOGFONT lf;
		SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);
		font = CreateFontIndirect(&lf);

		// Set the cpplot::Globals::figure to current Figure instance
		Globals::figure = this;

		// Set overall height and width
		win_height = height[0];
		win_width = width[0];

		// Allocate space and construct Window object
		windows = alloc.allocate(1);
		new (windows) Window(colors);

	}

	inline void Figure::plot(const std::vector<double>& x, const std::vector<double>& y, 
		std::string type, unsigned int width, COLORREF color, std::vector<unsigned int> position)
	{
		
	}

	inline void Figure::plot(const std::vector<double>& y, std::string type, unsigned int width,
		COLORREF color, std::vector<unsigned int> position)
	{
		unsigned int loc_active_window;

		// Check if the input position is default
		if (position.size() == 0)
		{
			++active_window;
			loc_active_window = active_window;
		}
		else if (position.size() != 2) // or if it is not of size 2
		{
			printf("Incompatible position! Default graph position taken!");

			++active_window;
			loc_active_window = active_window;
		}
		else
		{
			// Catch outside the boundaries positions of the graph
			if (position[0] >= y_dim || position[1] >= x_dim)
			{
				throw std::exception();
			}

			loc_active_window = x_dim * position[0] + position[1];

			active_window = loc_active_window;
		}

		// If the number of plot calls is larger than number of graphs
		if (loc_active_window >= (x_dim * y_dim))
		{
			printf("All individual graphs are already set! No action taken!");
			return;
		}

		// Send the variables to the selected Window
		windows[loc_active_window].prepare(y, type, width, color);
	}
	
	inline void Figure::paint(HDC hdc, HWND hwnd, RECT client_area)
	{
		// Select default font
		SelectObject(hdc, font);

		// Set default text alignment 
		SetTextAlign(hdc, TA_CENTER | TA_TOP);
			
		unsigned int pos_x;
		unsigned int pos_y;
		RECT rect;

		// Adjust new coordinates to the possibly resized window
		double width_ratio = (double)(client_area.right - client_area.left) / (double)win_width;
		double height_ratio = (double)(client_area.bottom - client_area.top) / (double)win_height;

		for (int i = 0; i != width.size(); ++i)
		{
			width[i] = (int)(width[i] * width_ratio);
		}

		for (int i = 0; i != height.size(); ++i)
		{
			height[i] = (int)(height[i] * height_ratio);
		}

		win_width = client_area.right - client_area.left;
		win_height = client_area.bottom - client_area.top;

		// Plot the individual windows
		for (unsigned int i = 0; i != (x_dim * y_dim); ++i)
		{
			// Set default text alignment 
			SetTextAlign(hdc, TA_CENTER);

			// Compute the position of the window in the plot
			pos_x = i / y_dim;
			pos_y = i - pos_x * y_dim;

			// Compute the RECT of that position
			rect.top = cumulative_sum(height, pos_y);
			rect.bottom = cumulative_sum(height, pos_y + 1);
			rect.left = cumulative_sum(width, pos_x);
			rect.right = cumulative_sum(width, pos_x + 1);

			if (!windows[i].init_test())
			{
				throw std::exception(); // TODO : Exception: uninitialized plot segment
			}

			// Generate graphs from individual windows
			windows[i].show(hdc, hwnd, rect, font);
		}
	};
	
	void Figure::set_font(int nHeight, int nWidth, int nEscapement, int nOrientation, 
		int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD 
		fdwCharSet, DWORD fdwOututPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, 
		DWORD fdwPitchAndFamily, LPCTSTR lpszFace)
	{
		// set custom font
		font = CreateFont(nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic, 
			fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOututPrecision, fdwClipPrecision, 
			fdwQuality, fdwPitchAndFamily, lpszFace);
	}

	void Figure::xlabel(std::string lab)
	{
		windows[active_window].set_xlabel(lab);
	}

	void Figure::ylabel(std::string lab)
	{
		windows[active_window].set_ylabel(lab);
	}

	void Figure::save(std::string file)
	{
		// Set the suffix type of saved image
		wchar_t file_type[] = L".bmp\0";
		
		// Allocate enough space to hold also the suffix of type
		file_dir = new wchar_t[file.size() + sizeof(file_type) / sizeof(wchar_t)]; 

		// Copy the original string to the file_dir buffer
		MultiByteToWideChar(CP_UTF8, 0, file.c_str(), -1, file_dir, file.size());

		// Copy the suffix type to the buffer
		wcscpy_s(file_dir + file.size(), sizeof(file_type) / sizeof(wchar_t), file_type);

		// Set the cpplot::Globals::dir, so the CALLBACK function can see it
		cpplot::Globals::dir = file_dir;

		// Finally, show the window with the plot
		this->show();
	}

	Figure::~Figure()
	{
		// Deallocate the storage of inidividual objects allocated with placement new
		for (unsigned int i = 0; i != (y_dim * x_dim); ++i)
		{
			windows[i].~Window();
		};

		// Free the buffer allocated by allocate
		alloc.deallocate(windows, x_dim * y_dim);

		// Delete file_dir, if allocated
		if (file_dir)
		{
			delete[] file_dir;
		}
	};

	inline unsigned int cumulative_sum(const std::vector<unsigned int>& container, size_t index)
	{
		assert(index <= container.size()); // TODO : COMMENT

		unsigned int cum_sum = 0;
		for (unsigned int i = 0; i != index; ++i)
		{
			cum_sum += container[i];
		}

		return cum_sum;
	}

}


// Callback function
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Variable for the readiness of the window
	static bool window_ready = false;

	switch (msg)
	{

	case WM_PAINT:
	{
		PAINTSTRUCT ps;

		RECT client_area;
		GetClientRect(hwnd, &client_area);
		
		HDC hdc = BeginPaint(hwnd, &ps);
		cpplot::Globals::figure->paint(hdc, hwnd, client_area);

		// Save the image and destroy window
		if (window_ready && cpplot::Globals::dir)
		{
			CreateImage(hwnd, hdc, cpplot::Globals::dir);
			cpplot::Globals::dir = nullptr;
			DestroyWindow(hwnd);
		}
		
		EndPaint(hwnd, &ps);
	}
	break;
	
	case WM_SHOWWINDOW:
	{
		window_ready = true;
	}
	break;
	
	case WM_WINDOWPOSCHANGED:
	{
		SendMessage(hwnd, WM_PAINT, NULL, NULL);
	}
	break;

	case WM_KEYDOWN:
	{
		if (wParam == VK_ESCAPE)
		{
			DestroyWindow(hwnd);
		}
	}
	break;

	case WM_CLOSE:
	{
		DestroyWindow(hwnd);
	}
	break;

	case WM_DESTROY:
	{
		PostQuitMessage(0);
	}
	break;

	default:
	{
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	}

	return 0;
}

// Initialize Window
int InitializeWindow(int width, int height)
{
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;
	HINSTANCE hInstance = GetModuleHandle(NULL);

	// Registering the Window
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	swprintf_s(cpplot::Globals::FigureName, cpplot::Globals::size, L"%d\0", 
		cpplot::Globals::counter++);
	wc.lpszClassName = cpplot::Globals::FigureName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&wc))
	{
		printf("Call to RegisterClassEx failed!");
		return 1;
	}

	// Creating the Window
	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		wc.lpszClassName,
		L"Plot",
		WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height,
		NULL, NULL, hInstance, NULL);

	if (!hwnd)
	{
		printf("Call to CreateWindow failed!");
		return 1;
	}
	
	// Set the client area to the desired size
	DWORD dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
	DWORD dwExStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

	RECT rc = { 0, 0, width, height };

	AdjustWindowRectEx(&rc, dwStyle, FALSE, dwExStyle);

	SetWindowPos(hwnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, NULL);

	// Show the window
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	// The message loop
	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return 0;
}
