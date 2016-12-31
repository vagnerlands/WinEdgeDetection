# WinEdgeDetection
Windows application to perform Edge Detection on a given bitmap. 

This algorithm performs:
- Gaussian filter;
- Derivatives for vertical and horizontal pixels using Sobel;
- Calculates distances for color gradient intensity;
- Non Maximum supression
- Edges calculation (Canny);

Use the keys:
UP, DOWN, LEFT, RIGHT, '/', '*, HOME and END to perform changes on display.

Graphical display is being done using GDI.

<!> Input images shall be a valid BMP and maximum of 256 colors.
