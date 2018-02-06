#pragma once
#include "Header.h"

namespace cpplot
{

	/*
	Class interface that allows using unified rendering for ticks.
	RenderAxisX flips back flipped coordinates pushed to the
	show_ticks_internal fuction in the Axis class for x axis.
	*/
	class RenderAxis
	{
	public:
		virtual void render_text(HDC hdc, int x, int y,
			wchar_t* format, int length) = 0;

		virtual void render_tick(HDC hdc, int x, int y,
			int stick) = 0;
	};

	class RenderAxisX : public RenderAxis
	{
	public:
		RenderAxisX(RECT in_rect) : rect(in_rect) {};

		virtual void render_text(HDC hdc, int x, int y,
			wchar_t* format, int length);

		virtual void render_tick(HDC hdc, int x, int y,
			int stick);

	private:
		RECT rect;
	};

	class RenderAxisY : public RenderAxis
	{
	public:
		virtual void render_text(HDC hdc, int x, int y,
			wchar_t* format, int length);

		virtual void render_tick(HDC hdc, int x, int y,
			int stick);
	};

	inline void RenderAxisX::render_text(HDC hdc, int x,
		int y, wchar_t* text, int length)
	{
		int adj_x = rect.right - (y - rect.left);
		int adj_y = rect.bottom - (x - rect.top);

		TextOut(hdc, adj_x, adj_y, text, length);
	}

	inline void RenderAxisX::render_tick(HDC hdc, int x,
		int y, int stick)
	{
		int adj_x = rect.right - (y - rect.left);
		int adj_y = rect.bottom - (x - rect.top);

		MoveToEx(hdc, adj_x, adj_y, NULL);
		LineTo(hdc, adj_x, adj_y + stick);
	}

	inline void RenderAxisY::render_text(HDC hdc, int x,
		int y, wchar_t* text, int length)
	{
		TextOut(hdc, x, y, text, length);
	}

	inline void RenderAxisY::render_tick(HDC hdc, int x,
		int y, int stick)
	{
		MoveToEx(hdc, x, y, NULL);
		LineTo(hdc, x - stick, y);
	}


	/*
	Class interface that allows arbitrary scatter or line type.
	For proper functioning, the class needs to have two function:
	renderLegend and either renderPoint or renderLines.
	*/

	class RenderObjects
	{
	public:
		virtual void renderPoints(HDC hdc, const std::vector<double>& x,
			const std::vector<double>& y, RECT rect, const std::vector<double>& range) {};

		virtual void renderLines(HDC hdc, const std::map<double, double>& data, RECT rect,
			const std::vector<double>& range) {};

		virtual void renderLegend(HDC hdc, RECT pos) {};
	};

	class RenderScatter : public RenderObjects
	{
	public:
		virtual void renderPoints(HDC hdc, const std::vector<double>& x,
			const std::vector<double>& y, RECT rect, const std::vector<double>& range) {};

		virtual void renderLegend(HDC hdc, RECT pos) {};
	};

	class RenderScatterPoints : public RenderScatter
	{
	public:
		virtual void renderPoints(HDC hdc, const std::vector<double>& x,
			const std::vector<double>& y, RECT rect, const std::vector<double>& range);

		virtual void renderLegend(HDC hdc, RECT pos);
	};

	inline void RenderScatterPoints::renderPoints(HDC hdc, const std::vector<double>& x,
		const std::vector<double>& y, RECT rect, const std::vector<double>& range)
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
		int x_coord, y_coord;

		// Draw the points
		int x_size = x.size();
		for (int i = 0; i != x_size; ++i)
		{
			x_coord = rect.left +
				(int)round((x[i] - min_x) * win_length_x / length_x);
			y_coord = rect.bottom -
				(int)round((y[i] - min_y) * win_length_y / length_y);

			Ellipse(hdc, x_coord - 1, y_coord - 1, x_coord + 1, y_coord + 1);
		}
	}

	inline void RenderScatterPoints::renderLegend(HDC hdc, RECT pos)
	{
		int x = (int)(0.5 * (pos.left + pos.right));
		int y = (int)(0.5 * (pos.top + pos.bottom));

		Ellipse(hdc, x - 1, y - 1, x + 1, y + 1);
	}

	class RenderScatterSquares : public RenderScatter
	{
	public:
		virtual void renderPoints(HDC hdc, const std::vector<double>& x,
			const std::vector<double>& y, RECT rect, const std::vector<double>& range);

		void renderLegend(HDC hdc, RECT pos);
	};

	inline void RenderScatterSquares::renderPoints(HDC hdc, const std::vector<double>& x,
		const std::vector<double>& y, RECT rect, const std::vector<double>& range)
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
		int x_coord, y_coord;

		// Draw the points
		int x_size = x.size();
		for (int i = 0; i != x_size; ++i)
		{
			x_coord = rect.left +
				(int)round((x[i] - min_x) * win_length_x / length_x);
			y_coord = rect.bottom -
				(int)round((y[i] - min_y) * win_length_y / length_y);

			Rectangle(hdc, x_coord - 2, y_coord - 2, x_coord + 2, y_coord + 2);
		}
	}

	inline void RenderScatterSquares::renderLegend(HDC hdc, RECT pos)
	{
		int x = (int)(0.5 * (pos.left + pos.right));
		int y = (int)(0.5 * (pos.top + pos.bottom));

		Rectangle(hdc, x - 2, y - 2, x + 2, y + 2);
	}

	class RenderLines : public RenderObjects
	{
	public:
		virtual void renderLiness(HDC hdc, const std::map<double, double>& data, RECT rect,
			const std::vector<double>& range) {};

		virtual void renderLegend(HDC hdc, RECT pos) {};
	};

	class RenderLinesFull : public RenderLines
	{
	public:
		virtual void renderLines(HDC hdc, const std::map<double, double>& data, RECT rect,
			const std::vector<double>& range);

		virtual void renderLegend(HDC hdc, RECT pos);
	};

	inline void RenderLinesFull::renderLines(HDC hdc, const std::map<double, double>& data, RECT rect,
		const std::vector<double>& range)
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
		int x_coord = rect.left +
			(int)round((data.begin()->first - min_x) * win_length_x / length_x);
		int y_coord = rect.bottom -
			(int)round((data.begin()->second - min_y) * win_length_y / length_y);
		MoveToEx(hdc, x_coord, y_coord, NULL);

		for (std::map<double, double>::const_iterator it = data.begin();
			it != data.end(); ++it)
		{
			x_coord = rect.left +
				(int)round((it->first - min_x) * win_length_x / length_x);
			y_coord = rect.bottom -
				(int)round((it->second - min_y) * win_length_y / length_y);

			LineTo(hdc, x_coord, y_coord);
		}
	}

	inline void RenderLinesFull::renderLegend(HDC hdc, RECT pos)
	{
		int begin_x = (int)(pos.left + 0.25 * (pos.right - pos.left));
		int end_x = (int)(pos.left + 0.75 * (pos.right - pos.left));
		int y = (int)(0.5 * (pos.top + pos.bottom));

		MoveToEx(hdc, begin_x, y, NULL);
		LineTo(hdc, end_x, y);
	}

	class RenderLinesDotted : public RenderLines
	{
	public:
		RenderLinesDotted(int dot_length) : dot_length(dot_length) {};

		virtual void renderLines(HDC hdc, const std::map<double, double>& data, RECT rect,
			const std::vector<double>& range);

		virtual void renderLegend(HDC hdc, RECT pos);

	private:
		struct Point
		{
			Point() : x(0), y(0) {};

			Point(int in_x, int in_y) : x(in_x), y(in_y) {};

			int x, y;
		};

		double norm(Point a, Point b)
		{
			double distance = (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);

			return sqrt(distance);
		};

		Point interpolate(Point a, Point b, double alpha)
		{
			Point point;
			point.x = (int)(a.x * (1.0 - alpha) + b.x * alpha);
			point.y = (int)(a.y * (1.0 - alpha) + b.y * alpha);

			return point;
		}

		int dot_length;
	};


	inline void RenderLinesDotted::renderLines(HDC hdc, const std::map<double, double>& data, RECT rect,
		const std::vector<double>& range)
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

		// Starting point
		Point start_data, end_data;
		start_data.x = rect.left +
			(int)round((data.begin()->first - min_x) * win_length_x / length_x);
		start_data.y = rect.bottom -
			(int)round((data.begin()->second - min_y) * win_length_y / length_y);

		double distance; // length of the current line
		double start_dot_length = 0.0; // current dot start on the current line
		double end_dot_length = (double)dot_length; // current dot end on the current line
		Point start_point, end_point; // starting point and ending point of the current line

		// Start the data iterator at the second point
		std::map<double, double>::const_iterator it = ++data.begin();

		// Render the dotted line
		for (; it != data.end(); ++it)
		{
			end_data.x = rect.left +
				(int)round((it->first - min_x) * win_length_x / length_x);
			end_data.y = rect.bottom -
				(int)round((it->second - min_y) * win_length_y / length_y);

			// Compute the length of the current line
			distance = norm(start_data, end_data);

			// Render every second dot_length
			while (true)
			{
				start_point = interpolate(start_data, end_data, start_dot_length / distance);
				end_point = interpolate(start_data, end_data, end_dot_length / distance);

				MoveToEx(hdc, start_point.x, start_point.y, NULL);
				LineTo(hdc, end_point.x, end_point.y);

				start_dot_length = norm(end_point, start_data) + dot_length;
				end_dot_length = start_dot_length + dot_length;

				if (end_dot_length > distance)
				{
					end_dot_length = dot_length;
					break;
				}
			}

			// In case we can still paint a part of the current dotted segment,
			// we divide the segment. In case we cannot, we move to the next segment
			// with an appropriate offset
			if (start_dot_length < distance)
			{
				start_point = interpolate(start_data, end_data, start_dot_length / distance);
				end_point = end_data;

				MoveToEx(hdc, start_point.x, start_point.y, NULL);
				LineTo(hdc, end_point.x, end_point.y);

				end_dot_length -= norm(start_point, end_point);
				start_dot_length = 0.0;
			}
			else
			{
				// There needs to be this test, because of distance and start_dot_length being 0
				if ((abs(distance) - FP_ERROR) > 0)
				{
					start_dot_length -= distance;
					end_dot_length = start_dot_length + dot_length;
				}
			}

			start_data.x = end_data.x;
			start_data.y = end_data.y;
		}
	}

	inline void RenderLinesDotted::renderLegend(HDC hdc, RECT pos)
	{
		int begin_x = (int)(pos.left + 0.25 * (pos.right - pos.left));
		int end_x = (int)(pos.left + 0.75 * (pos.right - pos.left));
		int y = (int)(0.5 * (pos.top + pos.bottom));

		MoveToEx(hdc, begin_x, y, NULL);
		LineTo(hdc, end_x, y);
	}
}