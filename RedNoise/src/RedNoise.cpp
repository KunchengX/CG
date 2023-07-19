#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>

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

std::vector<float> interpolateSingleFloats(float from, float to, int numberOfValue) {
	std::vector<float> result;
	float difference = (to - from) / float(numberOfValue - 1);
	for (int i = 0; i < numberOfValue; i++) result.push_back(from + (float)i * difference);
	return result;
}

void drawGrey(DrawingWindow& window) {
	window.clearPixels();
	std::vector<float> gradient = interpolateSingleFloats(255.0, 0.0, window.width);
	for (size_t y = 0; y < window.height; ++y) {
		for (size_t x = 0; x < window.width; ++x) {
			float red = gradient[x];
			float green = gradient[x];
			float blue = gradient[x];
			uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
			window.setPixelColour(x, y, colour);
		}
	}
}

std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValue) {
	std::vector<glm::vec3> result;
	glm::vec3 difference = (to - from) * (1 / (float(numberOfValue) - 1));
	for (float i = 0.0; i < numberOfValue; i++) {
		result.push_back(from + difference * float(i));
	}
	return result;
}

void drawRainbow(DrawingWindow& window) {
	window.clearPixels();
	glm::vec3 red(255, 0, 0);
	glm::vec3 blue(0, 0, 255);
	glm::vec3 green(0, 255, 0);
	glm::vec3 yellow(255, 255, 0);
	std::vector<glm::vec3> left = interpolateThreeElementValues(red, yellow, int(window.height));
	std::vector<glm::vec3> right = interpolateThreeElementValues(blue, green, int(window.width));
	for (size_t y = 0; y < window.height; y++) {
		std::vector<glm::vec3> row = interpolateThreeElementValues(left[y], right[y], int(window.width));
		for (size_t x = 0; x < window.width; x++) {
			float colourred = row[x].x;
			float colourgreen = row[x].y;
			float colourblue = row[x].z;
			uint32_t colour = (255 << 24) + (int(colourred) << 16) + (int(colourgreen) << 8) + int(colourblue);
			window.setPixelColour(x, y, colour);
		}
	}
}


void handleEvent(SDL_Event event, DrawingWindow& window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
	}
	else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char* argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		
		//draw(window);
		//drawGrey(window);
		drawRainbow(window);
		
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}