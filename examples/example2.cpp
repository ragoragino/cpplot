#include "D:\Materials\Programming\Projekty\cpplot\src\Figure.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <random>

int main()
{	
	// Create Figure object
	cpplot::Figure plt(
		std::vector<int> { 1500 }, 
		std::vector<int> { 1000 }, 
		std::vector<COLORREF>{ LIGHT_GREY }
	);
	
	// Plot sine and cosine
	cpplot::RenderObjects *rptr = &cpplot::RenderLinesDotted(10);
	plt.fplot(sin, -M_PI, M_PI, "Sine", "line", 3, RED, { 0, 0 }, rptr);
	plt.fplot(cos, -M_PI, M_PI, "Cosine", "line", 3, GREEN, { 0, 0 }, rptr);
	plt.xlabel("Index");
	plt.ylabel("Value");
	plt.title("Sine and Cosine");	
	plt.legend();
	plt.save("example2", "png");
	
	return 0;
}