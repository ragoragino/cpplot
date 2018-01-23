Cpplot is a simple single-header library for plotting in C++. Written in WINAPI and with the use of STL.

The library runs only on Windows, however, only native libraries are used and therefore no additional
dependencies are required.

### Example

```cpp
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
	plt.show();
	
	return 0;
}
```

The example above should give you something like this:

![](https://github.com/ragoragino/cpplot/PLOT.bmp?raw=true)


### Note
The project is still in active development phase.

