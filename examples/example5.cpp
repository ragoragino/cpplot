#include "Figure.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <random>

int main()
{	
	// Create data
	int length = 1000;
	std::vector<double> x(length);
	std::vector<double> y(length);
	std::vector<double> z(length);

	std::default_random_engine generator;
	generator.seed(123);
	std::normal_distribution<double> distribution(0.0, 1.0);

	for (int i = 0; i != length; ++i)
	{
		x[i] = distribution(generator);
		y[i] = distribution(generator);
		z[i] = distribution(generator);
		// y[i] = distribution(generator);
	}
	/*
	cpplot::Figure plt(1000, 1000, RGB(100, 0, 200));
	plt.plot(y, "Super", "line", 1, PINK, std::vector<unsigned int>{});
	plt.legend();
	plt.show();
	*/
	
	// Create Figure object
	cpplot::Figure plt(
		std::vector<int> { 400, 400 }, 
		std::vector<int> { 400, 400 }, 
		std::vector<COLORREF>{ LIGHT_GREY, LIGHT_GREY, LIGHT_GREY, LIGHT_GREY },
		true
	);
	
	// Iterate over individual graphs
	cpplot::RenderObjects *rptr = &cpplot::RenderLinesDotted(10);
	for (int i = 0; i != 2; ++i)
	{
		for (int j = 0; j != 2; ++j)
		{
			plt.plot(x, y, "Line A", "line", 1, PINK, { i, j });
			plt.plot(x, z, "Line B", "line", 1, YELLOW, { i, j });
			plt.plot(x, y, "Scatter A", "scatter", 5, PINK, { i, j });
			plt.plot(x, z, "Scatter B", "scatter", 5, YELLOW, { i, j });
			plt.fplot(sin, -M_PI, M_PI, "Sine", "line", 3, GREEN, { i, j }, rptr);
			// plt.hist(y, 100, std::vector<double>{-10.0, 10.0}, "Histogram", 1, GREEN, true, { i, 0 });
			plt.xlabel("Index");
			plt.ylabel("Value");
			plt.title("Graph");
		}		
	}
	plt.legend();
	// plt.save("PLOT", "png");
	plt.show();
	
	return 0;
}