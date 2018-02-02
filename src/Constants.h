#pragma once

// define floating-point error comparison allowance
#define FP_ERROR 0.0001

// COLORS
#define BLACK RGB(0, 0, 0)
#define BLUE RGB(0, 0, 255)
#define GREEN RGB(0, 255, 0)
#define GREY RGB(128, 128, 128)
#define LIGHT_GREY RGB(211, 211, 211)
#define ORANGE RGB(255, 128, 0)
#define PINK RGB(255, 0, 255)
#define RED RGB(255, 0, 0)
#define YELLOW RGB(255, 255, 0)
#define WHITE RGB(255, 255, 255)

// VARIABLES
// defines number of Graph objects handled in a single window
#ifndef MAX_GRAPHS
#define MAX_GRAPHS 10
#endif

// defines adjustment of left, right, up and bottom regions
#ifndef ADJUSTMENT_GRAPH
#define ADJUSTMENT_GRAPH 0.05
#endif

// define space between the Figure and Window = usable rectangle
#ifndef ADJUSTMENT_WINDOW
#define ADJUSTMENT_WINDOW 10
#endif

// define ratio of attribute to the width of the whole usable rectangle
#ifndef ATTRIBUTE_DISTANCE
#define ATTRIBUTE_DISTANCE 0.12
#endif

// define ratio of x axis tick area w.r.t. font size
#ifndef X_TICK_RATIO
#define X_TICK_RATIO (4.0 / 1.5)
#endif

// define ratio of y axis tick area w.r.t. font size
#ifndef Y_TICK_RATIO
#define Y_TICK_RATIO (4.0 / 1.5)
#endif

// define ratio of text height to title area
#ifndef TITLE_RATIO
#define TITLE_RATIO (3.0 / 2.0)
#endif

// absolute length of the symbol area (i.e. point, line) in the legend box
#ifndef LEGEND_SYMBOL_LENGTH
#define LEGEND_SYMBOL_LENGTH 15
#endif

// ratio of the legend area to the whole unadjusted graph area
#ifndef MAX_LEGEND_RATIO
#define MAX_LEGEND_RATIO 0.2
#endif

// free space between graph and legend
#ifndef GRAP_LEGEND_SPACE
#define GRAPH_LEGEND_SPACE 10
#endif

// Space between individual successive points in the legend
#ifndef LEGEND_TEXT_POINT_SPACE
#define LEGEND_TEXT_POINT_SPACE 10
#endif

// Space between individual individual rows in the legend
#ifndef LEGEND_TEXT_ROW_SPACE
#define LEGEND_TEXT_ROW_SPACE 5
#endif

// Necessary space between two tick values
#ifndef AXIS_VALUE_SPACE
#define AXIS_VALUE_SPACE 10
#endif

// Beginning of axis values from the axis as a ratio to tick length
#ifndef TICK_TEXT_FACTOR
#define TICK_TEXT_FACTOR 2
#endif

// Minimum difference, under which a shift in the representation
// of a number in the tick values changes
// e.g. the for the range of 10.000001 to 10.000002, one
// considers writing 10 next to thh label and 1e-6 to 2e-6 on the axis ticks
#ifndef MIN_RANGE_DIFF
#define MIN_RANGE_DIFF 3
#endif

// After this value, the ticks will be represented by scientific notation
#ifndef MAX_RANGE_VALUE
#define MAX_RANGE_VALUE 1000000
#endif

// How many digits the fractional part of the scientific notation should have
// above the basic one, e.g. basic is 5 in 1.5e10
#ifndef SCIENTIFIC_FRAC_DIGITS
#define SCIENTIFIC_FRAC_DIGITS 2
#endif