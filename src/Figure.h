#include "Header.h"
#include "Window.h"
#include "Bitmap.h"

/*
TODO:
- exceptions
- finish assymetric Figure constructor
- check for data
- normed histogram
- finalize CreateImage class
*/

int InitializeWindow(int width, int height);

namespace cpplot
{
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
	
		void plot(const std::vector<double>& x, const std::vector<double>& y, 
			std::string name = "", std::string type = "line", unsigned int width = 1,
			COLORREF color = WHITE, const std::vector<unsigned int>& position =
			std::vector<unsigned int>{});

		void plot(const std::vector<double>& y, std::string name = "", std::string
			type = "line", unsigned int width = 1, COLORREF color = WHITE, 
			const std::vector<unsigned int>& position = std::vector<unsigned int>{});

		void fplot(double(*func)(double x), double from, double to,
			std::string name = "", std::string type = "line", 
			unsigned int width = 1, COLORREF color = WHITE,
			const std::vector<unsigned int>& position = std::vector<unsigned int>{});

		void hist(const std::vector<double>& data, int bins, std::string name = "",
			unsigned int size = 1.0, COLORREF color = BLUE, bool normed = false,
			const std::vector<unsigned int>& position = std::vector<unsigned int>{});

		void hist(const std::vector<double>& data, const std::vector<double>& bins,
			std::string name = "", unsigned int size = 1.0, COLORREF color = BLUE, bool normed = false,
			const std::vector<unsigned int>& position = std::vector<unsigned int>{});

		void plot_check(const std::vector<unsigned int>& position, unsigned int& local_window);

		void xlabel(std::string xlab);

		void ylabel(std::string ylab);

		void title(std::string title);

		void legend();

		void show()
		{
			// Initialize the window with adjusted window coordinates
			::InitializeWindow(win_width, win_height);
		};

		~Figure();

		void paint(HDC hdc, HWND hwnd, RECT client_area);

		void save(std::string file);

		wchar_t *file_dir; // file directory

	private:
		unsigned int x_dim, y_dim; // dimensionality of the plotting area
		std::allocator<Window> alloc;
		Window *windows; // array of individual plots
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
			printf("Warning: Unrecognized color dimension selected."
				"White color is applied.");

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
		std::string name, std::string type, unsigned int width, COLORREF color, 
		const std::vector<unsigned int>& position)
	{
		unsigned int loc_active_window = 0;

		// Perform all the necessary controls of input position
		// and fill in current active window
		this->plot_check(position, loc_active_window);

		// Send the variables to the selected Window
		windows[loc_active_window].prepare(x, y, name, type, width, color);
	}

	inline void Figure::plot(const std::vector<double>& y, std::string name,
		std::string type, unsigned int width, COLORREF color, 
		const std::vector<unsigned int>& position)
	{
		unsigned int loc_active_window = 0;

		// Perform all the necessary controls of input position
		// and fill in current active window
		this->plot_check(position, loc_active_window);

		// Send the variables to the selected Window
		windows[loc_active_window].prepare(y, name, type, width, color);
	}

	void Figure::fplot(double(*func)(double x), double from, double to, 
		std::string name, std::string type, unsigned int width, 
		COLORREF color, const std::vector<unsigned int>& position)
	{
		static constexpr unsigned int length = 1000;
		std::vector<double> x(length);
		std::vector<double> y(length);

		for (int i = 0; i != length; ++i)
		{
			x[i] = from + i * (to - from) / length;
			y[i] = func(x[i]);
		}

		this->plot(y, name, type, width, color, position);
	}

	inline void Figure::hist(const std::vector<double>& data, int bins, std::string name,
		unsigned int size, COLORREF color, bool normed, const std::vector<unsigned int>& position)
	{
		unsigned int loc_active_window = 0;

		// Perform all the necessary controls of input position
		// and fill in current active window
		this->plot_check(position, loc_active_window);

		// Send the variables to the selected Window
		windows[loc_active_window].hist(data, bins, name, size, color, normed);
	}
	
	inline void Figure::hist(const std::vector<double>& data, const std::vector<double>& bins,
		std::string name, unsigned int size, COLORREF color, bool normed,
		const std::vector<unsigned int>& position)
	{
		unsigned int loc_active_window = 0;

		// Perform all the necessary controls of input position
		// and fill in current active window
		this->plot_check(position, loc_active_window);

		// Send the variables to the selected Window
		windows[loc_active_window].hist(data, bins, name, size, color, normed);
	}

	inline void Figure::plot_check(const std::vector<unsigned int>& position, unsigned int& local_window)
	{
		// Check if the input position is default
		if (position.size() == 0)
		{
			++active_window;
			local_window = active_window;
		}
		else if (position.size() != 2) // or if it is not of size 2
		{
			printf("Incompatible position! Default graph position taken!");

			++active_window;
			local_window = active_window;
		}
		else
		{
			// Catch outside the boundaries positions of the graph
			if (position[0] >= y_dim || position[1] >= x_dim)
			{
				throw std::exception();
			}

			local_window = x_dim * position[0] + position[1];

			active_window = local_window;
		}

		// If the number of plot calls is larger than number of graphs
		if (local_window >= (x_dim * y_dim))
		{
			printf("All individual graphs are already set! No action taken!");
			return;
		}
	}

	inline void Figure::paint(HDC hdc, HWND hwnd, RECT client_area)
	{
		// Select default font
		SelectObject(hdc, font);

		// Set default text alignment 
		SetTextAlign(hdc, TA_CENTER | TA_TOP);
	
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
		unsigned int pos_x;
		unsigned int pos_y;
		RECT rect;

		for (unsigned int i = 0; i != (x_dim * y_dim); ++i)
		{
			if (!windows[i].is_window_initialized())
			{
				throw std::exception(); // TODO : Exception: uninitialized plot segment
			}

			// Compute the beginning position x and y of the window in the plot
			pos_x = i / y_dim;
			pos_y = i - pos_x * y_dim;

			// Compute the RECT of that position
			rect.top = cumulative_sum(height, pos_y);
			rect.bottom = cumulative_sum(height, pos_y + 1);
			rect.left = cumulative_sum(width, pos_x);
			rect.right = cumulative_sum(width, pos_x + 1);

			// Generate and show contents of individual windows
			windows[i].show(hdc, hwnd, rect, font);
		}
	};

	inline void Figure::xlabel(std::string lab)
	{
		windows[active_window].set_xlabel(lab);
	}

	inline void Figure::ylabel(std::string lab)
	{
		windows[active_window].set_ylabel(lab);
	}

	inline void Figure::title(std::string lab)
	{
		windows[active_window].set_title(lab);
	}

	inline void Figure::legend()
	{
		for (unsigned int i = 0; i != (x_dim * y_dim); ++i)
		{
			windows[i].activate_legend();
		}
	}

	inline void Figure::save(std::string file)
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

		// Finally, show the whole figure
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
			CreateImage2(hwnd, hdc, cpplot::Globals::dir);
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
		cpplot::Globals::id++); // for each new Window change 
									 // increment 1 to the id
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
