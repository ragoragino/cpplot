#include "D:\Materials\Programming\Projekty\cpplot\src\Figure.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <random>

int main()
{
	// Create data
	int length = 100;
	std::vector<double> x(length);
	std::vector<double> y(length);
	std::vector<double> z(length);

	std::mt19937_64 generator;
	generator.seed(123);
	std::normal_distribution<double> distribution(0.0, 1.0);

	for (int i = 0; i != length; ++i)
	{
		x[i] = distribution(generator);
		y[i] = distribution(generator);
		z[i] = distribution(generator);
	}

	// Create Figure object
	cpplot::Figure plt(
		std::vector<int> { 900, 900 },
		std::vector<int> { 600, 600 },
		std::vector<COLORREF>{ },
		true
	);

	// Iterate over individual graphs
	cpplot::RenderObjects *rptr_dotted = &cpplot::RenderLinesDotted(10);
	cpplot::RenderObjects *rptr_squares = &cpplot::RenderScatterSquares();
	for (int i = 0; i != 2; ++i)
	{
		for (int j = 0; j != 2; ++j)
		{
			plt.plot(x, y, "Line A", "line", 1, BLUE, { i, j });
			plt.plot(x, z, "Line B", "line", 1, GREEN, { i, j }, rptr_dotted);
			plt.plot(x, y, "Scatter A", "scatter", 5, BLUE, { i, j });
			plt.plot(x, z, "Scatter B", "scatter", 3, GREEN, { i, j }, rptr_squares);
			plt.xlabel("Index");
			plt.ylabel("Value");
			plt.title("Graph");
		}
	}

	plt.legend();
	plt.save("example4", "png");

	return 0;
}