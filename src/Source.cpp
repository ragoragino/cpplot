#include "Figure.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <random>

int main()
{	
	// Create data
	unsigned int length = 100000;
	std::vector<double> x(length);
	std::vector<double> y(length);
	std::vector<double> z(length);

	std::default_random_engine generator;
	generator.seed(std::random_device()());
	std::normal_distribution<double> distribution(0.0, 1.0);

	for (unsigned int i = 0; i != length; ++i)
	{
		//x[i] = 10.0 * (double)rand() / (double)RAND_MAX;
		//y[i] = 10.0 * (double)rand() / (double)RAND_MAX;
		//z[i] = 10.0 * (double)rand() / (double)RAND_MAX;
		y[i] = distribution(generator);
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
		// plt.plot(x, "Line A", "line", 1, BLUE, { i, 0 });
		// plt.plot(x, y, "Scatter B", "scatter", 3, RED, { i, 0 });
		// plt.plot(x, z, "Scatter C", "scatter", 3, YELLOW, { i, 0 });
		// plt.fplot(sin, 0,  2.0 * M_PI, "Sine", "line", 1, PINK, { i, 0 });
		plt.hist(y, 100, "Histogram", 1, GREEN, false, { i, 0 });
		plt.xlabel("Index");
		plt.ylabel("Value");
		plt.title("Graph");
	}
	// plt.legend();
	plt.save("PLOT", "png");
	// plt.show();
	
	return 0;
}