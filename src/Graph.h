#pragma once
#include "Header.h"
#include "Constants.h"

namespace cpplot {

	class Figure;
	inline unsigned int cumulative_sum(const std::vector<unsigned int>& container, size_t index)
	{
		assert(index <= container.size());

		unsigned int cum_sum = 0;
		for (unsigned int i = 0; i != index; ++i)
		{
			cum_sum += container[i];
		}

		return cum_sum;
	}

	inline unsigned int get_frac_digits(double x, unsigned int precision)
	{
		unsigned int no_digits = precision;
		unsigned int power = (unsigned int)pow(10, precision);
		unsigned int modulo = 10;

		unsigned int int_part = (unsigned int)(abs(x));
		double decim_part = abs(x) - int_part;

		unsigned int indicator;

		while (modulo <= power)
		{
			indicator = (unsigned int)(decim_part * power + FP_ERROR) % modulo;

			if (indicator > 0)
			{
				// add 1 for the dot
				return ++no_digits;
			}

			modulo *= 10;
			--no_digits;
		}

		return no_digits;
	}

	inline unsigned int get_int_digits(double x)
	{
		unsigned int no_digits = 0;
		unsigned int int_part = (unsigned int)(abs(x));

		while (int_part >= 1)
		{
			int_part /= 10;
			++no_digits;
		}

		// In case x is negative, add also - sign
		if (x < 0)
		{
			++no_digits;
		}

		return no_digits;
	}

	// Global variables
	namespace Globals
	{
		// Maximum length of the ID (as wchar) of the Figure
		static constexpr unsigned int size = 10;

		// Buffer holding ID of the Figure
		static wchar_t FigureName[size];

		// ID of the current Figure
		static unsigned int id = 0;

		// Pointer to current Figure object
		static cpplot::Figure *figure = nullptr;

		// Pointer to directory for saving the image
		static wchar_t *dir = nullptr;

		// Type of the saved image
		static wchar_t* ext = nullptr;
	};

	// ABC
	class Graph
	{
	public:
		virtual void show(HDC hdc, HWND hwnd, RECT rect,
			const std::vector<double>& range) const = 0;

		virtual ~Graph() = default;

	protected:
		COLORREF color;
		unsigned int size;
	};

	// Derived class for scatterplots
	class Scatter : public Graph
	{
	public:
		Scatter(const std::vector<double>& in_x,
			const std::vector<double>& in_y, unsigned int in_size,
			COLORREF in_color, std::vector<double>& range);

		virtual void show(HDC hdc, HWND hwnd, RECT rect,
			const std::vector<double>& range) const;

		virtual ~Scatter() = default;

	private:
		std::vector<double> x, y;
	};

	Scatter::Scatter(const std::vector<double>& in_x,
		const std::vector<double>& in_y, unsigned int in_size,
		COLORREF in_color, std::vector<double>& range)
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

		// Set x and y range for Window member range
		range[0] = range[0] < min_x ? range[0] : min_x;
		range[1] = range[1] > max_x ? range[1] : max_x;
		range[2] = range[2] < min_y ? range[2] : min_y;
		range[3] = range[3] > max_y ? range[3] : max_y;
	}

	void Scatter::show(HDC hdc, HWND hwnd, RECT rect,
		const std::vector<double>& range) const
	{
		// Set adjusted min and max values, adjusted for the 
		// free space before/after the first/last point
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
				(unsigned int)round((x[i] - adj_min_x) * win_length_x / length_x);
			y_coord = rect.bottom -
				(unsigned int)round((y[i] - adj_min_y) * win_length_y / length_y);

			Ellipse(hdc, x_coord - 1, y_coord - 1, x_coord + 1, y_coord + 1);
		}

		// Clean graphic objects
		DeleteObject(hGraphPen);
		DeleteObject(hGraphBrush);

		// Set previous graphic properties
		SelectObject(hdc, hGraphPreviousPen);
		SelectObject(hdc, hGraphPreviousBrush);
	}

	// Derived class for line plots
	class Line : public Graph
	{
	public:
		Line(const std::vector<double>& in_x,
			const std::vector<double>& in_y, unsigned int in_size,
			COLORREF in_color, std::vector<double>& range);

		virtual void show(HDC hdc, HWND hwnd, RECT rect,
			const std::vector<double>& range) const;

		virtual ~Line() = default;

	private:
		std::map<double, double> data;
	};

	Line::Line(const std::vector<double>& in_x,
		const std::vector<double>& in_y, unsigned int in_size,
		COLORREF in_color, std::vector<double>& range)
	{
		for (unsigned int i = 0; i != in_x.size(); ++i)
		{
			data[in_x[i]] = in_y[i]; // TODO : OPTIMIZE
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

		// Set x and y range for Window member range
		range[0] = range[0] < min_x ? range[0] : min_x;
		range[1] = range[1] > max_x ? range[1] : max_x;
		range[2] = range[2] < min_y ? range[2] : min_y;
		range[3] = range[3] > max_y ? range[3] : max_y;
	}

	void Line::show(HDC hdc, HWND hwnd, RECT rect,
		const std::vector<double>& range) const
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

		// Set appropriate graph properties and draw the lines
		HPEN hGraphPen = CreatePen(PS_SOLID, size, color);
		HPEN hGraphPreviousPen = (HPEN)SelectObject(hdc, hGraphPen);
		for (std::map<double, double>::const_iterator it = data.begin();
			it != data.end(); ++it)
		{
			x_coord = rect.left +
				(unsigned int)round((it->first - adj_min_x) * win_length_x / length_x);
			y_coord = rect.bottom -
				(unsigned int)round((it->second - adj_min_y) * win_length_y / length_y);

			LineTo(hdc, x_coord, y_coord);
		}

		// Delete graphics objects
		DeleteObject(hGraphPen);

		// Set previous graphic properties
		SelectObject(hdc, hGraphPreviousPen);
	}

	class Histogram : public Graph
	{
	public:
		Histogram(const std::vector<double>& in_x,
			const std::vector<double>& bins, unsigned int in_size,
			COLORREF in_color, bool normed, std::vector<double>& range);

		Histogram(const std::vector<double>& in_x, int bins, 
			unsigned int in_size, COLORREF in_color, bool normed, 
			std::vector<double>& range);

		virtual void initialize(const std::vector<double>& in_x,
			const std::vector<double>& bins, unsigned int in_size,
			COLORREF in_color, bool normed, std::vector<double>& range);

		virtual void show(HDC hdc, HWND hwnd, RECT rect,
			const std::vector<double>& range) const;

		virtual ~Histogram() = default;

	private:
		std::vector<double> x, y, bin_pos;
	};

	Histogram::Histogram(const std::vector<double>& in_x,
		const std::vector<double>& bins, unsigned int in_size,
		COLORREF in_color, bool normed, std::vector<double>& range)
	{
		this->initialize(in_x, bins, in_size, in_color, normed, range);
	}

	Histogram::Histogram(const std::vector<double>& in_x,
		int bins, unsigned int in_size,	COLORREF in_color, 
		bool normed, std::vector<double>& range)
	{
		// Find the max of x
		double min_x = *std::min_element(in_x.begin(), in_x.end(),
			[](const double& a, const double& b)
		{ return a < b;  });
		double max_x = *std::max_element(in_x.begin(), in_x.end(),
			[](const double& a, const double& b)
		{ return a < b;  });		

		// Fill the positions of bins
		const double offset = (max_x - min_x) / bins; 
		bin_pos = std::vector<double>(bins + 1);
		for (int i = 0; i != bins + 1; i++)
		{
			bin_pos[i] = min_x + i * offset;
		}

		this->initialize(in_x, bin_pos, in_size, in_color, normed, range);
	}

	void Histogram::initialize(const std::vector<double>& in_x,
		const std::vector<double>& bins, unsigned int in_size,
		COLORREF in_color, bool normed, std::vector<double>& range)
	{
		x = in_x;
		color = in_color;
		size = in_size;
		if (bin_pos.empty()) { bin_pos = bins; }
		y = std::vector<double>(bin_pos.size() - 1, 0.0);

		// Sort x and find min and max of x
		std::sort(x.begin(), x.end());
		double min_x = x.front();
		double max_x = x.back();

		// Find the count of x values in the bucket
		std::vector<double>::iterator y_iter = y.begin();
		std::vector<double>::const_iterator bin_pos_iter = ++bin_pos.begin();
		std::vector<double>::const_iterator x_iter = x.begin();
		for (; bin_pos_iter != bin_pos.end(); bin_pos_iter++)
		{
			while (*x_iter < (*bin_pos_iter + FP_ERROR))
			{
				*y_iter += 1.0;
				++x_iter;

				if (x_iter == x.end()) { break; }
			}

			++y_iter;
		}

		// TODO : Normalize
		if (normed)
		{

		}

		// Find the offset from the starting x
		double max_y = *std::max_element(y.begin(), y.end(),
			[](const double& a, const double& b)
		{ return a < b;  });
		double min_y = *std::min_element(y.begin(), y.end(),
			[](const double& a, const double& b)
		{ return a < b;  });

		// Set x and y range for Window member range
		range[0] = range[0] < min_x ? range[0] : min_x;
		range[1] = range[1] > max_x ? range[1] : max_x;
		range[2] = range[2] < 0.0 ? range[2] : 0.0;
		range[3] = range[3] > max_y ? range[3] : max_y;
	}

	void Histogram::show(HDC hdc, HWND hwnd, RECT rect,
		const std::vector<double>& range) const
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

		// Set appropriate graph properties
		HPEN hGraphPen = CreatePen(PS_SOLID, size, BLACK);
		HBRUSH hGraphBrush = CreateSolidBrush(color);
		HPEN hGraphPreviousPen = (HPEN)SelectObject(hdc, hGraphPen);
		HBRUSH hGraphPreviousBrush = (HBRUSH)SelectObject(hdc, hGraphBrush);

		// Render the histogram rectangles
		RECT bin_rect;
		bin_rect.bottom = rect.bottom -
			(unsigned int)round((0.0 - adj_min_y) * win_length_y / length_y);
		for (int i = 0; i != bin_pos.size() - 1; i++)
		{
			bin_rect.left = rect.left +
				(unsigned int)round((bin_pos[i] - adj_min_x) * win_length_x / length_x);
			bin_rect.right = rect.left +
				(unsigned int)round((bin_pos[i + 1] - adj_min_x) * win_length_x / length_x);
			bin_rect.top = rect.bottom -
				(unsigned int)round((y[i] - adj_min_y) * win_length_y / length_y);
			Rectangle(hdc, bin_rect.left, bin_rect.top, bin_rect.right, bin_rect.bottom);
		}

		// Delete graphics objects
		DeleteObject(hGraphPen);
		DeleteObject(hGraphBrush);

		// Set previous graphic properties
		SelectObject(hdc, hGraphPreviousPen);
		SelectObject(hdc, hGraphPreviousBrush);
	}
	
	class Axis
	{
	public:
		Axis() : legend_state{ false } {};

		void show_ticks(HDC hdc, HWND hwnd, RECT x_rect, RECT y_rect,
			std::vector<double> range, HFONT font); // range by value!

		void show_xticks(HDC hdc, HWND hwnd, RECT x_rect, RECT y_rect,
			std::vector<double> range, TEXTMETRIC textMetric);

		void show_yticks(HDC hdc, HWND hwnd, RECT x_rect, RECT y_rect,
			std::vector<double> range, TEXTMETRIC textMetric);

		void tick_value_map(double diff, double& y_tick_period_adj,
			double& y_value_period_adj);

		void show_title(HDC hdc, HWND hwnd, RECT rect, HFONT font) const;

		void show_xlabel(HDC hdc, HWND hwnd, RECT rect, HFONT font) const;

		void show_ylabel(HDC hdc, HWND hwnd, RECT rect, HFONT font) const;

		void show_legend(HDC hdc, HWND hwnd, RECT rect, HFONT font,
			unsigned int text_width, unsigned int text_height) const;

		double tick_xoffset(unsigned int text_height) const;

		double label_xoffset(unsigned int text_height) const;

		double tick_yoffset(unsigned int text_height) const;

		double label_yoffset(unsigned int text_height) const;

		double title_offset(unsigned int text_height) const;

		double legend_offset(HDC hdc, unsigned int text_width, unsigned int base) const;

		void set_xlabel(std::string xlab) { xlabel = xlab; }

		void set_ylabel(std::string ylab) { ylabel = ylab; }

		void set_title(std::string in_title) { title = in_title; }

		void set_legend(std::string name, std::string type, COLORREF color,
			unsigned int size);

		void activate_legend() { legend_state = true; }

		bool is_xlabel_activated() const { return !xlabel.empty(); }

		bool is_ylabel_activated() const { return !ylabel.empty(); }

		bool is_title_activated() const { return !title.empty(); }

		bool is_legend_activated() const { return legend_state; }

	private:
		struct LEGEND
		{
			LEGEND() = default;

			LEGEND(std::string in_name, std::string in_type, COLORREF in_color,
				unsigned int in_size) : name{ in_name }, type{ in_type },
				color{ in_color }, size{ in_size } {};

			std::string name;
			std::string type;
			COLORREF color;
			unsigned int size;
		};

		std::string xlabel;
		std::string ylabel;
		std::string title;
		std::vector<LEGEND> legend;

		bool legend_state;
	};

	void Axis::tick_value_map(double diff, double& y_tick_period_adj, 
		double& y_value_period_adj)
	{
		// nothing, multication or division group
		unsigned int y_op_sign = 0;
		double y_factor = 0.0;

		// Find the multiplicative/divisive factor
		if ((diff + FP_ERROR) < 10.0)
		{
			y_op_sign = 1;

			y_factor += 1.0;
			diff *= 10.0;

			while ((diff + FP_ERROR) < 10.0)
			{
				y_factor += 1.0;
				diff *= 10.0;
			}
		}
		else if ((diff - FP_ERROR) >= 100.0)
		{
			y_op_sign = 2;

			y_factor += 1.0;
			diff /= 10.0;

			while ((diff - FP_ERROR) >= 100.0)
			{
				y_factor += 1.0;
				diff /= 10.0;
			}
		}

		// Find appropriate tick and value dispersion 
		double y_value_period = 2.0;
		double y_tick_period = 1.0;
		if (diff >= 20.0 && diff < 40.0)
		{
			y_value_period = 5.0;
			y_tick_period = 2.5;
		}
		else if (diff >= 40.0)
		{
			y_value_period = 10.0;
			y_tick_period = 5.0;
		}

		// Set the graph-specific tick and value dispersion
		y_value_period_adj = y_value_period;
		y_tick_period_adj = y_tick_period;
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
	}

	void Axis::show_xticks(HDC hdc, HWND hwnd, RECT x_rect, RECT y_rect,
		std::vector<double> range, TEXTMETRIC textMetric)
	{
		/* *********************************
		// X axis ticks and values rendering
		********************************* */

		// Set length of axis ticks
		unsigned int x_stick = (unsigned int)(((x_rect.bottom - x_rect.top) -
			2.0 * textMetric.tmHeight) / 3.0);

		double x_factor = 0.0;
		double x_diff = range[1] - range[0];
		unsigned int x_op_sign = 0; // nothing, mult or div

		// Obtain the appropriate steps for ticks and values
		double x_value_period, x_tick_period;
		this->tick_value_map(x_diff, x_tick_period, x_value_period);

		// Find starting values for the first tick and value
		int x_value_divisor = (int)ceil(range[0] / x_value_period);
		int x_tick_divisor = (int)ceil(range[0] / x_tick_period);

		double x_value = x_value_divisor * x_value_period;
		double x_tick = x_tick_divisor * x_tick_period;

		// Pre-compute variables
		double win_length_x = x_rect.right - x_rect.left;
		double length_x = range[1] - range[0];

		unsigned int x_coord, y_coord = x_rect.top;

		// Render ticks
		while (x_tick < range[1])
		{
			x_coord = x_rect.left +
				(unsigned int)round((x_tick - range[0]) * win_length_x / length_x);

			MoveToEx(hdc, x_coord, y_coord, NULL);
			LineTo(hdc, x_coord, y_coord + x_stick);

			x_tick += x_tick_period;
		}

		// Render values
		y_coord += TICK_TEXT_FACTOR * x_stick;
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

				x_value += x_value_period;
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

				x_value += x_value_period;
			}
		}

	}


	void Axis::show_yticks(HDC hdc, HWND hwnd, RECT x_rect, RECT y_rect,
		std::vector<double> range, TEXTMETRIC textMetric)
	{
		static unsigned int call_counter = 0;

		/* *********************************
		// Y axis ticks and values rendering
		********************************* */

		unsigned int y_stick = (unsigned int)(((y_rect.right - y_rect.left) -
			2.0 * textMetric.tmHeight) / 3.0);

		double y_factor = 0.0;
		double y_diff = range[3] - range[2];

		// Automatic scientific notation
		unsigned int y_value_digits = 0; // overall number of digits
		unsigned int y_value_int_digits = 0; // number of integral digits
		unsigned int exp_value_capacity = 0; // how many values are expected
		bool scientific = 0; // whether values will be rendered in the scientific notation

		// First check whether the difference is not too small, so that we 
		// should extract a certain value and keep only the rest in the rendering
		// of the axis -> e.g. when range is from 100.000001 to 100.000002, then
		// plot 0.000001 and 0.000002 in the scientific notation and add 100 to the label
		if (y_diff <= pow(10.0, -MIN_RANGE_DIFF))
		{
			scientific = 1;

			// Add-on that is going to be extracted and added to the label
			// Ranges need to be internally changed -> that is the reason why
			// range vector needs to be passed by value!
			int add_on = (int)range[2];
			range[2] = range[2] - add_on;
			range[3] = range[3] - add_on;
			if (call_counter++ == 0)
			{
				ylabel += " [ " + std::to_string(add_on) + "+ ]";
			}

			// 6 is a constant because basic notation is: 
			// e.g. (+/-)1.(SCIENTIFIC_FRAC_DIGITS)e(+/-)10
			y_value_digits = 6 + SCIENTIFIC_FRAC_DIGITS;

			// In case of negative number, also increase number of digits
			if (range[2] < 0)
			{
				++y_value_digits;
			}
		}
		else if (abs(range[3]) > MAX_RANGE_VALUE)
		{
			scientific = 1;

			// 6 is a constant because basic notation is: 
			// e.g. (+/-)1.(SCIENTIFIC_FRAC_DIGITS)e(+/-)10
			y_value_digits = 6 + SCIENTIFIC_FRAC_DIGITS;

			// In case of negative number, also increase number of digits
			if (range[2] < 0)
			{
				++y_value_digits;
			}
		}

		// Obtain the appropriate steps for ticks and values
		double y_value_period, y_tick_period;
		this->tick_value_map(y_diff, y_tick_period, y_value_period);

		// Find the starting values for the first tick and value
		int y_value_divisor = (int)ceil(range[2] / y_value_period);
		int y_tick_divisor = (int)ceil(range[2] / y_tick_period);

		double y_value = y_value_divisor * y_value_period;
		double y_tick = y_tick_divisor * y_tick_period;

		// Pre-compute variables
		double win_length_y = y_rect.bottom - y_rect.top;
		double length_y = range[3] - range[2];
		unsigned int x_coord = y_rect.right;
		unsigned int y_coord;

		// Render ticks
		while (y_tick < range[3])
		{
			y_coord = y_rect.bottom -
				(unsigned int)round((y_tick - range[2]) * win_length_y / length_y);

			MoveToEx(hdc, x_coord, y_coord, NULL);
			LineTo(hdc, x_coord - y_stick, y_coord);

			y_tick += y_tick_period;
		}

		x_coord -= (unsigned int)(TICK_TEXT_FACTOR * y_stick);
		std::wstring y_text;


		// If the values do not classify as scientific, find the number of integral
		// and fractional digits -> e.g. 100.01 = 3 integral and 2 fractional
		if (!scientific)
		{
			double y_value_c = y_value;
			unsigned int current_digits = 0, current_int_digits = 0;
			while (y_value_c <= range[3])
			{
				current_int_digits = get_int_digits(y_value_c);
				current_digits = current_int_digits +
					get_frac_digits(y_value_c, MIN_RANGE_DIFF + 1);

				if (y_value_digits < current_digits)
				{
					y_value_digits = current_digits;
					y_value_int_digits = current_int_digits;
				}

				y_value_c += y_value_period;
				++exp_value_capacity;
			}
		}
		else
		{
			double y_value_c = y_value;
			while (y_value_c <= range[3])
			{
				y_value_c += y_value_period;
				++exp_value_capacity;
			}
		}

		// Find correct text length and max capacity of the rectangle
		unsigned int av_value_length = AXIS_VALUE_SPACE +
			y_value_digits * textMetric.tmAveCharWidth;
		unsigned int max_value_capacity = (y_rect.bottom - y_rect.top) / av_value_length;

		// If expected space is higher than maximal space, halve the expectations 
		double halving = 2.0;
		while (exp_value_capacity > max_value_capacity)
		{
			if (exp_value_capacity <= 2)
			{
				exp_value_capacity = 1;
				break;
			}

			exp_value_capacity = (unsigned int)ceil(exp_value_capacity / halving);
			y_value_period *= halving;
		}

		// Make proper format for the text rendering via snprintf
		// char *format = "% int . frac f" or scientific notation
		char *sbuffer = new char[y_value_digits + 1];
		wchar_t *wbuffer = new wchar_t[y_value_digits + 1];

		char format[6];
		if (!scientific)
		{
			unsigned int y_value_frac_digits = y_value_digits - y_value_int_digits;
			if (y_value_frac_digits)
			{
				// minus one for the dot in case the number has fractional digits
				--y_value_frac_digits;
			}

			format[0] = '%';
			snprintf(&format[1], 2, "%u", y_value_int_digits);
			format[2] = '.';
			snprintf(&format[3], 2, "%u", y_value_frac_digits);
			format[4] = 'f';
			format[5] = '\0';
		}
		else
		{
			format[0] = '%';
			snprintf(&format[1], 2, "%u", 1);
			format[2] = '.';
			snprintf(&format[3], 2, "%u", SCIENTIFIC_FRAC_DIGITS);
			format[4] = 'e';
			format[5] = '\0';
		}

		// Render the text
		while (y_value < range[3])
		{
			y_coord = y_rect.bottom -
				(unsigned int)round((y_value - range[2]) * win_length_y / length_y);

			// + 1 in y_value_digits for null-termination
			snprintf(sbuffer, y_value_digits + 1, format, y_value);

			MultiByteToWideChar(CP_UTF8, 0, sbuffer, -1, wbuffer, y_value_digits + 1);
			TextOut(hdc, x_coord, y_coord, wbuffer, y_value_digits);

			y_value += y_value_period;
		}

		// Clean allocated buffers 
		delete[] sbuffer;
		delete[] wbuffer;
	}

	void Axis::show_ticks(HDC hdc, HWND hwnd, RECT x_rect, RECT y_rect,
		std::vector<double> range, HFONT font)
	{
		// Set graphics attributes
		HPEN hBoxPen = CreatePen(PS_SOLID, 1, BLACK);
		SelectObject(hdc, hBoxPen);

		// Set proper text alignment
		unsigned int prev_text_align = SetTextAlign(hdc, TA_CENTER | TA_TOP);

		// Get parameters of current font -> for proper rendering of axis values
		TEXTMETRIC textMetric;
		GetTextMetrics(hdc, &textMetric);

		// Render ticks on the x axis
		this->show_xticks(hdc, hwnd, x_rect, y_rect, range, textMetric);

		// Set bottom and center text alignment
		SetTextAlign(hdc, TA_CENTER | TA_BOTTOM);

		// Set font rotation by 90 degrees
		LOGFONT lf;
		GetObject(font, sizeof(LOGFONT), &lf);
		lf.lfEscapement = 900;
		HFONT lfont = CreateFontIndirect(&lf);
		HFONT h_prev_font = (HFONT)SelectObject(hdc, (HGDIOBJ)(HFONT)(lfont));

		// Render ticks on the y axis
		this->show_yticks(hdc, hwnd, x_rect, y_rect, range, textMetric);

		// Set text alignment and font that was in place before rendering axis attributes
		SetTextAlign(hdc, prev_text_align);
		SelectObject(hdc, h_prev_font);

		DeleteObject(hBoxPen);
	}

	void Axis::show_xlabel(HDC hdc, HWND hwnd, RECT rect, HFONT font) const
	{
		// Set proper text alignment
		unsigned int prev_text_align = SetTextAlign(hdc, TA_CENTER | TA_BOTTOM);

		// Set pen attribute
		HPEN hBoxPen = CreatePen(PS_SOLID, 1, BLACK);
		SelectObject(hdc, hBoxPen);

		// Write the label of x axis
		unsigned int x_coord_xlabel = (unsigned int)((rect.right + rect.left) * 0.5);
		unsigned int y_coord_xlabel = rect.bottom;
		wchar_t * wide_xlabel = new wchar_t[xlabel.size() + 1];
		MultiByteToWideChar(CP_UTF8, 0, xlabel.c_str(), -1, wide_xlabel, xlabel.size() + 1);
		TextOut(hdc, x_coord_xlabel, y_coord_xlabel, wide_xlabel, xlabel.size());

		delete[] wide_xlabel;

		// Set previous graphic properties
		SetTextAlign(hdc, prev_text_align);

		DeleteObject(hBoxPen);
	}

	void Axis::show_ylabel(HDC hdc, HWND hwnd, RECT rect, HFONT font) const
	{
		// Set proper text alignment
		unsigned int prev_text_align = SetTextAlign(hdc, TA_CENTER | TA_TOP);

		// Set font rotated by 90 degrees
		LOGFONT lf;
		GetObject(font, sizeof(LOGFONT), &lf);
		lf.lfEscapement = 900;
		HFONT lfont = CreateFontIndirect(&lf);
		HFONT h_prev_font = (HFONT)SelectObject(hdc, (HGDIOBJ)(HFONT)(lfont));

		// Set pen attribute
		HPEN hBoxPen = CreatePen(PS_SOLID, 1, BLACK);
		HPEN hPreviousPen = (HPEN)SelectObject(hdc, hBoxPen);

		// Write the label of y axis
		unsigned int x_coord_ylabel = rect.left;
		unsigned int y_coord_ylabel = (unsigned int)((rect.top + rect.bottom) * 0.5);
		wchar_t * wide_ylabel = new wchar_t[ylabel.size() + 1];
		MultiByteToWideChar(CP_UTF8, 0, ylabel.c_str(), -1, wide_ylabel, ylabel.size() + 1);
		TextOut(hdc, x_coord_ylabel, y_coord_ylabel, wide_ylabel, ylabel.size());

		delete[] wide_ylabel;

		// Set previous graphic properties
		SetTextAlign(hdc, prev_text_align);
		SelectObject(hdc, h_prev_font);
		SelectObject(hdc, hPreviousPen);

		// Clean graphic objects
		DeleteObject(hBoxPen);
	}

	void Axis::show_title(HDC hdc, HWND hwnd, RECT rect, HFONT font) const
	{
		// Set proper text alignment
		unsigned int prev_text_align = SetTextAlign(hdc, TA_CENTER | TA_TOP);

		// Make the font bold
		LOGFONT lf;
		GetObject(font, sizeof(LOGFONT), &lf);
		lf.lfWeight = FW_BOLD;
		HFONT lfont = CreateFontIndirect(&lf);
		HFONT h_prev_font = (HFONT)SelectObject(hdc, (HGDIOBJ)(HFONT)(lfont));

		// Set pen attribute
		HPEN hBoxPen = CreatePen(PS_SOLID, 1, BLACK);
		HPEN hPreviousPen = (HPEN)SelectObject(hdc, hBoxPen);

		// Write the title
		unsigned int x_coord_title = (unsigned int)((rect.right + rect.left) / 2);
		unsigned int y_coord_title = rect.top;
		wchar_t * wide_title = new wchar_t[title.size() + 1];
		MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, wide_title, ylabel.size() + 1);
		TextOut(hdc, x_coord_title, y_coord_title, wide_title, title.size());

		delete[] wide_title;

		// Set previous graphic properties
		SetTextAlign(hdc, prev_text_align);
		SelectObject(hdc, h_prev_font);
		SelectObject(hdc, hPreviousPen);

		// Clean graphic objects
		DeleteObject(hBoxPen);
	}

	inline void Axis::show_legend(HDC hdc, HWND hwnd, RECT rect, HFONT font,
		unsigned int text_width, unsigned int text_height) const
	{
		// Set proper text alignment
		unsigned int prev_text_align = SetTextAlign(hdc, TA_LEFT | TA_TOP);

		// Get current graphic properties
		HPEN hGraphPreviousPen = (HPEN)GetCurrentObject(hdc, OBJ_PEN);
		HBRUSH hGraphPreviousBrush = (HBRUSH)GetCurrentObject(hdc, OBJ_BRUSH);
		HPEN hGraphPen;
		HBRUSH hGraphBrush;

		// Position of points/centre of lines for symbols of the legend
		unsigned int point_pos_x, point_pos_y;

		// Length of the space for writing the text of the legend
		unsigned int legend_text_width = rect.right - rect.left -
			LEGEND_SYMBOL_LENGTH;

		// Maximum amount of symbols that can be written in the legend space
		unsigned int max_symbols = legend_text_width / text_width;

		// Buffer to hold current output text
		wchar_t *buffer = new wchar_t[max_symbols + 1]; // one for null for wcsncpy_s

		// Length of unwritten string, length of whole string and beginning
		// and end of the current state of writing
		unsigned int diff, string_size, begin, end;

		// Offset of the y axis from the top of the rectangle
		unsigned int current_offset = LEGEND_SYMBOL_LENGTH;

		for (std::vector<LEGEND>::const_iterator it = legend.begin();
			it != legend.end(); ++it)
		{
			// Set proper graphics attributes
			hGraphPen = CreatePen(PS_SOLID, it->size, it->color);
			hGraphBrush = CreateSolidBrush(it->color);
			(HPEN)SelectObject(hdc, hGraphPen);
			(HBRUSH)SelectObject(hdc, hGraphBrush);

			// Paint the point/line in the left column of the legend
			point_pos_x = (unsigned int)((rect.left + LEGEND_SYMBOL_LENGTH * 0.5));
			point_pos_y = (unsigned int)(rect.top + text_height * 0.5
				+ current_offset);
			if (it->type == "scatter")
			{
				Ellipse(hdc, point_pos_x - 1, point_pos_y - 1, point_pos_x + 1,
					point_pos_y + 1);
			}
			else if (it->type == "line")
			{
				unsigned int begin_x = (unsigned int)((rect.left +
					LEGEND_SYMBOL_LENGTH * 0.25));
				unsigned int end_x = (unsigned int)((rect.left +
					LEGEND_SYMBOL_LENGTH * 0.75));
				MoveToEx(hdc, begin_x, point_pos_y, NULL);
				LineTo(hdc, end_x, point_pos_y);
			}
			else if (it->type == "hist")
			{
				hGraphPen = CreatePen(PS_SOLID, it->size, BLACK);
				(HPEN)SelectObject(hdc, hGraphPen);

				RECT hist_rect;
				hist_rect.top = (unsigned int)(rect.top + current_offset);
				hist_rect.bottom = (unsigned int)(rect.top + text_height + 
					current_offset);
				hist_rect.left = (unsigned int)((rect.left +
					LEGEND_SYMBOL_LENGTH * 0.25));
				hist_rect.right = (unsigned int)((rect.left +
					LEGEND_SYMBOL_LENGTH * 0.75));
				Rectangle(hdc, hist_rect.left, hist_rect.top, hist_rect.right, hist_rect.bottom);
			}
			else
			{
				printf("Warning: Unsupported graph type. No legend symbol for this"
					"plot written");
			}

			// Show the text associated with a given point/line
			string_size = (it->name).size();
			diff = string_size;
			begin = 0;

			while (true)
			{
				// If the rest of the string is smaller than the possible output length
				if (diff <= max_symbols)
				{
					end = string_size;

					MultiByteToWideChar(CP_UTF8, 0, &((it->name).c_str())[begin],
						end - begin, buffer, diff);
					TextOut(hdc, rect.left + LEGEND_SYMBOL_LENGTH, rect.top
						+ current_offset, buffer, end - begin);
					break;
				}
				else
				{
					end = begin + max_symbols - 1;

					MultiByteToWideChar(CP_UTF8, 0, &(it->name).c_str()[begin],
						end - begin, buffer, max_symbols - 1);
					wcsncpy_s(&buffer[max_symbols - 1], 2, L"-", 1);
					TextOut(hdc, rect.left + LEGEND_SYMBOL_LENGTH, rect.top +
						current_offset, buffer, max_symbols);

					begin = end;
					diff = string_size - begin;

					current_offset += text_height + LEGEND_TEXT_ROW_SPACE;
				}

				// Break in case the text is too long
				if (current_offset > (unsigned int)rect.bottom)
				{
					break;
				}
			}

			current_offset += text_height + LEGEND_TEXT_POINT_SPACE;
		}

		// Set the previous graphics properties
		SelectObject(hdc, hGraphPreviousPen);
		SelectObject(hdc, hGraphPreviousBrush);
		SetTextAlign(hdc, prev_text_align);

		// Clean graphics objects
		DeleteObject(hGraphPen);
		DeleteObject(hGraphBrush);

		delete[] buffer;
	}

	double Axis::legend_offset(HDC hdc, unsigned int text_width, unsigned int base) const
	{
		// width of the rectangle available for text of the legend
		unsigned int base_width = base - LEGEND_SYMBOL_LENGTH;

		// finding whether any string representing label names is longer
		// than available width
		unsigned int max_width = 0;
		unsigned int string_width = 0;
		for (std::vector<LEGEND>::const_iterator
			it = legend.begin(); it != legend.end(); ++it)
		{
			string_width = (it->name).size() * text_width;
			if (string_width >= base_width)
			{
				return base_width;
			};

			max_width = (string_width > max_width) ? string_width : max_width;
		}

		return max_width + LEGEND_SYMBOL_LENGTH;
	}

	double Axis::tick_xoffset(unsigned int text_height) const
	{
		return text_height * X_TICK_RATIO;
	}

	double Axis::label_xoffset(unsigned int text_height) const
	{
		if (xlabel.empty()) { return 0.0; }

		return text_height;
	}

	double Axis::tick_yoffset(unsigned int text_height) const
	{
		return text_height * Y_TICK_RATIO;
	}

	double Axis::label_yoffset(unsigned int text_height) const
	{
		if (ylabel.empty()) { return 0.0; }

		return text_height;
	}

	double Axis::title_offset(unsigned int text_height) const
	{
		if (title.empty()) { return 0.0; }

		return text_height * TITLE_RATIO;
	}

	void Axis::set_legend(std::string name, std::string type, COLORREF color,
		unsigned int size)
	{
		legend.emplace_back(name, type, color, size);
	}
}
