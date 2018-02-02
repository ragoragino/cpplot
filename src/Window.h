#pragma once
#include "Header.h"
#include "Graph.h"

namespace cpplot
{
	// Window class
	class Window
	{
	public:
		Window() = delete;

		Window(COLORREF in_color) :
			background_color(in_color), initialized{ false }, first_show{ false },
			active_graph{ 0 }, max_graphs{ MAX_GRAPHS },
			xy_range{ INFINITY, -INFINITY, INFINITY, -INFINITY }, axis{ new Axis() }
		{
			graph = alloc.allocate(MAX_GRAPHS);
		};

		void prepare(const std::vector<double>& in_x, const std::vector<double>& in_y,
			std::string in_name, std::string in_type, unsigned int in_size,
			COLORREF in_color);

		void prepare(const std::vector<double>& in_y, std::string in_name,
			std::string in_type, unsigned int in_size, COLORREF in_color);

		void hist(const std::vector<double>& data, int bins, std::string name,
			unsigned int in_size, COLORREF color, bool normed);

		void hist(const std::vector<double>& data, const std::vector<double>& bins,
			std::string name, unsigned int in_size, COLORREF color, 
			bool normed);

		void show(HDC hdc, HWND hwnd, RECT rect, HFONT font);

		bool is_window_initialized() const { return initialized; }

		void resize();

		void set_xlabel(std::string xlab);

		void set_ylabel(std::string ylab);

		void set_title(std::string ylab);

		void activate_legend();

		~Window();

	private:
		Graph * *graph;
		Axis *axis;
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

		// Get parameters of current font
		TEXTMETRIC textMetric;
		GetTextMetrics(hdc, &textMetric);

		// Find which attributes the graph should have
		double x_tick_offset = axis->tick_xoffset(textMetric.tmHeight);
		double y_tick_offset = axis->tick_yoffset(textMetric.tmHeight);
		double x_label_offset = axis->label_xoffset(textMetric.tmHeight);
		double y_label_offset = axis->label_yoffset(textMetric.tmHeight);
		double title_offset = axis->title_offset(textMetric.tmHeight);
		double legend_offset = 0.0;

		// Find legend offset if the user wants the legend
		if (axis->is_legend_activated())
		{
			// Maximum legend area
			unsigned int legend_width = (unsigned int)((graph_rect.right -
				graph_rect.left) * MAX_LEGEND_RATIO - GRAPH_LEGEND_SPACE);

			legend_offset = axis->legend_offset(hdc, textMetric.tmAveCharWidth,
				legend_width);
		}

		// TODO : Delete -> for visual purposes
		HPEN hBoxPen = CreatePen(PS_SOLID, 1, BLACK);
		HGDIOBJ prev_hBoxPen = SelectObject(hdc, hBoxPen);
		Rectangle(hdc, graph_rect.left, graph_rect.top, graph_rect.right, graph_rect.bottom);

		// Prepare rectangle for axis and labels
		RECT x_ticks, y_ticks, x_label, y_label;

		// Set x and y label rectangles
		x_label.bottom = graph_rect.bottom;
		x_label.top = (unsigned int)(x_label.bottom - x_label_offset);

		y_label.left = graph_rect.left;
		y_label.right = (unsigned int)(y_label.left + y_label_offset);

		// Set x and y tick rectangles
		x_ticks.bottom = x_label.top;
		x_ticks.top = (unsigned int)(x_ticks.bottom - x_tick_offset);

		y_ticks.left = y_label.right;
		y_ticks.right = (unsigned int)(y_ticks.left + y_tick_offset);

		// Create rectangle for legend
		RECT legend = graph_rect;
		legend.right -= GRAPH_LEGEND_SPACE; // move also the right side of the legend
		legend.left = legend.right - (unsigned int)legend_offset;

		// Create title rectangle
		RECT title_rect = graph_rect;
		title_rect.bottom = (unsigned int)(title_rect.top + title_offset);

		// Adjust the graph rectangle
		graph_rect.bottom = x_ticks.top;
		graph_rect.left = y_ticks.right;
		graph_rect.right = legend.left - GRAPH_LEGEND_SPACE;
		graph_rect.top = title_rect.bottom;

		// Adjust axis attributes to fit the adjusted graph rectangle,
		// i.e. those sides which are not affected by that particular attribute
		x_ticks.left = graph_rect.left;
		x_ticks.right = graph_rect.right;
		y_ticks.top = graph_rect.top;
		y_ticks.bottom = graph_rect.bottom;
		x_label.left = graph_rect.left;
		x_label.right = graph_rect.right;
		y_label.top = graph_rect.top;
		y_label.bottom = graph_rect.bottom;
		legend.top = graph_rect.top;
		legend.bottom = graph_rect.bottom;
		title_rect.right = graph_rect.right;
		title_rect.left = graph_rect.left;

		// Draw and fill the enclosing rectangle
		hBoxPen = CreatePen(PS_SOLID, 1, BLACK);
		HBRUSH hGraphBrush = CreateSolidBrush(background_color);
		SelectObject(hdc, hBoxPen);
		HGDIOBJ prev_hGraphBrush = SelectObject(hdc, hGraphBrush);
		Rectangle(hdc, graph_rect.left, graph_rect.top, graph_rect.right, graph_rect.bottom);

		// Set previous options and delete graphics objects
		SelectObject(hdc, prev_hBoxPen);
		SelectObject(hdc, prev_hGraphBrush);
		DeleteObject(hBoxPen);
		DeleteObject(hGraphBrush);

		// Paint individual graphs
		for (unsigned int i = 0; i != active_graph; ++i)
		{
			graph[i]->show(hdc, hwnd, graph_rect, xy_range);
		}

		// Call the rendering of axis ticks and labels -> automatic
		axis->show_ticks(hdc, hwnd, x_ticks, y_ticks, xy_range, font);

		// Call the rendering of x_label
		if (axis->is_xlabel_activated())
		{
			axis->show_xlabel(hdc, hwnd, x_label, font);
		}

		// Call the rendering of y_label
		if (axis->is_ylabel_activated())
		{
			axis->show_ylabel(hdc, hwnd, y_label, font);
		}

		// Call the rendering of title
		if (axis->is_title_activated())
		{
			axis->show_title(hdc, hwnd, title_rect, font);
		}

		// Call the rendering of legend
		if (axis->is_legend_activated())
		{
			axis->show_legend(hdc, hwnd, legend, font, textMetric.tmAveCharWidth,
				textMetric.tmHeight);
		}
	}

	inline void Window::prepare(const std::vector<double>& in_x, const std::vector<double>& in_y,
		std::string in_name, std::string in_type, unsigned int in_size, COLORREF in_color)
	{
		// Check for number of plotted graphs in the window and resize the buffer if needed
		if (active_graph >= max_graphs)
		{
			this->resize();
		}

		// Insert adequate Graph pointer 
		if (in_type == "scatter")
		{
			graph[active_graph++] = new Scatter(in_x, in_y, in_size, in_color, xy_range);
		}
		else if (in_type == "line")
		{
			graph[active_graph++] = new Line(in_x, in_y, in_size, in_color, xy_range);
		}
		else
		{
			printf("Warning: Unrecognized plot type selected."
				"Line type is initialized.");

			graph[active_graph++] = new Line(in_x, in_y, in_size, in_color, xy_range);
		}


		// Set legend parameters
		axis->set_legend(in_name, in_type, in_color, in_size);

		// Initialization of the window succedded
		initialized = true;
	}

	inline void Window::prepare(const std::vector<double>& in_y, std::string in_name,
		std::string in_type, unsigned int in_size, COLORREF in_color)
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
			graph[active_graph++] = new Scatter(x, in_y, in_size, in_color, xy_range);
		}
		else if (in_type == "line")
		{
			graph[active_graph++] = new Line(x, in_y, in_size, in_color, xy_range);
		}
		else
		{
			printf("Warning: Unrecognized plot type selected."
				"Line type is initialized.");

			graph[active_graph++] = new Line(x, in_y, in_size, in_color, xy_range);
		}

		// Set legend parameters
		axis->set_legend(in_name, in_type, in_color, in_size);

		// Initialization of the window succedded
		initialized = true;
	}

	void Window::hist(const std::vector<double>& data, int bins, std::string name,
		unsigned int in_size, COLORREF color, bool normed)
	{
		// Check for number of plotted graphs in the window and resize the buffer if needed
		if (active_graph >= max_graphs)
		{
			this->resize();
		}

		// Insert adequate Graph pointer 
		graph[active_graph++] = new Histogram(data, bins, in_size, color, normed, xy_range);

		// Set legend parameters
		axis->set_legend(name, "hist", color, in_size);

		// Initialization of the window succedded
		initialized = true;
	}

	void Window::hist(const std::vector<double>& data, const std::vector<double>& bins,
		std::string name, unsigned int in_size, COLORREF color, bool normed)
	{
		// Check for number of plotted graphs in the window and resize the buffer if needed
		if (active_graph >= max_graphs)
		{
			this->resize();
		}

		// Insert adequate Graph pointer 
		graph[active_graph++] = new Histogram(data, bins, in_size, color, normed, xy_range);

		// Set legend parameters
		axis->set_legend(name, "hist", color, in_size);

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

	inline void Window::set_title(std::string ylab)
	{
		axis->set_title(ylab);
	}

	inline void Window::activate_legend()
	{
		axis->activate_legend();
	}

	inline void Window::resize()
	{
		// Allocate new, larger, storage
		max_graphs *= 2;
		Graph **new_graph = alloc.allocate(max_graphs);

		// Copy values from the original to the new storage
		for (unsigned int i = 0; i != active_graph; ++i)
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
			printf("WARNING: There should be no reason to call"
				"the destructor at this point!");
		}

		// Delete Axis object
		delete axis;
	}
}