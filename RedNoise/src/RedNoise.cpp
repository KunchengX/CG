#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <CanvasPoint.h>
#include <Colour.h>
#include <ModelTriangle.h>
#include <RayTriangleIntersection.h>
#include <TextureMap.h>
#include <TexturePoint.h>
#include <iostream>
#include <string>
#include <sstream>

#define WIDTH 320
#define HEIGHT 240

void draw(DrawingWindow& window) {
	window.clearPixels();
	for (size_t y = 0; y < window.height; y++) {
		for (size_t x = 0; x < window.width; x++) {
			float red = rand() % 256;
			float green = 0.0;
			float blue = 0.0;
			uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
			window.setPixelColour(x, y, colour);
		}
	}
}

int judge(float number) {
	if (number < 0) return -1; // bottom-to-top
	else if (number == 0) return 0;
	else return 1; // top-to-bottom
}

std::vector<float> interpolateSingleFloats(float from, float to, size_t numberOfValue) {
	std::vector<float> result;
	float stepSize = (to - from) / (numberOfValue - 1);
	for (size_t i = 0; i < numberOfValue; i++) { 
		result.push_back(from + (float)i * stepSize); 
	}
	return result;
}

void drawGrey(DrawingWindow& window) {
	window.clearPixels();

	// Define the range of grayscale values (0.0 to 1.0)
	float startValue = 0.0;
	float endValue = 1.0;

	// Get the width of the window
	int width = window.width;

	// Generate the grayscale values using interpolation
	std::vector<float> gradientValues = interpolateSingleFloats(startValue, endValue, width);

	// Loop through each pixel in the window
	for (int y = 0; y < window.height; y++) {
		for (int x = 0; x < width; x++) {
			// Get the corresponding grayscale value for this pixel's x-coordinate
			float grayscaleValue = gradientValues[x];

			// Calculate the color value based on the grayscale (RGB channels are the same)
			int colorValue = static_cast<int>(grayscaleValue * 255);

			// Create a 32-bit integer color (RGBA format: 0xAARRGGBB)
			uint32_t color = (255 << 24) + (colorValue << 16) + (colorValue << 8) + colorValue;

			// Set the pixel's color in the window
			window.setPixelColour(x, y, color);
		}
	}

	// Render the frame to display the gradient
	window.renderFrame();
}


std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, size_t numberOfValues) {
	std::vector<glm::vec3> result;
	float stepSize = 1.0f / static_cast<float>(numberOfValues - 1);

	for (size_t i = 0; i < numberOfValues; i++) {
		glm::vec3 interpolatedValue = from + static_cast<float>(i) * stepSize * (to - from);
		result.push_back(interpolatedValue);
	}

	return result;
}


void drawRainbow(DrawingWindow& window) {
	window.clearPixels();

	// Define the corner colors of the window
	glm::vec3 topLeft(255, 0, 0);        // red
	glm::vec3 topRight(0, 0, 255);       // blue
	glm::vec3 bottomRight(0, 255, 0);    // green
	glm::vec3 bottomLeft(255, 255, 0);   // yellow

	// Get the width and height of the window
	int width = window.width;
	int height = window.height;

	// Calculate the colors for the left-most and right-most columns
	std::vector<glm::vec3> leftColumnColors = interpolateThreeElementValues(topLeft, bottomLeft, height);
	std::vector<glm::vec3> rightColumnColors = interpolateThreeElementValues(topRight, bottomRight, height);

	// Loop through each row in the window
	for (int y = 0; y < height; y++) {
		// Interpolate between the left and right column colors for this row
		std::vector<glm::vec3> rowColors = interpolateThreeElementValues(leftColumnColors[y], rightColumnColors[y], width);

		// Loop through each pixel in the row
		for (int x = 0; x < width; x++) {
			// Get the interpolated color for this pixel
			glm::vec3 color = rowColors[x];

			// Create a 32-bit integer color (RGBA format: 0xAARRGGBB)
			uint32_t pixelColor = (255 << 24) + (static_cast<int>(color.r) << 16) + (static_cast<int>(color.g) << 8) + static_cast<int>(color.b);

			// Set the pixel's color in the window
			window.setPixelColour(x, y, pixelColor);
		}
	}

	// Render the frame to display the color spectrum
	window.renderFrame();
}

void drawLine(DrawingWindow& window, CanvasPoint from, CanvasPoint to, Colour color) {
	int x0 = from.x;
	int y0 = from.y;
	int x1 = to.x;
	int y1 = to.y;

	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);
	int sx = (x0 < x1) ? 1 : -1;
	int sy = (y0 < y1) ? 1 : -1;
	int err = dx - dy;

	while (true) {
		// Calculate the 32-bit integer color value (RGBA format: 0xAARRGGBB)
		uint32_t pixelColor = (255 << 24) + (color.red << 16) + (color.green << 8) + color.blue;

		// Set the pixel's color in the window
		window.setPixelColour(x0, y0, pixelColor);

		if (x0 == x1 && y0 == y1) {
			break;
		}

		int e2 = 2 * err;

		if (e2 > -dy) {
			err -= dy;
			x0 += sx;
		}

		if (e2 < dx) {
			err += dx;
			y0 += sy;
		}
	}
}

void drawTriangle(DrawingWindow& window, CanvasTriangle triangle, Colour color) {
	drawLine(window, triangle[0], triangle[1], color);
	drawLine(window, triangle[1], triangle[2], color);
	drawLine(window, triangle[2], triangle[0], color);
}

void drawFilledTriangle(DrawingWindow& window, CanvasTriangle triangle, Colour color) {
	// Sort vertices based on their y-coordinate (top to bottom)
	if (triangle[1].y < triangle[0].y) std::swap(triangle[0], triangle[1]);
	if (triangle[2].y < triangle[0].y) std::swap(triangle[0], triangle[2]);
	if (triangle[2].y < triangle[1].y) std::swap(triangle[1], triangle[2]);

	CanvasPoint top = triangle[0];
	CanvasPoint mid = triangle[1];
	CanvasPoint bot = triangle[2];

	float slope1 = (mid.x - top.x) / (mid.y - top.y);
	float slope2 = (bot.x - top.x) / (bot.y - top.y);

	float currentX1 = top.x;
	float currentX2 = top.x;
	for (int y = top.y; y <= mid.y; y++) {
		drawLine(window, { currentX1, static_cast<float>(y) }, { currentX2, static_cast<float>(y) }, color);

		currentX1 += slope1;
		currentX2 += slope2;
	}

	// Now we have a second triangle (top,mid,bot). We need to find the slope to draw the bottom half of the triangle.
	// But since we are using this to rasterize and fill our triangle, we have to determine if the first line or the second line will be longer.
	if (bot.y != mid.y) {
		slope1 = (bot.x - mid.x) / (bot.y - mid.y);
		currentX1 = mid.x;
	}

	for (int y = mid.y; y <= bot.y; y++) {
		drawLine(window, { currentX1, static_cast<float>(y) }, { currentX2, static_cast<float>(y) }, color);

		currentX1 += slope1;
		currentX2 += slope2;
	}

	// Draw the white stroked triangle over the filled triangle
	Colour white(255, 255, 255);
	drawTriangle(window, triangle, white);
}

TextureMap getTextureMap(const std::string& image) {
	TextureMap textureMap = TextureMap(image);
	std::cout << "width " << textureMap.width << "height " << textureMap.height << std::endl;
	return textureMap;
}

// Function to perform texture mapping using barycentric coordinates
uint32_t textureMapper(TextureMap textureMap, CanvasTriangle triangle, CanvasPoint point) {
	// Calculate barycentric coordinates (u, v, w) for the given point inside the triangle
	float u = (-(point.x - triangle.v1().x) * (triangle.v2().y - triangle.v1().y) + (point.y - triangle.v1().y) * (triangle.v2().x - triangle.v1().x)) / (-(triangle.v0().x - triangle.v1().x) * (triangle.v2().y - triangle.v1().y) + (triangle.v0().y - triangle.v1().y) * (triangle.v2().x - triangle.v1().x));
	float v = (-(point.x - triangle.v2().x) * (triangle.v0().y - triangle.v2().y) + (point.y - triangle.v2().y) * (triangle.v0().x - triangle.v2().x)) / (-(triangle.v1().x - triangle.v2().x) * (triangle.v0().y - triangle.v2().y) + (triangle.v1().y - triangle.v2().y) * (triangle.v0().x - triangle.v2().x));
	float w = 1 - u - v;
	
	// Calculate the texture coordinate for the given point using barycentric interpolation
	CanvasPoint texturePoint((u * triangle.v0().texturePoint.x + v * triangle.v1().texturePoint.x + w * triangle.v2().texturePoint.x), (u * triangle.v0().texturePoint.y + v * triangle.v1().texturePoint.y + w * triangle.v2().texturePoint.y));
	
	// Calculate the index in the texture map array corresponding to the texture coordinate
	int index = int(texturePoint.y) * textureMap.width + int(texturePoint.x);

	// Retrieve the color of the texture from the texture map using the calculated index
	uint32_t colour = textureMap.pixels[index - 1];	// The index starts from 1
	return colour;
}

// fill a triangle with texture map
void textureTriangle(DrawingWindow& window, const TextureMap& textureMap, CanvasTriangle triangle, CanvasPoint v1, CanvasPoint v2, CanvasPoint v3) {
	CanvasPoint newV1 = v1;
	CanvasPoint newV2 = v1;
	for (int i = 1; i <= fabs(v2.y - v1.y); i++) {
		float f = i / fabs(v2.y - v1.y);
		newV1.x = v1.x + (v2.x - v1.x) * f;
		newV1.y -= judge(v1.y - v3.y);
		newV2.x = v1.x + (v3.x - v1.x) * f;
		newV2.y -= judge(v1.y - v3.y);
		for (float j = 0.0; j < (newV2.x - newV1.x); j++) {
			float x = newV1.x + j;
			float y = newV1.y;
			uint32_t colour = textureMapper(textureMap, triangle, CanvasPoint(x, y));
			window.setPixelColour(x, y, colour);
		}
	}
}

void drawTextureTriangle(DrawingWindow& window, const TextureMap& textureMap, CanvasTriangle triangle) {
	// draw white stroked triangle
	drawTriangle(window, triangle, Colour(255, 255, 255));

	// Sort vertices based on their y-coordinate (top to bottom)
	if (triangle[1].y < triangle[0].y) std::swap(triangle[0], triangle[1]);
	if (triangle[2].y < triangle[0].y) std::swap(triangle[0], triangle[2]);
	if (triangle[2].y < triangle[1].y) std::swap(triangle[1], triangle[2]);

	CanvasPoint top = triangle[0];
	CanvasPoint mid = triangle[1];
	CanvasPoint bot = triangle[2];

	CanvasPoint extra;
	double R = (bot.x - top.x) / (bot.y - top.y);
	float EX;
	if (R >= 0) {
		EX = top.x + ((mid.y - top.y) * fabs(R));
	}
	else {
		EX = top.x - ((mid.y - top.y) * fabs(R));
	}
	extra = CanvasPoint(EX, mid.y);
	// Fill the top and bottom parts of the triangle with the texture
	textureTriangle(window, textureMap, triangle, top, mid, extra);
	textureTriangle(window, textureMap, triangle, bot, mid, extra);
}


void handleEvent(SDL_Event event, DrawingWindow& window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
		else if (event.key.keysym.sym == SDLK_u) {	// Test Week 3 Task 3 draw stroked triangle
			// Generate three random points for the triangle
			CanvasPoint p1(rand() % window.width, rand() % window.height);
			CanvasPoint p2(rand() % window.width, rand() % window.height);
			CanvasPoint p3(rand() % window.width, rand() % window.height);

			// Generate a random color for the triangle
			Colour color(rand() % 256, rand() % 256, rand() % 256);

			// Create the triangle and draw it
			CanvasTriangle triangle(p1, p2, p3);
			drawTriangle(window, triangle, color);
		}
		else if (event.key.keysym.sym == SDLK_f) {	// Test Week 3 Task 4 draw filled triangle
			// Generate three random points for the triangle
			CanvasPoint p1(rand() % window.width, rand() % window.height);
			CanvasPoint p2(rand() % window.width, rand() % window.height);
			CanvasPoint p3(rand() % window.width, rand() % window.height);

			// Generate a random color for the triangle
			Colour color(rand() % 256, rand() % 256, rand() % 256);

			// Create the triangle and draw the filled triangle with white stroked triangle
			CanvasTriangle triangle(p1, p2, p3);
			drawFilledTriangle(window, triangle, color);
		}
		else if (event.key.keysym.sym == SDLK_t) {    // Test Week 3 Task 5 drawTextureTriangle
			// Load a texture map (need to be in the same directory as RedNoise.exe)
			TextureMap textureMap = getTextureMap("texture.ppm");

			CanvasPoint point1, point2, point3;
			point1 = CanvasPoint(160, 10);
			point2 = CanvasPoint(300, 230);
			point3 = CanvasPoint(10, 150);
			point1.texturePoint.x = 195; point1.texturePoint.y = 5;
			point2.texturePoint.x = 395; point2.texturePoint.y = 380;
			point3.texturePoint.x = 65; point3.texturePoint.y = 330;
			CanvasTriangle triangle = CanvasTriangle(point1, point2, point3);
			drawTextureTriangle(window, textureMap, triangle);
		}
	}
	else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char* argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;

	// Test Week 2 Task 2 interpolateSingleFloats
	//std::vector<float> result;
	//result = interpolateSingleFloats(2.2, 8.5, 7);
	//for (size_t i = 0; i < result.size(); i++) {
	//	std::cout << result[i] << " ";
	//}
	//std::cout << std::endl;

	// Test Week 2 Task 4 interpolateThreeElementValues
	//glm::vec3 from(1.0, 4.0, 9.2);
	//glm::vec3 to(4.0, 1.0, 9.8);
	//std::vector<glm::vec3> result = interpolateThreeElementValues(from, to, 4);

	//for (size_t i = 0; i < result.size(); i++) {
	//	glm::vec3 value = result[i];
	//	std::cout << "(" << value.x << ", " << value.y << ", " << value.z << ")" << std::endl;
	//}

	// Test Week 3 Task 2
	// Test lines
	//drawLine(window, CanvasPoint(0, 0), CanvasPoint(window.width / 2, window.height / 2), Colour(255, 0, 0));  // Red line from top-left to center
	//drawLine(window, CanvasPoint(window.width / 2, 0), CanvasPoint(window.width / 2, window.height), Colour(0, 255, 0));  // Green vertical line in the middle
	//drawLine(window, CanvasPoint(window.width / 3, window.height / 2), CanvasPoint(window.width * 2 / 3, window.height / 2), Colour(0, 0, 255));  // Blue horizontal line in the center



	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		
		// Test Week 1 Task 1
		//draw(window);
		// Test Week 2 Task 3
		//drawGrey(window);
		// Test Week 2 Task 5
		//drawRainbow(window);
		
		// Test Week 3 Task 5
		//textureCanvas(window);

		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}