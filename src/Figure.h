#include "Header.h"
#include "Window.h"
#include "Bitmap.h"
#include "Render.h"

namespace cpplot
{
	int InitializeWindow(int width, int height);

	class Figure
	{
	public:
		Figure(const std::vector<int>& width, const std::vector<int>&
			height, const std::vector<COLORREF>& colors = std::vector<COLORREF>{},
			bool in_divded = false);

		Figure(int width, int height, COLORREF colors = WHITE, bool in_divded = false);

		Figure(const Figure& figure) = delete;

		Figure& operator=(const Figure& figure) = delete;

		/*
		The functions are templated because I needed a hack how to tell
		whether the user specified a COLORREF or not, so that a default COLORREF
		from CircularArray for that particular window shall be assigned if not.
		*/
		template<typename T = bool>
		void plot(const std::vector<double>& x, const std::vector<double>& y,
			std::string name = "", std::string type = "line", int width = 1,
			T color = false, const std::vector<int>& position =
			std::vector<int>{}, RenderObjects *render_ptr = nullptr);

		template<typename T = bool>
		void plot(const std::vector<double>& y, std::string name = "", std::string
			type = "line", int width = 1, T color = false,
			const std::vector<int>& position = std::vector<int>{},
			RenderObjects *render_ptr = nullptr);

		template<typename T = bool>
		void fplot(double(*func)(double x), double from, double to,
			std::string name = "", std::string type = "line",
			int width = 1, T color = false,
			const std::vector<int>& position = std::vector<int>{},
			RenderObjects *render_ptr = nullptr);

		template<typename T = bool>
		void hist(const std::vector<double>& data, int bins, std::vector<double> range = {},
			std::string name = "", int size = 1.0, T color = false,
			bool normed = false, const std::vector<int>& position =
			std::vector<int>{});

		template<typename T = bool>
		void hist(const std::vector<double>& data, const std::vector<double>& bins,
			std::string name = "", int size = 1.0, T color = false,
			bool normed = false, const std::vector<int>&
			position = std::vector<int>{});

		void xlabel(std::string xlab);

		void ylabel(std::string ylab);

		void title(std::string title);

		void legend();

		void show()
		{
			// Initialize the window with adjusted window coordinates
			cpplot::InitializeWindow(win_width, win_height);
		};

		void paint(HDC hdc, HWND hwnd, RECT client_area);

		void save(std::string file, std::string extension);

		~Figure();

	private:
		void plot_check(const std::vector<int>& position, int&
			local_window);

		struct CircularArray
		{
			CircularArray() : value(0),
				colors{ RED, GREEN, BLUE, BLACK,
						GREY, ORANGE, PINK, YELLOW } {};

			COLORREF pop()
			{
				value %= length;
				return colors[value++];
			}

			static constexpr unsigned int length = 8; // number of default colors;
			COLORREF colors[length];
			int value;
		};

		int x_dim, y_dim; // dimensionality of the plotting area
		std::allocator<Window> alloc_windows;
		Window *windows; // array of individual plots
		CircularArray *circular; // array of default colors
		std::allocator<CircularArray> alloc_circular;
		int win_width, win_height; // original width and height of the GUI window
		std::vector<int> width, height; // user specified width and height
		std::vector<int> width_copy, height_copy; // width and height vectors for resizing
		std::vector<COLORREF> colors; // user specified colors of the windows
		HFONT font; // font of the rendering
		bool divided; // indicator whether individual windows should be divided by black line

		int active_window; // currently active window
	};

	Figure::Figure(const std::vector<int>& in_width, const std::vector<int>& in_height,
		const std::vector<COLORREF>& in_colors, bool in_divided) : x_dim((int)in_width.size()),
		y_dim((int)in_height.size()), width(in_width), height(in_height),
		width_copy(in_width.size()), height_copy(in_height.size()), colors(in_colors),
		win_height(0), win_width(0), active_window(-1), divided(in_divided)
	{
		// Save default font
		LOGFONT lf;
		SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);
		font = CreateFontIndirect(&lf);

		// Check if dimensions of individual vectors are nonzero
		if ((x_dim == 0) || (y_dim == 0))
		{
			printf("Warning: Wrong width or height input." 
				"Default plot dimensionality and width and height selected.\n");

			x_dim = 1;
			y_dim = 1;
			width = std::vector<int>{ 800 };
			height = std::vector<int>{ 800 };
			width_copy = std::vector<int>(1);
			height_copy = std::vector<int>(1);
		}

		// Check if colors are proplerly set, otherwise set them to default WHITE
		if (colors.size() != (x_dim * y_dim))
		{
			int size = 
				(int)colors.size() > (x_dim * y_dim) ? (x_dim * y_dim) : (int)colors.size();
			for (int i = size; i != (x_dim * y_dim); ++i)
			{
				colors.emplace_back(WHITE);			
			}
		}

		// Set the Globals::figure to current Figure instance
		Globals::figure = this;

		// Compute the overall width and height
		for (int i = 0; i != x_dim; ++i)
		{
			win_width += width[i];
		}

		for (int i = 0; i != y_dim; ++i)
		{
			win_height += height[i];
		}

		// Allocate space and construct individual Window objects
		windows = alloc_windows.allocate(x_dim * y_dim);
		circular = alloc_circular.allocate(x_dim * y_dim);

		for (int i = 0; i != (x_dim * y_dim); ++i)
		{
			new (&windows[i]) Window(colors[i]);
			new (&circular[i]) CircularArray();
		}
	}

	Figure::Figure(int in_width, int in_height, COLORREF colors, bool in_divided) :
		x_dim(1), y_dim(1), width(1, in_width), height(1, in_height),
		width_copy(1), height_copy(1), active_window(-1), divided(in_divided)
	{
		// Set default font
		LOGFONT lf;
		SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);
		font = CreateFontIndirect(&lf);

		// Set the Globals::figure to current Figure instance
		Globals::figure = this;

		// Set overall height and width
		win_height = height[0];
		win_width = width[0];

		// Allocate space and construct Window and CircularArray object
		windows = alloc_windows.allocate(1);
		new (windows) Window(colors);

		circular = alloc_circular.allocate(1);
		new (circular) CircularArray();
	}

	template<typename T>
	inline void Figure::plot(const std::vector<double>& x, const std::vector<double>& y,
		std::string name, std::string type, int width, T color,
		const std::vector<int>& position, RenderObjects *render_ptr)
	{
		// Check whether at least one data point in each container
		if (x.empty() || y.empty())
		{
			printf("Warning: X or Y container is empty. No action taken.\n");
			return;
		}

		int loc_active_window = 0;

		// Perform all the necessary controls of input position
		// and fill in current active window
		this->plot_check(position, loc_active_window);

		COLORREF loc_color = color;
		if (typeid(color) != typeid(COLORREF))
		{
			loc_color = circular[loc_active_window].pop();
		}

		// Send the variables to the selected Window
		windows[loc_active_window].prepare(x, y, name, type, width, loc_color, render_ptr);
	}

	template<typename T>
	inline void Figure::plot(const std::vector<double>& y, std::string name,
		std::string type, int width, T color,
		const std::vector<int>& position, RenderObjects *render_ptr)
	{
		// Check whether at least one data point in the container
		if (y.empty())
		{
			printf("Warning: Y container is empty. No action taken.\n");
			return;
		}

		int loc_active_window = 0;

		// Perform all the necessary controls of input position
		// and fill in current active window
		this->plot_check(position, loc_active_window);

		COLORREF loc_color = color;
		if (typeid(color) != typeid(COLORREF))
		{
			loc_color = circular[loc_active_window].pop();
		}

		// Send the variables to the selected Window
		windows[loc_active_window].prepare(y, name, type, width, loc_color, render_ptr);
	}

	template<typename T>
	void Figure::fplot(double(*func)(double x), double from, double to,
		std::string name, std::string type, int width,
		T color, const std::vector<int>& position,
		RenderObjects *render_ptr)
	{
		static constexpr int length = FPLOT_LENGTH;
		std::vector<double> x(length);
		std::vector<double> y(length);

		for (int i = 0; i != length; ++i)
		{
			x[i] = from + i * (to - from) / length;
			y[i] = func(x[i]);
		}

		this->plot(x, y, name, type, width, color, position, render_ptr);
	}

	template<typename T>
	inline void Figure::hist(const std::vector<double>& data, int bins,
		std::vector<double> range, std::string name, int size, T color,
		bool normed, const std::vector<int>& position)
	{
		// Check whether at least one data point in the container
		if (data.empty())
		{
			printf("Warning: Data container is empty. No action taken.\n");
			return;
		}

		int loc_active_window = 0;

		// Perform all the necessary controls of input position
		// and fill in current active window
		this->plot_check(position, loc_active_window);

		COLORREF loc_color = color;
		if (typeid(color) != typeid(COLORREF))
		{
			loc_color = circular[loc_active_window].pop();
		} 

		// Send the variables to the selected Window
		windows[loc_active_window].hist(data, bins, range, name, size, loc_color, normed);
	}

	template<typename T>
	inline void Figure::hist(const std::vector<double>& data, const std::vector<double>&
		bins, std::string name, int size, T color, bool normed,
		const std::vector<int>& position)
	{
		// Check whether at least one data point in the container
		if (data.empty())
		{
			printf("Warning: Data container is empty. No action taken.\n");
			return;
		}

		int loc_active_window = 0;

		// Perform all the necessary controls of input position
		// and fill in current active window
		this->plot_check(position, loc_active_window);

		COLORREF loc_color = color;
		if (typeid(color) != typeid(COLORREF))
		{
			loc_color = circular[loc_active_window].pop();
		}

		// Send the variables to the selected Window
		windows[loc_active_window].hist(data, bins, name, size, loc_color, normed);
	}

	inline void Figure::plot_check(const std::vector<int>& position, int& local_window)
	{
		// Check if the input position is default
		if (position.size() == 0)
		{
			++active_window;
			local_window = active_window;
		}
		else if (position.size() != 2) // or if it is not of size 2
		{
			printf("Warning: Incompatible position! Default graph position taken!\n");

			++active_window;
			local_window = active_window;
		}
		else
		{
			// Catch outside the boundaries positions of the window
			if (position[0] >= y_dim || position[1] >= x_dim)
			{
				printf("Warning: Out of boundaries position! " 
					"Default graph position taken!\n");

				++active_window;
				local_window = active_window;
			}
			else
			{
				local_window = x_dim * position[0] + position[1];
				active_window = local_window;
			}			
		}

		// If the number of plot calls is larger than number of graphs
		if (local_window >= (x_dim * y_dim))
		{
			active_window = x_dim * y_dim - 1;
			local_window = active_window;
		}
	}

	inline void Figure::paint(HDC hdc, HWND hwnd, RECT client_area)
	{
		if (active_window == -1)
		{
			printf("Warning: No window was properly initialized. No action taken.\n");

			SendMessage(hwnd, WM_CLOSE, NULL, NULL);

			return;
		}

		// In case the window is distorted (e.g. minimized), do not paint
		if (client_area.right - client_area.left <= 0 ||
			client_area.bottom - client_area.top <= 0)
		{
			return;
		}

		// Select default font
		SelectObject(hdc, font);

		// Set default text alignment 
		SetTextAlign(hdc, TA_CENTER | TA_TOP);

		// Adjust new coordinates to the possibly resized window
		double width_ratio =
			(double)(client_area.right - client_area.left) / (double)win_width;
		double height_ratio =
			(double)(client_area.bottom - client_area.top) / (double)win_height;
		for (int i = 0; i != width.size(); ++i)
		{
			width_copy[i] = (int)(width[i] * width_ratio);
		}
		for (int i = 0; i != height.size(); ++i)
		{
			height_copy[i] = (int)(height[i] * height_ratio);
		}

		// Divide the windows by black line if the user requested it
		if (divided)
		{
			HPEN hBoxPen = CreatePen(PS_SOLID, 1, BLACK);
			HGDIOBJ prev_hBoxPen = SelectObject(hdc, hBoxPen);

			int agg_width = 0, agg_height = 0;
			for (int i = 1; i != width.size(); ++i)
			{
				agg_width += width_copy[i];

				MoveToEx(hdc, agg_width, client_area.bottom, NULL);
				LineTo(hdc, agg_width, client_area.top);
			}
			for (int i = 1; i != height.size(); ++i)
			{
				agg_height += height_copy[i];

				MoveToEx(hdc, client_area.left, agg_height, NULL);
				LineTo(hdc, client_area.right, agg_height);
			}

			// Set previous options and delete graphics objects
			SelectObject(hdc, prev_hBoxPen);
			DeleteObject(hBoxPen);
		}

		// Plot the individual windows
		int pos_x, pos_y;
		RECT rect;

		for (int i = 0; i != (x_dim * y_dim); ++i)
		{
			if (!windows[i].is_window_initialized())
			{
				printf("Warning: The window is not initialized. No action taken.\n");

				return;
			}

			// Compute the beginning position x and y of the window in the plot
			pos_x = i / y_dim;
			pos_y = i - pos_x * y_dim;

			// Compute the RECT of that position
			rect.top = cumulative_sum(height_copy, pos_y);
			rect.bottom = cumulative_sum(height_copy, pos_y + 1);
			rect.left = cumulative_sum(width_copy, pos_x);
			rect.right = cumulative_sum(width_copy, pos_x + 1);

			// Generate and show contents of individual windows
			windows[i].show(hdc, hwnd, rect, font);
		}
	};

	inline void Figure::xlabel(std::string lab)
	{
		if (active_window > -1)
		{
			windows[active_window].set_xlabel(lab);
		}
	}

	inline void Figure::ylabel(std::string lab)
	{
		if (active_window > -1)
		{
			windows[active_window].set_ylabel(lab);
		}
	}

	inline void Figure::title(std::string lab)
	{
		if (active_window > -1)
		{
			windows[active_window].set_title(lab);
		}
	}

	inline void Figure::legend()
	{
		for (int i = 0; i != (x_dim * y_dim); ++i)
		{
			if (active_window > -1)
			{
				windows[i].activate_legend();
			}
		}
	}

	inline void Figure::save(std::string file, std::string extension)
	{
		// Set the suffix type of saved image
		wchar_t *ext_dir = new wchar_t[extension.size() + 1];
		MultiByteToWideChar(CP_UTF8, 0, extension.c_str(), -1, ext_dir, (int)extension.size() + 1);

		// Allocate enough space to hold also the suffix of type 
		// +2 because of dot and null termination
		wchar_t *file_dir = new wchar_t[file.size() + (int)extension.size() + 2];

		// Copy the original string to the file_dir buffer
		MultiByteToWideChar(CP_UTF8, 0, file.c_str(), -1, file_dir, (int)file.size() + 1);
		file_dir[file.size()] = L'.';

		// Copy the suffix type to the buffer
		wcscpy_s(file_dir + file.size() + 1, (int)extension.size() + 1, ext_dir);

		// Set the Globals::dir and Globals::ext, so the CALLBACK function can see it
		cpplot::Globals::dir = file_dir;
		cpplot::Globals::ext = ext_dir;

		// Finally, show the whole figure
		this->show();

		delete[] ext_dir;
		delete[] file_dir;
	}

	Figure::~Figure()
	{
		// Deallocate the storage of inidividual objects allocated with placement new
		for (int i = 0; i != (y_dim * x_dim); ++i)
		{
			windows[i].~Window();
			circular[i].~CircularArray();
		}

		// Free the buffer allocated by allocate
		alloc_windows.deallocate(windows, x_dim * y_dim);
		alloc_circular.deallocate(circular, x_dim * y_dim);
	};

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
			if (window_ready && cpplot::Globals::dir && cpplot::Globals::ext)
			{
				CreateImage(hwnd, hdc, cpplot::Globals::dir, cpplot::Globals::ext);
				cpplot::Globals::dir = nullptr;
				cpplot::Globals::ext = nullptr;
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
			printf("Call to RegisterClassEx failed!\n");
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
			printf("Call to CreateWindow failed!\n");
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
}
