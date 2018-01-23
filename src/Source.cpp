#include "Figure.h"
#define _USE_MATH_DEFINES
#include <math.h>

int main()
{
	
	// Create data
	unsigned int length = 1000;
	std::vector<double> x(length);
	std::vector<double> y(length);
	std::vector<double> z(length);
	for (int i = 0; i != length; ++i)
	{
		x[i] = -1.0 * (double)rand() / (double)RAND_MAX;
		y[i] = -5.0 * (double)rand() / (double)RAND_MAX;
		z[i] = -10.0 * (double)rand() / (double)RAND_MAX;
	}
	/*
	cpplot::Figure plt(1000, 1000, RGB(100, 0, 200));
	plt.plot(y, "Super", "line", 1, PINK, std::vector<unsigned int>{});
	plt.legend();
	plt.show();
	*/
	
	// Create Figure object
	cpplot::Figure plt(
		std::vector<unsigned int> { 1600 }, 
		std::vector<unsigned int> { 600, 600 }, 
		std::vector<COLORREF>{ LIGHT_GREY, LIGHT_GREY }
	);
	
	// Iterate over individual graphs
	for (unsigned int i = 0; i != 2; ++i)
	{
		plt.plot(x, "Line A", "line", 1, BLUE, { i, 0 });
		plt.plot(y, "Line B", "line", 1, RED, { i, 0 });
		plt.plot(z, "Scatter C", "scatter", 3, YELLOW, { i, 0 });
		plt.xlabel("Index");
		plt.ylabel("Value");
		plt.title("Graph");
	}
	plt.legend();
	// plt.set_font();
	plt.save("PLOT");
	// plt.show();
	
	return 0;
}