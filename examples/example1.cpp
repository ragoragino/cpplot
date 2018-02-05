#include "D:\Materials\Programming\Projekty\cpplot\src\Figure.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <random>

int main()
{	
	// Create data
	int length = 1000;
	std::vector<double> x(length);
	std::vector<double> y(length);

	// Set the random generator
	std::mt19937_64 generator;
	generator.seed(123);
	std::normal_distribution<double> distribution(0.0, 1.0);

	// Fill the data container
	for (int i = 0; i != length; ++i)
	{
		x[i] = distribution(generator);
		y[i] = distribution(generator);
	}
	
	// Create plot
	cpplot::Figure plt(1000, 1000);
	plt.plot(x, y, "", "scatter", 5);
	plt.xlabel("Dimension 1");
	plt.ylabel("Dimension 2");
	plt.title("2-variate Normal Random Variable");
	// plt.save("example1", "png");
	plt.show();

	return 0;
}