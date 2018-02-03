#pragma once
#include "Header.h"
#include "Constants.h"
#include "Render.h"

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
		unsigned int int_part = (unsigned int)(abs(x) + FP_ERROR);

		while (int_part >= 1)
		{
			int_part /= 10;
			++no_digits;
		}

		// In case x is negative, add also - sign
		if (x < 0.0)
		{
			++no_digits;
		}

		// In case x is below +/-one, add one for the beginning zero 
		if (abs(x) + FP_ERROR < 1.0)
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
		Graph(COLORREF in_color, unsigned int in_size) :
			color(in_color), size(in_size), render_pointer(nullptr) {};

		Graph(COLORREF in_color, unsigned int in_size, RenderObjects *render_ptr) : 
		color(in_color), size(in_size), render_pointer(render_ptr) {};

		virtual void show(HDC hdc, HWND hwnd, RECT rect,
			const std::vector<double>& range) const = 0;

		virtual ~Graph() = default;

	protected:
		COLORREF color;
		unsigned int size;
		RenderObjects *render_pointer;
	};

	// Derived class for scatterplots
	class Scatter : public Graph
	{
	public:
		Scatter(const std::vector<double>& in_x,
			const std::vector<double>& in_y, unsigned int in_size,
			COLORREF in_color, std::vector<double>& range,
			RenderObjects *render_ptr);

		virtual void show(HDC hdc, HWND hwnd, RECT rect,
			const std::vector<double>& range) const;

		virtual ~Scatter() = default;

	private:
		std::vector<double> x, y;
	};

	Scatter::Scatter(const std::vector<double>& in_x,
		const std::vector<double>& in_y, unsigned int in_size,
		COLORREF in_color, std::vector<double>& range,
		RenderObjects *render_ptr) : x{ in_x }, y{ in_y }, 
		Graph(in_color, in_size, render_ptr)
	{
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
		// Check for nullptr
		if (!render_pointer)
		{
			RenderObjects *local_render_ptr = &RenderScatterPoints();
			local_render_ptr->renderPoints(hdc, x, y, rect, range, color, size);
		}
		else
		{
			render_pointer->renderPoints(hdc, x, y, rect, range, color, size);
		}
	}

	// Derived class for line plots
	class Line : public Graph
	{
	public:
		Line(const std::vector<double>& in_x,
			const std::vector<double>& in_y, unsigned int in_size,
			COLORREF in_color, std::vector<double>& range,
			RenderObjects *render_ptr);

		virtual void show(HDC hdc, HWND hwnd, RECT rect,
			const std::vector<double>& range) const;

		virtual ~Line() = default;

	private:
		std::map<double, double> data;
	};

	Line::Line(const std::vector<double>& in_x,
		const std::vector<double>& in_y, unsigned int in_size,
		COLORREF in_color, std::vector<double>& range,
		RenderObjects *render_ptr) : Graph(in_color, in_size, render_ptr)
	{
		for (unsigned int i = 0; i != in_x.size(); ++i)
		{
			data[in_x[i]] = in_y[i]; // TODO : OPTIMIZE
		}

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
		// Check for nullptr
		if (!render_pointer)
		{
			RenderObjects *local_render_ptr = &RenderLinesFull();
			local_render_ptr->renderLines(hdc, data, rect, range, color, size);
		}
		else
		{
			render_pointer->renderLines(hdc, data, rect, range, color, size);
		}
	}

	class Histogram : public Graph
	{
	public:
		Histogram(const std::vector<double>& in_x, const std::vector<double>& bins,
			unsigned int in_size,COLORREF in_color, bool normed, std::vector<double>& range);

		Histogram(const std::vector<double>& in_x, int bins, const std::vector<double>&
			max_min_range, unsigned int in_size, COLORREF in_color, bool normed,
			std::vector<double>& range);

		virtual void initialize(const std::vector<double>& in_x,
			const std::vector<double>& bins, unsigned int in_size, COLORREF in_color, 
			bool normed, std::vector<double>& range);

		virtual void show(HDC hdc, HWND hwnd, RECT rect,
			const std::vector<double>& range) const;

		virtual ~Histogram() = default;

	private:
		std::vector<double> x, y, bin_pos;
	};

	Histogram::Histogram(const std::vector<double>& in_x,
		const std::vector<double>& bins, unsigned int in_size, COLORREF in_color, 
		bool normed, std::vector<double>& range) : x(in_x), y(bins),
		bin_pos(bins), Graph(in_color, in_size)
	{
		this->initialize(in_x, bins, in_size, in_color, normed, range);

		double max_y = *std::max_element(y.begin(), y.end(),
			[](const double& a, const double& b)
		{ return a < b;  });

		// Set x and y range for Window member range
		range[0] = range[0] < x.front() ? range[0] : x.front();
		range[1] = range[1] > x.back() ? range[1] : x.back();
		range[2] = range[2] < 0.0 ? range[2] : 0.0;
		range[3] = range[3] > max_y ? range[3] : max_y;
	}

	Histogram::Histogram(const std::vector<double>& in_x,
		int bins, const std::vector<double>& max_min_range, 
		unsigned int in_size, COLORREF in_color,
		bool normed, std::vector<double>& range) : 
		x(in_x), y(bins, 0.0), Graph(in_color, in_size)
	{
		// Find the max of x
		double min_x, max_x;
		if (max_min_range.empty())
		{
			min_x = *std::min_element(in_x.begin(), in_x.end(),
				[](const double& a, const double& b)
			{ return a < b;  });
			max_x = *std::max_element(in_x.begin(), in_x.end(),
				[](const double& a, const double& b)
			{ return a < b;  });
		}
		else
		{
			min_x = max_min_range[0];
			max_x = max_min_range[1];
		}

		// Fill the positions of bins
		const double offset = (max_x - min_x) / bins; 
		bin_pos = std::vector<double>(bins + 1);
		for (int i = 0; i != bins + 1; i++)
		{
			bin_pos[i] = min_x + i * offset;
		}

		this->initialize(in_x, bin_pos, in_size, in_color, normed, range);

		double max_y = *std::max_element(y.begin(), y.end(),
			[](const double& a, const double& b)
		{ return a < b;  });

		range[0] = range[0] < min_x ? range[0] : min_x;
		range[1] = range[1] > max_x ? range[1] : max_x;
		range[2] = range[2] < 0.0 ? range[2] : 0.0;
		range[3] = range[3] > max_y ? range[3] : max_y;
	}

	void Histogram::initialize(const std::vector<double>& in_x,
		const std::vector<double>& bins, unsigned int in_size, 
		COLORREF in_color, bool normed, std::vector<double>& range)
	{
		std::sort(x.begin(), x.end());

		// Find the count of x values in the bucket
		std::vector<double>::iterator y_iter = y.begin();
		std::vector<double>::const_iterator bin_pos_iter = ++bin_pos.begin();
		std::vector<double>::const_iterator x_iter = x.begin();

		// Find the starting values for x
		while(*x_iter++ < (*bin_pos_iter - FP_ERROR)) { }

		// Fill the bins
		double sum_y = 0.0;
		for (; bin_pos_iter != bin_pos.end(); bin_pos_iter++)
		{
			while (*x_iter < (*bin_pos_iter - FP_ERROR))
			{
				++x_iter;
				*y_iter += 1.0;
				sum_y += 1.0;

				if (x_iter == x.end()) { break; }
			}

			++y_iter;
		}

		// In case there are still values in the x equal to the end of the last bin
		--y_iter;
		--bin_pos_iter;
		while (x_iter != x.end() && *x_iter < (*bin_pos_iter + FP_ERROR))
		{
			*y_iter += 1.0;
			sum_y += 1.0;
			++x_iter;
		}

		// Normalize by dividing by number of observations times bin width
		if (normed)
		{
			for (int i = 0; i != y.size(); ++i)
			{
				y[i] /= sum_y * (bin_pos[i + 1] - bin_pos[i]);
			}
		}
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

		void show_ticks_internal(HDC hdc, HWND hwnd, RECT rect,	std::vector<double>
			range, TEXTMETRIC textMetric, RenderAxis *render);

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
			unsigned int size, RenderObjects *render_ptr);

		void set_legend(std::string name, std::string type, COLORREF color,
			unsigned int size);

		void activate_legend() { legend_state = true; }

		bool is_xlabel_activated() const { return !xlabel.empty(); }

		bool is_ylabel_activated() const { return !ylabel.empty(); }

		bool is_title_activated() const { return !title.empty(); }

		bool is_legend_activated() const { return legend_state; }

	private:
		class LEGEND
		{
		public:
			LEGEND() = delete;

			LEGEND(std::string in_name, std::string in_type, COLORREF in_color,
				unsigned int in_size) : name(in_name), type(in_type), color(in_color),
				size(in_size), render_pointer(nullptr) {};

			LEGEND(std::string in_name, std::string in_type, COLORREF in_color,
				unsigned int in_size, RenderObjects *render_ptr);

			LEGEND(LEGEND& legend)
			{
				name = legend.name;
				type = legend.type;
				color = legend.color;
				size = legend.size;
				render_pointer = legend.render_pointer;
				ownership_render_pointer = legend.ownership_render_pointer;
				legend.ownership_render_pointer = false;
			}

			~LEGEND();

			std::string name;
			std::string type;
			COLORREF color;
			unsigned int size;
			RenderObjects *render_pointer;

			bool ownership_render_pointer;
		};

		std::string xlabel;
		std::string ylabel;
		std::string title;
		std::vector<LEGEND> legend;

		bool legend_state;
	};

	Axis::LEGEND::LEGEND(std::string in_name, std::string in_type, 
		COLORREF in_color, unsigned int in_size, RenderObjects *render_ptr) :
		name(in_name), type(in_type), color(in_color), size(in_size),
		ownership_render_pointer(false)
	{
		if (render_ptr)
		{
			render_pointer = render_ptr;
			return;
		}
		ownership_render_pointer = true;

		if (type == "scatter")
		{
			render_pointer = new RenderScatterPoints();
		}
		else if (type == "line")
		{
			render_pointer = new RenderLinesFull();
		}
		else
		{
			printf("Warning: Unrecognized type for legend selected! Line pointer "
				"initialized!");

			render_pointer = new RenderLinesFull();
		}
	};

	Axis::LEGEND::~LEGEND()
	{
		if (ownership_render_pointer)
		{
			delete render_pointer;
		}
	}

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

	void Axis::show_ticks_internal(HDC hdc, HWND hwnd, RECT rect, std::vector<double>
		range, TEXTMETRIC textMetric, RenderAxis *render)
	{
		static unsigned int call_counter = 0;

		unsigned int stick = (unsigned int)(((rect.right - rect.left) -
			2.0 * textMetric.tmHeight) / 3.0);

		double factor = 0.0;
		double diff = range[1] - range[0];

		// Automatic scientific notation
		unsigned int value_digits = 0; // overall number of digits
		unsigned int value_int_digits = 0; // number of integral digits
		unsigned int exp_value_capacity = 0; // how many values are expected
		bool scientific = 0; // whether values will be rendered in the scientific notation

		// First check whether the difference is not too small, so that we 
		// should extract a certain value and keep only the rest in the rendering
		// of the axis -> e.g. when range is from 100.000001 to 100.000002, then
		// plot 0.000001 and 0.000002 in the scientific notation and add 100 to the label
		if (diff <= pow(10.0, -MIN_RANGE_DIFF))
		{
			scientific = 1;

			// Add-on that is going to be extracted and added to the label
			// Ranges need to be internally changed -> that is the reason why
			// range vector needs to be passed by value!
			int add_on = (int)range[0];
			range[0] = range[0] - add_on;
			range[1] = range[1] - add_on;
			if (call_counter++ == 0)
			{
				ylabel += " [ " + std::to_string(add_on) + "+ ]";
			}

			// 6 is a constant because basic notation is: 
			// e.g. (+/-)x.(SCIENTIFIC_FRAC_DIGITS)e(+/-)10
			value_digits = 6 + SCIENTIFIC_FRAC_DIGITS;

			// In case of negative number, also increase number of digits
			if (range[0] < 0)
			{
				++value_digits;
			}
		}
		else if (abs(range[1]) > MAX_RANGE_VALUE)
		{
			scientific = 1;

			// 6 is a constant because basic notation is: 
			// e.g. (+/-)1.(SCIENTIFIC_FRAC_DIGITS)e(+/-)10
			value_digits = 6 + SCIENTIFIC_FRAC_DIGITS;

			// In case of negative number, also increase number of digits
			if (range[0] < 0)
			{
				++value_digits;
			}
		}

		// Obtain the appropriate steps for ticks and values
		double value_period, tick_period;
		this->tick_value_map(diff, tick_period, value_period);

		// Find the starting values for the first tick and value
		int value_divisor = (int)ceil(range[0] / value_period);
		int tick_divisor = (int)ceil(range[0] / tick_period);

		double value = value_divisor * value_period;
		double tick = tick_divisor * tick_period;

		// Pre-compute variables
		double win_length = rect.bottom - rect.top;
		double length = range[1] - range[0];
		unsigned int x_coord = rect.right;
		unsigned int y_coord;

		// Render ticks
		while (tick < range[1])
		{
			y_coord = rect.bottom -
				(unsigned int)round((tick - range[0]) * win_length / length);

			render->render_tick(hdc, x_coord, y_coord, stick);

			tick += tick_period;
		}

		x_coord -= (unsigned int)(TICK_TEXT_FACTOR * stick);
		std::wstring y_text;
		
		// If the values do not classify as scientific, find the number of integral
		// and fractional digits -> e.g. 100.01 = 3 integral and 2 fractional
		if (!scientific)
		{
			double value_c = value;
			unsigned int current_digits = 0, current_int_digits = 0;
			while (value_c <= range[1])
			{
				current_int_digits = get_int_digits(value_c);
				current_digits = current_int_digits +
					get_frac_digits(value_c, MIN_RANGE_DIFF + 1);

				if (value_digits < current_digits)
				{
					value_digits = current_digits;
					value_int_digits = current_int_digits;
				}

				value_c += value_period;
				++exp_value_capacity;
			}
		}
		else
		{
			double value_c = value;
			while (value_c <= range[1])
			{
				value_c += value_period;
				++exp_value_capacity;
			}
		}

		// Find correct text length and max capacity of the rectangle
		unsigned int av_value_length = AXIS_VALUE_SPACE +
			value_digits * textMetric.tmAveCharWidth;
		unsigned int max_value_capacity = (rect.bottom - rect.top) / av_value_length;

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
			value_period *= halving;
		}

		// Make proper format for the text rendering via snprintf
		// char *format = "% int . frac f" or scientific notation
		char *sbuffer = new char[value_digits + 1];
		wchar_t *wbuffer = new wchar_t[value_digits + 1];

		char format[6];
		if (!scientific)
		{
			unsigned int value_frac_digits = value_digits - value_int_digits;
						
			if (value_frac_digits)
			{
				// minus one for the dot in case the number has fractional digits
				--value_frac_digits;
			}

			format[0] = '%';
			snprintf(&format[1], 2, "%u", value_int_digits);
			format[2] = '.';
			snprintf(&format[3], 2, "%u", value_frac_digits);
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
		while (value < range[1])
		{
			y_coord = rect.bottom -
				(unsigned int)round((value - range[0]) * win_length / length);

			// + 1 in y_value_digits for null-termination
			snprintf(sbuffer, value_digits + 1, format, value);

			MultiByteToWideChar(CP_UTF8, 0, sbuffer, -1, wbuffer, value_digits + 1);
			render->render_text(hdc, x_coord, y_coord, wbuffer, value_digits);

			value += value_period;
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

		// Render ticks on the x axis -> flip the rendering rectangle
		RenderAxisX renderX = RenderAxisX(x_rect);
		std::vector<double> x_range{ range[0], range[1] };
		RECT x_rect_flipped = {x_rect.top, x_rect.left, x_rect.bottom, x_rect.right};
		this->show_ticks_internal(hdc, hwnd, x_rect_flipped, x_range, textMetric, &renderX);

		// Set bottom and center text alignment
		SetTextAlign(hdc, TA_CENTER | TA_BOTTOM);

		// Set font rotation by 90 degrees
		LOGFONT lf;
		GetObject(font, sizeof(LOGFONT), &lf);
		lf.lfEscapement = 900;
		HFONT lfont = CreateFontIndirect(&lf);
		HFONT h_prev_font = (HFONT)SelectObject(hdc, (HGDIOBJ)(HFONT)(lfont));

		// Render ticks on the y axis
		RenderAxisY renderY = RenderAxisY();
		std::vector<double> y_range{ range[2], range[3] };
		this->show_ticks_internal(hdc, hwnd, y_rect, y_range, textMetric, &renderY);

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
		wchar_t *wide_xlabel = new wchar_t[xlabel.size() + 1];
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
				(it->render_pointer)->renderLegend(hdc, point_pos_x, point_pos_y);
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
		unsigned int size, RenderObjects *render_ptr)
	{
		legend.emplace_back(name, type, color, size, render_ptr);
	}

	void Axis::set_legend(std::string name, std::string type, COLORREF color,
		unsigned int size)
	{
		legend.emplace_back(name, type, color, size);
	}
}
