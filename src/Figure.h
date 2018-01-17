#pragma once
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Gdi32.lib")

#include "Header.h"

#include <windows.h>

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
		static cpplot::Figure *figure;
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
		double adj_max_x = range[1] + (range[1] - range[0]) * ADJUSTMENT_GRAPH;
		double adj_min_x = range[0] - (range[1] - range[0]) * ADJUSTMENT_GRAPH;
		double adj_max_y = range[3] + (range[3] - range[2]) * ADJUSTMENT_GRAPH;
		double adj_min_y = range[2] - (range[3] - range[2]) * ADJUSTMENT_GRAPH;

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
		double max_x = data.begin()->first;
		double min_x = (--data.end())->first;
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
		
		// Set the adjusted min and max values, adjusted for the free space before/after 
		// first/last point
		double adj_max_x = range[1] + (range[1] - range[0]) * ADJUSTMENT_GRAPH;
		double adj_min_x = range[0] - (range[1] - range[0]) * ADJUSTMENT_GRAPH;
		double adj_max_y = range[3] + (range[3] - range[2]) * ADJUSTMENT_GRAPH;
		double adj_min_y = range[2] - (range[3] - range[2]) * ADJUSTMENT_GRAPH;

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
		virtual void prepare(const std::string& str);

		virtual void show(HDC hdc, HWND hwnd, RECT rect, const std::vector<double>& range) const;

		virtual ~Axis() = default;

	private:
		std::string text;
	};

	void Axis::prepare(const std::string& str)
	{
		text = str;
	}

	void Axis::show(HDC hdc, HWND hwnd, RECT rect, const std::vector<double>& range) const
	{

	}

	// Window class
	class Window
	{
	public:
		Window() = delete;

		Window(COLORREF in_color) :
			background_color(in_color), initialized{ false }, active_graph{ 0 }, 
			max_graphs{ MAX_GRAPHS }, xy_range{ INFINITY, -INFINITY, INFINITY, -INFINITY }
		{
			graph = alloc.allocate(max_graphs);
		};

		void prepare(const std::vector<double>& in_x, const std::vector<double>& in_y, 
			std::string in_type, unsigned int in_size, COLORREF in_color);

		void prepare(const std::vector<double>& in_y, std::string in_type, unsigned int in_size, 
			COLORREF in_color);

		void show(HDC hdc, HWND hwnd, RECT rect);

		bool init_test() const { return initialized; }

		void resize();

		~Window();

	private:
		Graph **graph;
		std::allocator<Graph*> alloc;

		COLORREF background_color;
		bool initialized;
		unsigned int active_graph, max_graphs;
		std::vector<double> xy_range; // min_x, max_x, min_y, max_y
	};

	inline void Window::show(HDC hdc, HWND hwnd, RECT rect)
	{
		/*
		// Fill the client area with a brush TODO
		HRGN BackgroundRegion = CreateRectRgnIndirect(&rect);
		HBRUSH hBackgroundBrush = CreateSolidBrush(background_color);
		FillRgn(hdc, BackgroundRegion, hBackgroundBrush);
		*/
		
		// Set rectangle for graph space
		RECT graph_rect = { rect.left + 10, rect.top + 10, rect.right - 10, rect.bottom - 10 };
		
		// Draw and fill the enclosing rectangle
		HPEN hBoxPen = CreatePen(PS_SOLID, 1, BLACK);
		HBRUSH hGraphBrush = CreateSolidBrush(background_color);
		SelectObject(hdc, hBoxPen);
		SelectObject(hdc, hGraphBrush);
		Rectangle(hdc, graph_rect.left, graph_rect.top, graph_rect.right, graph_rect.bottom);

		// Paint individual graphs
		for (unsigned int i = 0; i != active_graph; ++i)
		{
			graph[i]->show(hdc, hwnd, graph_rect, xy_range);
		}

		// Delete the graphics objects
		DeleteObject(hBoxPen);

		// Set rectangle for adjacent objects
		// TODO
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
		for (unsigned int i = 0; i != x.size(); ++i)
		{
			x[i] = i;
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
				graph[i]->~Graph();
			}

			// Deallocate the storage of Graph pointers
			alloc.deallocate(graph, max_graphs);
		}
		else
		{
			printf("WARNING: There should be no reason to call the destructor at this point!");
		}
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
		~Figure() 
		{ 
			// Deallocate the storage of inidividual objects allocated with placement new
			for (unsigned int i = 0; i != (y_dim * x_dim); ++i)
			{
				windows[i].~Window();
			};

			// Free the buffer allocated by malloc
			alloc.deallocate(windows, x_dim * y_dim);
		};

		void plot(const std::vector<double>& x, const std::vector<double>& y, 
			std::string type = "line", unsigned int width = 1, COLORREF color = WHITE,
			std::vector<unsigned int> position = std::vector<unsigned int>{});

		void plot(const std::vector<double>& y, std::string type = "line", 
			unsigned int width = 1, COLORREF color = WHITE, std::vector<unsigned int> position = 
			std::vector<unsigned int>{});

		void legend(std::string leg);

		void xlabel(std::string xlab);

		void ylabel(std::string ylab);

		void title(std::string title);

		void show()
		{
			// Initialize the window with adjusted window coordinates
			::InitializeWindow(win_width, win_height);
		};

		void paint(HDC hdc, HWND hwnd);

	private:
		RECT client_rect; // rectangle specifying coordinates of the client area of GUI window
		unsigned int x_dim, y_dim; // dimensionality of the plotting area
		std::allocator<Window> alloc;
		Window * windows; // array of individual plots
		unsigned int win_width, win_height; // Width and Height of the GUI window
		const std::vector<unsigned int> width, height; // user specified width and height
		std::vector<COLORREF> colors; // user specified colors of the windows

		int active_window;
	};

	Figure::Figure(const std::vector<unsigned int>& in_width, const std::vector<unsigned int>& in_height, 
		const std::vector<COLORREF>& in_colors) : x_dim{ in_width.size() }, y_dim{ in_height.size() }, 
		width{ in_width }, height{ in_height }, colors{ in_colors }, win_height{ 0 }, win_width{ 0 }, 
		active_window { -1 }
	{

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
		x_dim{ 1 }, y_dim{ 1 }, width(1, in_width), height(1, in_height), active_window{ -1 }
	{
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

	inline void Figure::paint(HDC hdc, HWND hwnd)
	{
		unsigned int pos_x;
		unsigned int pos_y;
		RECT rect;

		// Plot the individual windows
		for (unsigned int i = 0; i != (x_dim * y_dim); ++i)
		{
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
			windows[i].show(hdc, hwnd, rect);
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
	switch (msg)
	{

	case WM_PAINT:
	{
		PAINTSTRUCT ps;

		HDC hdc = BeginPaint(hwnd, &ps);
		cpplot::Globals::figure->paint(hdc, hwnd);
		EndPaint(hwnd, &ps);
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
		return DefWindowProc(hwnd, msg, wParam, lParam);
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
	swprintf_s(cpplot::Globals::FigureName, cpplot::Globals::size, L"%d", 
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
		NULL,
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





