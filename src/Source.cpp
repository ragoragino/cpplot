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
	for (unsigned int i = 0; i != length; ++i)
	{
		x[i] = -20.0 + 0.01 * (double)rand() / (double)RAND_MAX;
		y[i] = -20.0 + 0.01 * (double)rand() / (double)RAND_MAX;
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
		std::vector<unsigned int> { 800 }, 
		std::vector<unsigned int> { 400, 400 }, 
		std::vector<COLORREF>{ LIGHT_GREY, LIGHT_GREY }
	);
	

	// Iterate over individual graphs
	for (unsigned int i = 0; i != 2; ++i)
	{
		plt.plot(x, "Line A", "line", 1, BLUE, { i, 0 });
		plt.plot(y, "Line B", "line", 1, RED, { i, 0 });
		plt.plot(y, "Scatter C", "scatter", 3, YELLOW, { i, 0 });
		// plt.fplot(sin, 0,  2.0 * M_PI, "Sine", "line", 1, PINK, { i, 0 });
		plt.xlabel("Index");
		plt.ylabel("Value");
		plt.title("Graph");
	}
	plt.legend();
	// plt.save("PLOT");
	plt.show();
	
	return 0;
}