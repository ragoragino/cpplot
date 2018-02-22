#include "D:\Materials\Programming\Projekty\cpplot\src\Figure.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <random>

int main()
{	
	// Allocate vectors for data
	int length = 100000;
	std::vector<double> x(length);
	std::vector<double> y(length);

	// Set distributions
	std::mt19937_64 generator;
	generator.seed(123);
	std::normal_distribution<double> distribution_n(0.0, 1.0);
	std::exponential_distribution<double> distribution_e(1.0);

	// Fill in the data
	for (int i = 0; i != length; ++i)
	{
		x[i] = distribution_n(generator);
		y[i] = distribution_e(generator);
	}
	
	// Create Figure object
	cpplot::Figure plt(
		std::vector<int> { 1000 }, 
		std::vector<int> { 500, 500 }
	);
	
	// Plot individual windows
	plt.hist(x, 100, {}, "", 1, GREEN, true, { 0, 0 });
	plt.title("Histogram of a sample of normally distributed r.v.s, normalized");
	plt.hist(y, 100, {}, "", 1, BLUE, true, { 1, 0 });
	plt.title("Histogram of a sample of exponentially distributed r.v.s, normalized");
	plt.save("example3", "png");

	return 0;
}