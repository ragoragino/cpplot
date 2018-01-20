#include "Figure.h"

int main()
{
	/*
	std::vector<double> y = { 1.0,2.0,3.0 };
	cpplot::Figure plt(1000, 1000, RGB(100, 0, 200));
	plt.plot(y, "line", 1.0, PINK, std::vector<unsigned int>{});
	plt.show();
	*/
	
	cpplot::Figure plt(
		std::vector<unsigned int> { 800 }, 
		std::vector<unsigned int> { 400, 400 }, 
		std::vector<COLORREF>{LIGHT_GREY, LIGHT_GREY}
	);
	
	unsigned int length = 100;
	std::vector<double> y(length);
	std::vector<double> z(length);
	for (int i = 0; i != length; ++i)
	{
		y[i] = - 5.0 * (double)rand() / (double)RAND_MAX;
		z[i] = - 10.0 * (double)rand() / (double)RAND_MAX;
	}
	
	for (unsigned int i = 0; i != 2; ++i)
	{
		plt.plot(y, "scatter", 3, BLUE, { i, 0 });
		plt.plot(z, "scatter", 3, RED, { i, 0 });
		plt.xlabel("Time");
		plt.ylabel("Price");
	}
	// plt.set_font();
	// plt.save("PLOT");
	plt.show();
	
	return 0;
}