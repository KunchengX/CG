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
	drawLine(window, triangle[0], triangle[1], white);
	drawLine(window, triangle[1], triangle[2], white);
	drawLine(window, triangle[2], triangle[0], white);
}

void drawTextureTriangle(DrawingWindow& window, CanvasTriangle triangle, Colour color, TextureMap& texture) {
	// Sort vertices based on their y-coordinate (top to bottom)
	if (triangle[1].y < triangle[0].y) std::swap(triangle[0], triangle[1]);
	if (triangle[2].y < triangle[0].y) std::swap(triangle[0], triangle[2]);
	if (triangle[2].y < triangle[1].y) std::swap(triangle[1], triangle[2]);

	// Calculate the texture coordinates for each vertex
	glm::vec2 texCoord1 = triangle[0].texturePoint.coordinates;
	glm::vec2 texCoord2 = triangle[1].texturePoint.coordinates;
	glm::vec2 texCoord3 = triangle[2].texturePoint.coordinates;

	// Interpolate texture coordinates along the edges
	std::vector<glm::vec2> leftColumnTexCoords = interpolateThreeElementValues(texCoord1, texCoord3, triangle[0].y - triangle[2].y + 1);
	std::vector<glm::vec2> rightColumnTexCoords = interpolateThreeElementValues(texCoord1, texCoord2, triangle[0].y - triangle[1].y + 1);

	// Loop through each row in the triangle
	for (int y = triangle[0].y; y >= triangle[2].y; y--) {
		// Get the interpolated texture coordinates for this row
		glm::vec2 texCoordLeft = leftColumnTexCoords[triangle[0].y - y];
		glm::vec2 texCoordRight = rightColumnTexCoords[triangle[0].y - y];

		// Interpolate texture coordinates along the row
		std::vector<glm::vec2> rowTexCoords = interpolateThreeElementValues(texCoordLeft, texCoordRight, triangle[1].x - triangle[0].x + 1);

		// Loop through each pixel in the row
		for (int x = triangle[0].x; x <= triangle[1].x; x++) {
			// Get the interpolated texture coordinates for this pixel
			glm::vec2 texCoord = rowTexCoords[x - triangle[0].x];

			// Calculate the texture map pixel index values (convert floating-point texture coordinates to integers)
			int texMapX = static_cast<int>(texCoord.x * (texture.width - 1));
			int texMapY = static_cast<int>(texCoord.y * (texture.height - 1));

			// Calculate the index of the pixel in the texture map vector
			int texMapIndex = texMapY * texture.width + texMapX;

			// Get the color from the texture map for this pixel
			uint32_t texColor = texture.pixels[texMapIndex];

			// Set the pixel's color in the window
			window.setPixelColour(x, y, texColor);
		}
	}

	// Draw the white stroked triangle over the filled triangle
	Colour white(255, 255, 255);
	drawLine(window, triangle[0], triangle[1], white);
	drawLine(window, triangle[1], triangle[2], white);
	drawLine(window, triangle[2], triangle[0], white);
}

#include <iostream>
#include <SDL2/SDL.h>
#include <CanvasPoint.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <vector>
#include <CanvasTriangle.h>
#include <TextureMap.h>

// ... (TexturePoint and CanvasPoint classes and other code remain unchanged)

void drawFilledTriangle(DrawingWindow& window, CanvasTriangle triangle, Colour color, TextureMap& texture) {
    // Sort vertices based on their y-coordinate (top to bottom)
    if (triangle[1].y < triangle[0].y) std::swap(triangle[0], triangle[1]);
    if (triangle[2].y < triangle[0].y) std::swap(triangle[0], triangle[2]);
    if (triangle[2].y < triangle[1].y) std::swap(triangle[1], triangle[2]);

    // Calculate the texture coordinates for each vertex
    TexturePoint texCoord1 = triangle[0].texturePoint;
    TexturePoint texCoord2 = triangle[1].texturePoint;
    TexturePoint texCoord3 = triangle[2].texturePoint;

    // Interpolate texture coordinates along the edges
    std::vector<TexturePoint> leftColumnTexCoords = interpolateThreeElementValues(texCoord1, texCoord3, triangle[0].y - triangle[2].y + 1);
    std::vector<TexturePoint> rightColumnTexCoords = interpolateThreeElementValues(texCoord1, texCoord2, triangle[0].y - triangle[1].y + 1);

    // Loop through each row in the triangle
    for (int y = triangle[0].y; y >= triangle[2].y; y--) {
        // Get the interpolated texture coordinates for this row
        TexturePoint texCoordLeft = leftColumnTexCoords[triangle[0].y - y];
        TexturePoint texCoordRight = rightColumnTexCoords[triangle[0].y - y];

        // Interpolate texture coordinates along the row
        std::vector<TexturePoint> rowTexCoords = interpolateThreeElementValues(texCoordLeft, texCoordRight, triangle[1].x - triangle[0].x + 1);

        // Loop through each pixel in the row
        for (int x = triangle[0].x; x <= triangle[1].x; x++) {
            // Get the interpolated texture coordinates for this pixel
            TexturePoint texCoord = rowTexCoords[x - triangle[0].x];

            // Calculate the texture map pixel index values (convert floating-point texture coordinates to integers)
            int texMapX = static_cast<int>(texCoord.x * (texture.width - 1));
            int texMapY = static_cast<int>(texCoord.y * (texture.height - 1));

            // Calculate the index of the pixel in the texture map vector
            int texMapIndex = texMapY * texture.width + texMapX;

            // Get the color from the texture map for this pixel
            uint32_t texColor = texture.pixels[texMapIndex];

            // Set the pixel's color in the window
            window.setPixelColour(x, y, texColor);
        }
    }

    // Draw the white stroked triangle over the filled triangle
    Colour white(255, 255, 255);
    drawLine(window, triangle[0], triangle[1], white);
    drawLine(window, triangle[1], triangle[2], white);
    drawLine(window, triangle[2], triangle[0], white);
}

void drawTextureTriangle(DrawingWindow& window, CanvasTriangle triangle, Colour color, TextureMap& texture) {
	// Sort vertices based on their y-coordinate (top to bottom)
	if (triangle[1].y < triangle[0].y) std::swap(triangle[0], triangle[1]);
	if (triangle[2].y < triangle[0].y) std::swap(triangle[0], triangle[2]);
	if (triangle[2].y < triangle[1].y) std::swap(triangle[1], triangle[2]);

	// Calculate the texture coordinates for each vertex
	TexturePoint texCoord1 = triangle[0].texturePoint;
	TexturePoint texCoord2 = triangle[1].texturePoint;
	TexturePoint texCoord3 = triangle[2].texturePoint;

	// Interpolate texture coordinates along the edges
	std::vector<TexturePoint> leftColumnTexCoords = interpolateThreeElementValues(texCoord1, texCoord3, triangle[0].y - triangle[2].y + 1);
	std::vector<TexturePoint> rightColumnTexCoords = interpolateThreeElementValues(texCoord1, texCoord2, triangle[0].y - triangle[1].y + 1);

	// Loop through each row in the triangle
	for (int y = triangle[0].y; y >= triangle[2].y; y--) {
		// Get the interpolated texture coordinates for this row
		TexturePoint texCoordLeft = leftColumnTexCoords[triangle[0].y - y];
		TexturePoint texCoordRight = rightColumnTexCoords[triangle[0].y - y];

		// Interpolate texture coordinates along the row
		std::vector<TexturePoint> rowTexCoords = interpolateThreeElementValues(texCoordLeft, texCoordRight, triangle[1].x - triangle[0].x + 1);

		// Loop through each pixel in the row
		for (int x = triangle[0].x; x <= triangle[1].x; x++) {
			// Get the interpolated texture coordinates for this pixel
			TexturePoint texCoord = rowTexCoords[x - triangle[0].x];

			// Calculate the texture map pixel index values (convert floating-point texture coordinates to integers)
			int texMapX = static_cast<int>(texCoord.x * (texture.width - 1));
			int texMapY = static_cast<int>(texCoord.y * (texture.height - 1));

			// Calculate the index of the pixel in the texture map vector
			int texMapIndex = texMapY * texture.width + texMapX;

			// Get the color from the texture map for this pixel
			uint32_t texColor = texture.pixels[texMapIndex];

			// Set the pixel's color in the window
			window.setPixelColour(x, y, texColor);
		}
	}

	// Draw the white stroked triangle over the filled triangle
	Colour white(255, 255, 255);
	drawLine(window, triangle[0], triangle[1], white);
	drawLine(window, triangle[1], triangle[2], white);
	drawLine(window, triangle[2], triangle[0], white);
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
		
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}