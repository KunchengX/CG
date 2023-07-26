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
#include <map>

#define WIDTH 320
#define HEIGHT 240

float depthBuffer[HEIGHT][WIDTH];

glm::vec3 cameraPosition(0.0, 0.0, 4.0);
glm::mat3 cameraOrientation(1, 0, 0, 0, 1, 0, 0, 0, 1);
glm::mat3 Rotation(1, 0, 0, 0, 1, 0, 0, 0, 1);
glm::vec3 lightposition = glm::vec3(0.0, 0.5, 0.5);
glm::vec3 SpherePoint(0, 0, 0);

int shadingfactor = 1;
std::vector<glm::vec3> lightpoints;

float xyzdistance;
float x = 0.0;
float y = 0.0;
glm::vec3 newCameraPosition = cameraPosition;


int imageSequence = 81;
float rotateSphereY = 0.0;

float focalLength = 2.0;

void clearDepthBuffer() {
	for (int y = 0; y < HEIGHT; y++)
		for (int x = 0; x < WIDTH; x++)
			depthBuffer[y][x] = INT32_MIN;
}

void draw(DrawingWindow& window) {
	window.clearPixels();
	// draw random red noise
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

glm::vec3 NormalCalculator(glm::vec3 vertex, const std::vector<ModelTriangle>& modelTriangles) {
	float index = 0.0;
	glm::vec3 vertexNormal;
	for (ModelTriangle triangle : modelTriangles) {
		if (triangle.vertices[0] == vertex || triangle.vertices[1] == vertex || triangle.vertices[2] == vertex) {
			index++;
			vertexNormal += triangle.normal;
		}
	}
	return glm::normalize(vertexNormal / index);
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

	// Generate the grayscale values using interpolation
	std::vector<float> gradientValues = interpolateSingleFloats(startValue, endValue, WIDTH);

	// Loop through each pixel in the window
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
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

	// Calculate the colors for the left-most and right-most columns
	std::vector<glm::vec3> leftColumnColours = interpolateThreeElementValues(topLeft, bottomLeft, HEIGHT);
	std::vector<glm::vec3> rightColumnColours = interpolateThreeElementValues(topRight, bottomRight, HEIGHT);

	// Loop through each row in the window
	for (int y = 0; y < HEIGHT; y++) {
		// Interpolate between the left and right column colors for this row
		std::vector<glm::vec3> rowColours = interpolateThreeElementValues(leftColumnColours[y], rightColumnColours[y], WIDTH);

		// Loop through each pixel in the row
		for (int x = 0; x < WIDTH; x++) {
			// Get the interpolated color for this pixel
			glm::vec3 colour = rowColours[x];

			// Create a 32-bit integer color (RGBA format: 0xAARRGGBB)
			uint32_t pixelColour = (255 << 24) + (static_cast<int>(colour.r) << 16) + (static_cast<int>(colour.g) << 8) + static_cast<int>(colour.b);

			// Set the pixel's color in the window
			window.setPixelColour(x, y, pixelColour);
		}
	}

	// Render the frame to display the color spectrum
	window.renderFrame();
}

void drawLine(DrawingWindow& window, const CanvasPoint from, const CanvasPoint to, Colour colour) {
	uint32_t pixelColour = (255 << 24) + (colour.red << 16) + (colour.green << 8) + (colour.blue);
	if (x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT) {
		for (float i = 0.0; i < fmax(fabs(to.x - from.x), fabs(to.y - from.y)); i++) {
			float x = from.x + (((to.x - from.x) / fmax(fabs(to.x - from.x), fabs(to.y - from.y))) * i);
			float y = from.y + (((to.y - from.y) / fmax(fabs(to.x - from.x), fabs(to.y - from.y))) * i);
			float z = from.depth + ((to.depth - from.depth) / fmax(fabs(to.x - from.x), fabs(to.y - from.y)) * i);
			if (depthBuffer[int(round(y))][int(round(x))] <= z) {
				depthBuffer[int(round(y))][int(round(x))] = z;
				window.setPixelColour(round(x), round(y), pixelColour);
			}
		}
	}
}

void drawStrokedTriangle(DrawingWindow& window, CanvasTriangle triangle, Colour color) {
	drawLine(window, triangle[0], triangle[1], color);
	drawLine(window, triangle[1], triangle[2], color);
	drawLine(window, triangle[2], triangle[0], color);
}

void triangleRasteriser(DrawingWindow& window, CanvasPoint v1, CanvasPoint v2, CanvasPoint v3, Colour colour) {
	CanvasPoint newV1 = v1;
	CanvasPoint newV2 = v1;
	for (float i = 1.00; i <= fabs(v2.y - v1.y); i++) {
		newV1.x = v1.x + (v2.x - v1.x) * (i / fabs(v2.y - v1.y));
		newV1.y -= judge(v1.y - v3.y);
		newV1.depth = v1.depth + (v2.depth - v1.depth) * (i / fabs(v2.y - v1.y));
		newV2.x = v1.x + (v3.x - v1.x) * (i / fabs(v2.y - v1.y));
		newV2.y -= judge(v1.y - v3.y);
		newV2.depth = v1.depth + (v3.depth - v1.depth) * (i / fabs(v2.y - v1.y));
		drawLine(window, newV1, newV2, colour);
	}
}

void drawFilledTriangle(DrawingWindow& window, CanvasTriangle triangle, Colour colour) {

	if (triangle[1].y < triangle[0].y) std::swap(triangle[0], triangle[1]);
	if (triangle[2].y < triangle[0].y) std::swap(triangle[0], triangle[2]);
	if (triangle[2].y < triangle[1].y) std::swap(triangle[1], triangle[2]);

	CanvasPoint top = triangle[0];
	CanvasPoint mid = triangle[1];
	CanvasPoint bot = triangle[2];

	CanvasPoint extra;

	double R = (bot.x - top.x) / (bot.y - top.y);
	float EX;
	float EY;
	float EDepth;
	if (R >= 0) {
		EX = top.x + ((mid.y - top.y) * fabs(R));
	}
	else {
		EX = top.x - ((mid.y - top.y) * fabs(R));
	}
	EY = mid.y;
	float u = (-(EX - triangle.v1().x) * (triangle.v2().y - triangle.v1().y) + (EY - triangle.v1().y) * (triangle.v2().x - triangle.v1().x)) / (-(triangle.v0().x - triangle.v1().x) * (triangle.v2().y - triangle.v1().y) + (triangle.v0().y - triangle.v1().y) * (triangle.v2().x - triangle.v1().x));
	float v = (-(EX - triangle.v2().x) * (triangle.v0().y - triangle.v2().y) + (EY - triangle.v2().y) * (triangle.v0().x - triangle.v2().x)) / (-(triangle.v1().x - triangle.v2().x) * (triangle.v0().y - triangle.v2().y) + (triangle.v1().y - triangle.v2().y) * (triangle.v0().x - triangle.v2().x));
	float w = 1 - u - v;
	EDepth = u * triangle.v0().depth + v * triangle.v1().depth + w * triangle.v2().depth;
	extra = CanvasPoint(EX, mid.y, EDepth);
	triangleRasteriser(window, top, mid, extra, colour);
	triangleRasteriser(window, bot, mid, extra, colour);
}

TextureMap getTextureMap(const std::string& image) {
	TextureMap textureMap = TextureMap(image);
	//std::cout << "width =" << textureMap.width << "height =" << textureMap.height << std::endl;
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
			uint32_t pixelColour = textureMapper(textureMap, triangle, CanvasPoint(x, y));
			window.setPixelColour(x, y, pixelColour);
		}
	}
}

void drawTextureTriangle(DrawingWindow& window, const TextureMap& textureMap, CanvasTriangle triangle) {
	// draw white stroked triangle
	drawStrokedTriangle(window, triangle, Colour(255, 255, 255));

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

/*空间四点确定球心坐标(克莱姆法则)*/
void get_xyz(double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3, double x4, double y4, double z4) {
	double x, y, z;
	double a11, a12, a13, a21, a22, a23, a31, a32, a33, b1, b2, b3, d, d1, d2, d3;
	a11 = 2 * (x2 - x1); a12 = 2 * (y2 - y1); a13 = 2 * (z2 - z1);
	a21 = 2 * (x3 - x2); a22 = 2 * (y3 - y2); a23 = 2 * (z3 - z2);
	a31 = 2 * (x4 - x3); a32 = 2 * (y4 - y3); a33 = 2 * (z4 - z3);
	b1 = x2 * x2 - x1 * x1 + y2 * y2 - y1 * y1 + z2 * z2 - z1 * z1;
	b2 = x3 * x3 - x2 * x2 + y3 * y3 - y2 * y2 + z3 * z3 - z2 * z2;
	b3 = x4 * x4 - x3 * x3 + y4 * y4 - y3 * y3 + z4 * z4 - z3 * z3;
	d = a11 * a22 * a33 + a12 * a23 * a31 + a13 * a21 * a32 - a11 * a23 * a32 - a12 * a21 * a33 - a13 * a22 * a31;
	d1 = b1 * a22 * a33 + a12 * a23 * b3 + a13 * b2 * a32 - b1 * a23 * a32 - a12 * b2 * a33 - a13 * a22 * b3;
	d2 = a11 * b2 * a33 + b1 * a23 * a31 + a13 * a21 * b3 - a11 * a23 * b3 - b1 * a21 * a33 - a13 * b2 * a31;
	d3 = a11 * a22 * b3 + a12 * b2 * a31 + b1 * a21 * a32 - a11 * b2 * a32 - a12 * a21 * b3 - b1 * a22 * a31;
	x = d1 / d;
	y = d2 / d;
	z = d3 / d;
	SpherePoint = glm::vec3(x, y, z);
}

Colour mtlConverter(double r, double g, double b) {
	Colour colour = Colour(int(r * 255), int(g * 255), int(b * 255));
	return colour;
}

std::map<std::string, Colour> mtlReader(const std::string& mtlFileName) {
	std::map<std::string, Colour> colours;
	std::string line;
	std::string colourName;
	std::ifstream File(mtlFileName);
	while (getline(File, line)) {
		std::vector<std::string> text = split(line, ' ');
		if (text[0] == "newmtl") {
			colourName = text[1];
		}
		else if (text[0] == "Kd") {
			colours[colourName] = mtlConverter(std::stod(text[1]), std::stod(text[2]), std::stod(text[3]));
		}
	}
	File.close();
	return colours;
}

std::map<std::string, std::string> mtlTextureReader(const std::string& mtlFileName) {
	std::map<std::string, std::string> textures;
	std::string textureName;
	std::string line;
	std::ifstream File(mtlFileName);
	while (getline(File, line)) {
		std::vector<std::string> text = split(line, ' ');
		if (text[0] == "map_Kd") {
			textureName = text[1];
			textures[textureName] = textureName;
		}
	}
	File.close();
	return textures;
}

std::vector<ModelTriangle> objReader(const std::string& objFileName, const std::string& mtlFileName, float scalingFactor) {
	std::vector<glm::vec3> vertices;
	std::vector<std::vector<std::string>> facets;
	std::vector<ModelTriangle> modelTriangles;
	std::vector<glm::vec2> texture;
	std::string line;
	std::string colourName;
	std::ifstream File(objFileName);
	while (getline(File, line)) {
		std::vector<std::string> text = split(line, ' ');
		if (text[0] == "usemtl") {
			colourName = text[1];
		}
		else if (text[0] == "v") {
			glm::vec3 v = glm::vec3(std::stod(text[1]), std::stod(text[2]), std::stod(text[3]));
			vertices.push_back(v);
		}
		else if (text[0] == "f") {
			// text[1] = 14/3 ------> 14: obj point 3: texture point
			std::vector<std::string> f{ text[1], text[2], text[3], colourName };
			facets.push_back(f);
		}
		else if (text[0] == "vt") {
			glm::vec2 vt = glm::vec2(std::stod(text[1]), std::stod(text[2]));
			//std::cout << glm::to_string(vt) << std::endl;
			texture.push_back(vt);
		}
	}
	File.close();
	std::map<std::string, Colour> colourMap = mtlReader(mtlFileName);
	//std::map<std::string, std::string> textureMap = mtlTextureReader(mtlFileName);

	// 目前只有一个面需要着色
	//TextureMap texture = getTextureMap(textureMap["texture.ppm"]);
	for (std::vector<std::string> i : facets) {
		glm::vec3 v0 = vertices[std::stoi(i[0]) - 1];
		glm::vec3 v1 = vertices[std::stoi(i[1]) - 1];
		glm::vec3 v2 = vertices[std::stoi(i[2]) - 1];
		glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
		Colour colour = colourMap[i[3]];
		ModelTriangle modelTriangle = ModelTriangle(v0 * scalingFactor, v1 * scalingFactor, v2 * scalingFactor, colour);
		modelTriangle.normal = normal;
		modelTriangle.colour.name = i[3];

		std::vector<std::string> v0String = split(i[0], '/');
		std::vector<std::string> v1String = split(i[1], '/');
		std::vector<std::string> v2String = split(i[2], '/');
		if (atoi(v0String[1].c_str()) != 0 && atoi(v1String[1].c_str()) != 0 && atoi(v2String[1].c_str()) != 0) {
			modelTriangle.texturePoints = { 
				TexturePoint(texture[atoi(v0String[1].c_str()) - 1][0], texture[atoi(v0String[1].c_str()) - 1][1]),
				TexturePoint(texture[atoi(v1String[1].c_str()) - 1][0], texture[atoi(v1String[1].c_str()) - 1][1]),
				TexturePoint(texture[atoi(v2String[1].c_str()) - 1][0], texture[atoi(v2String[1].c_str()) - 1][1]) 
			};
		}
		modelTriangles.push_back(modelTriangle);
		//std::cout << triangle.texturePoints[0] << std::endl;
	}

	return modelTriangles;
}

// 没有vt
std::vector<ModelTriangle> cubeReader(const std::string& objFileName, float scalingFactor) {
	std::vector<glm::vec3> vertices;
	std::vector<std::vector<std::string>> facets;
	std::vector<ModelTriangle> modelTriangles;
	std::vector<glm::vec2> texture;
	std::string line;
	std::string colourName;
	std::ifstream File(objFileName);
	while (getline(File, line)) {
		std::vector<std::string> text = split(line, ' ');
		if (text[0] == "usemtl") {
			colourName = text[1];
		}
		else if (text[0] == "v") {
			glm::vec3 v = glm::vec3(std::stod(text[1]), std::stod(text[2]), std::stod(text[3]));
			vertices.push_back(v);
		}
		else if (text[0] == "f") {
			std::vector<std::string> f{ text[1], text[2], text[3], colourName };
			facets.push_back(f);
		}
	}
	File.close();
	// 只有白色
	
	Colour white = Colour(255, 255, 255);
	for (std::vector<std::string> i : facets) {
		glm::vec3 v0 = vertices[std::stoi(i[0]) - 1];
		glm::vec3 v1 = vertices[std::stoi(i[1]) - 1];
		glm::vec3 v2 = vertices[std::stoi(i[2]) - 1];
		glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
		Colour colour = white;
		ModelTriangle modelTriangle = ModelTriangle(v0 * scalingFactor, v1 * scalingFactor, v2 * scalingFactor, colour);
		modelTriangle.normal = normal;
		modelTriangle.colour.name = i[3];

		std::vector<std::string> v0String = split(i[0], '/');
		std::vector<std::string> v1String = split(i[1], '/');
		std::vector<std::string> v2String = split(i[2], '/');
		if (atoi(v0String[1].c_str()) != 0 && atoi(v1String[1].c_str()) != 0 && atoi(v2String[1].c_str()) != 0) {
			modelTriangle.texturePoints = {
				TexturePoint(texture[atoi(v0String[1].c_str()) - 1][0], texture[atoi(v0String[1].c_str()) - 1][1]),
				TexturePoint(texture[atoi(v1String[1].c_str()) - 1][0], texture[atoi(v1String[1].c_str()) - 1][1]),
				TexturePoint(texture[atoi(v2String[1].c_str()) - 1][0], texture[atoi(v2String[1].c_str()) - 1][1])
			};
		}
		modelTriangles.push_back(modelTriangle);
		//std::cout << triangle.texturePoints[0] << std::endl;
	}

	return modelTriangles;
}

std::vector<ModelTriangle> SphereReader(const std::string& objFile, float scalingFactor) {
	std::vector<glm::vec3> vertex;
	std::vector<std::vector<std::string>> facets;
	std::vector<ModelTriangle> modelTriangles;
	std::string myText;
	std::ifstream File(objFile);
	while (getline(File, myText)) {
		std::vector<std::string> text = split(myText, ' ');
		if (text[0] == "v") {
			glm::vec3 v = glm::vec3(std::stod(text[1]) * scalingFactor - 0.5, std::stod(text[2]) * scalingFactor - 1.1349, std::stod(text[3]) * scalingFactor);
			vertex.push_back(v);
		}
		else if (text[0] == "f") {
			std::vector<std::string> f{ text[1], text[2], text[3] };
			facets.push_back(f);
		}
	}

	File.close();
	for (std::vector<std::string> i : facets) {
		glm::vec3 v0 = vertex[std::stoi(i[0]) - 1];
		glm::vec3 v1 = vertex[std::stoi(i[1]) - 1];
		glm::vec3 v2 = vertex[std::stoi(i[2]) - 1];
		glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
		float red = 12.0;
		float green = 13.0;
		float blue = 14.0;
		Colour colour = Colour(int(red), int(green), int(blue));
		ModelTriangle triangle = ModelTriangle(v0, v1, v2, colour);
		triangle.normal = normal;
		modelTriangles.push_back(triangle);
	}
	get_xyz(vertex[0].x, vertex[0].y, vertex[0].z, vertex[10].x, vertex[10].y, vertex[10].z, vertex[20].x, vertex[20].y, vertex[20].z, vertex[30].x, vertex[30].y, vertex[30].z);
	xyzdistance = glm::distance(vertex[0], SpherePoint);
	//std::cout<<xyzdistance<<std::endl;
	//std::cout<< glm::to_string(SpherePoint)<<std::endl;
	return modelTriangles;
}

CanvasPoint getCanvasIntersectionPoint(glm::vec3 cameraPosition, glm::mat3 cameraOrientation, glm::vec3 vertexPosition, float focallength, float scalingFactor) {
	CanvasPoint twoDimensionalPoint;
	glm::vec3 cameraToVertex = { vertexPosition.x - cameraPosition.x, vertexPosition.y - cameraPosition.y, vertexPosition.z - cameraPosition.z };
	glm::vec3 cameraToVertexInCameraSpace = cameraToVertex * cameraOrientation;
	twoDimensionalPoint.x = round(focallength * (-scalingFactor) * cameraToVertexInCameraSpace.x / cameraToVertexInCameraSpace.z + float(WIDTH / 2));
	twoDimensionalPoint.y = round(focallength * scalingFactor * cameraToVertexInCameraSpace.y / cameraToVertexInCameraSpace.z + float(HEIGHT / 2));
	twoDimensionalPoint.depth = 1 / fabs(cameraToVertexInCameraSpace.z);
	return twoDimensionalPoint;
}

CanvasTriangle getCanvasIntersectionTriangle(glm::vec3 cameraPosition, glm::mat3 cameraOrientation, ModelTriangle triangle, float focallength, float scalingFactor) {
	CanvasPoint v0 = getCanvasIntersectionPoint(cameraPosition, cameraOrientation, triangle.vertices[0], focallength, scalingFactor);
	CanvasPoint v1 = getCanvasIntersectionPoint(cameraPosition, cameraOrientation, triangle.vertices[1], focallength, scalingFactor);
	CanvasPoint v2 = getCanvasIntersectionPoint(cameraPosition, cameraOrientation, triangle.vertices[2], focallength, scalingFactor);
	CanvasTriangle canvasTriangle = CanvasTriangle(v0, v1, v2);
	return canvasTriangle;
}

void PointCloud(DrawingWindow& window, const std::vector<ModelTriangle> triangles, glm::vec3 cameraPosition, glm::mat3 cameraOrientation, float focallength, float scalingFactor, const Colour& colour) {
	int red = colour.red;
	int green = colour.green;
	int blue = colour.blue;
	uint32_t pixelColour = (255 << 24) + (red << 16) + (green << 8) + (blue);
	for (ModelTriangle triangle : triangles) {
		CanvasTriangle canvasTriangle = getCanvasIntersectionTriangle(cameraPosition, cameraOrientation, triangle, focallength, scalingFactor);
		window.setPixelColour(canvasTriangle.v0().x, canvasTriangle.v0().y, pixelColour);
		window.setPixelColour(canvasTriangle.v1().x, canvasTriangle.v1().y, pixelColour);
		window.setPixelColour(canvasTriangle.v2().x, canvasTriangle.v2().y, pixelColour);
	}
}

void WireFrame(DrawingWindow& window, const std::vector<ModelTriangle> triangles, glm::vec3 cameraPosition, glm::mat3 cameraOrientation, float focallength, float scalingFactor, const Colour& colour) {
	for (ModelTriangle triangle : triangles) {
		CanvasTriangle canvasTriangle = getCanvasIntersectionTriangle(cameraPosition, cameraOrientation, triangle, focallength, scalingFactor);
		drawLine(window, canvasTriangle.v0(), canvasTriangle.v1(), triangle.colour);
		drawLine(window, canvasTriangle.v1(), canvasTriangle.v2(), triangle.colour);
		drawLine(window, canvasTriangle.v2(), canvasTriangle.v0(), triangle.colour);
	}
}

void Rasterised(DrawingWindow& window, const std::vector<ModelTriangle> triangles, glm::vec3 cameraPosition, glm::mat3 cameraOrientation, float focallength, float scalingFactor, const Colour& colour) {
	//std::cout << "\ntriangle.colour" << std::endl;
	for (ModelTriangle triangle : triangles) {
		CanvasTriangle canvasTriangle = getCanvasIntersectionTriangle(cameraPosition, cameraOrientation, triangle, focallength, scalingFactor);
		//std::cout << triangle.colour << std::endl;
		drawFilledTriangle(window, canvasTriangle, triangle.colour);
	}
}

glm::mat3 lookAt(glm::vec3 cameraPosition) {
	glm::vec3 vertical(0, 1, 0);
	glm::vec3 center(0, 0, 0);
	glm::vec3 forward = glm::normalize(cameraPosition - center);
	glm::vec3 right = glm::cross(vertical, forward);
	glm::vec3 up = glm::cross(forward, right);
	glm::mat3 newCameraOrientation = glm::mat3(right.x, right.y, right.z, up.x, up.y, up.z, forward.x, forward.y, forward.z);
	return newCameraOrientation;
}

void handleEvent(SDL_Event event, DrawingWindow& window) {
	auto modelTriangles = objReader("textured-cornell-box.obj", "textured-cornell-box.mtl", 0.35);
	//std::vector<ModelTriangle> modelTriangles;
	auto modelSphere = SphereReader("sphere.obj", 0.35);
	for (ModelTriangle triangle : modelSphere) {
		modelTriangles.push_back(triangle);
	}
	//auto modelTriangles = cubeReader("cube.obj", 0.35);
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) {
			std::cout << "LEFT" << std::endl;
			float radianx = 0.05;
			window.clearPixels();
			clearDepthBuffer();
			glm::mat3 cameraRotation = glm::mat3(cos(radianx), 0, -sin(radianx), 0, 1, 0, sin(radianx), 0, cos(radianx));
			Rotation = lookAt(cameraPosition);
			cameraPosition = cameraRotation * cameraPosition;
			float focalLength = 2.0;
			Rasterised(window, modelTriangles, cameraPosition, Rotation, focalLength, float(HEIGHT) * 2 / 3, Colour(255, 255, 255));
		}
		else if (event.key.keysym.sym == SDLK_RIGHT) {
			std::cout << "RIGHT" << std::endl;
			float radianx = 0.05;
			window.clearPixels();
			clearDepthBuffer();
			glm::mat3 cameraRotation = glm::mat3(cos(radianx), 0, sin(radianx), 0, 1, 0, -sin(radianx), 0, cos(radianx));
			Rotation = lookAt(cameraPosition);
			cameraPosition = cameraRotation * cameraPosition;
			float focalLength = 2.0;
			Rasterised(window, modelTriangles, cameraPosition, Rotation, focalLength, float(HEIGHT) * 2 / 3, Colour(255, 255, 255));
		}
		else if (event.key.keysym.sym == SDLK_UP) {
			std::cout << "UP" << std::endl;
			float radianx = 0.05;
			window.clearPixels();
			clearDepthBuffer();
			//x axis anticlockwise
			glm::mat3 cameraRotation = glm::mat3(1, 0, 0, 0, cos(radianx), sin(radianx), 0, -sin(radianx), cos(radianx));
			Rotation = lookAt(cameraPosition);
			Rotation = Rotation * cameraRotation;
			cameraPosition =  cameraRotation * cameraPosition;
			float focalLength = 2.0;
			Rasterised(window, modelTriangles, cameraPosition, Rotation, focalLength, float(HEIGHT)*2/3, Colour(255, 255, 255));
		}
		else if (event.key.keysym.sym == SDLK_DOWN) {
			std::cout << "DOWN" << std::endl;
			float radianx = 0.05;
			window.clearPixels();
			clearDepthBuffer();
			glm::mat3 cameraRotation = glm::mat3(1, 0, 0, 0, cos(radianx), -sin(radianx), 0, sin(radianx), cos(radianx));
			Rotation = lookAt(cameraPosition);
			Rotation = Rotation * cameraRotation;
			cameraPosition = cameraRotation * cameraPosition;
			float focalLength = 2.0;
			Rasterised(window, modelTriangles, cameraPosition, Rotation, focalLength, float(HEIGHT) * 2 / 3, Colour(255, 255, 255));
		}
		else if (event.key.keysym.sym == SDLK_u) {	// Test Week 3 Task 3 draw stroked triangle
			// Generate three random points for the triangle
			CanvasPoint p1(rand() % window.width, rand() % window.height);
			CanvasPoint p2(rand() % window.width, rand() % window.height);
			CanvasPoint p3(rand() % window.width, rand() % window.height);

			// Generate a random color for the triangle
			Colour color(rand() % 256, rand() % 256, rand() % 256);

			// Create the triangle and draw it
			CanvasTriangle triangle(p1, p2, p3);
			drawStrokedTriangle(window, triangle, color);
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

			CanvasPoint p1, p2, p3;
			p1 = CanvasPoint(160, 10);
			p2 = CanvasPoint(300, 230);
			p3 = CanvasPoint(10, 150);
			p1.texturePoint.x = 195; p1.texturePoint.y = 5;
			p2.texturePoint.x = 395; p2.texturePoint.y = 380;
			p3.texturePoint.x = 65; p3.texturePoint.y = 330;
			CanvasTriangle triangle = CanvasTriangle(p1, p2, p3);
			drawTextureTriangle(window, textureMap, triangle);
		}
		else if (event.key.keysym.sym == SDLK_j) {
			// Test Week 4 Task 6 print cloud
			//PointCloud(window, modelTriangles, cameraPosition, cameraOrientation, focalLength, float(HEIGHT) * 2 / 3, Colour(255, 255, 255));
			
			// Test Week 4 Task 7 wireframe render
			//WireFrame(window, modelTriangles, cameraPosition, cameraOrientation, focalLength, float(HEIGHT)*2/3, Colour(255, 255, 255));
			
			// Test Week 4 task 8 rasteried render
			Rasterised(window, modelTriangles, cameraPosition, cameraOrientation, focalLength, float(HEIGHT)*2/3, Colour(255, 255, 255));
		}
		else if (event.key.keysym.sym == SDLK_w) {
			window.clearPixels();
			clearDepthBuffer();
			cameraPosition.y -= 0.05;
			Rasterised(window, modelTriangles, cameraPosition, cameraOrientation, focalLength, float(HEIGHT) * 2 / 3, Colour(255, 255, 255));
		}
		else if (event.key.keysym.sym == SDLK_s) {
			window.clearPixels();
			clearDepthBuffer();
			cameraPosition.y += 0.05;
			Rasterised(window, modelTriangles, cameraPosition, cameraOrientation, focalLength, float(HEIGHT) * 2 / 3, Colour(255, 255, 255));
		}
		else if (event.key.keysym.sym == SDLK_a) {
			window.clearPixels();
			clearDepthBuffer();
			cameraPosition.x += 0.05;
			Rasterised(window, modelTriangles, cameraPosition, cameraOrientation, focalLength, float(HEIGHT) * 2 / 3, Colour(255, 255, 255));
		}
		else if (event.key.keysym.sym == SDLK_d) {
			window.clearPixels();
			clearDepthBuffer();
			cameraPosition.x -= 0.05;
			Rasterised(window, modelTriangles, cameraPosition, cameraOrientation, focalLength, float(HEIGHT) * 2 / 3, Colour(255, 255, 255));
		}
		else if (event.key.keysym.sym == SDLK_q) {
			//left
			window.clearPixels();
			clearDepthBuffer();
			x += 1.0;
			float radianx = glm::radians(x);
			glm::mat3 cameraRotation = glm::mat3(cos(radianx), 0, sin(radianx), 0, 1, 0, -sin(radianx), 0, cos(radianx));
			cameraPosition = cameraPosition * cameraRotation;
			Rotation = Rotation * cameraRotation;
			Rasterised(window, modelTriangles, cameraPosition, Rotation, focalLength, float(HEIGHT) * 2 / 3, Colour(255, 255, 255));
		}
		else if (event.key.keysym.sym == SDLK_e) {
			//right
			window.clearPixels();
			clearDepthBuffer();
			x -= 1.0;
			float radianx = glm::radians(x);
			glm::mat3 cameraRotation = glm::mat3(cos(radianx), 0, -sin(radianx), 0, 1, 0, sin(radianx), 0, cos(radianx));
			cameraPosition = cameraPosition * cameraRotation;
			Rotation = Rotation * cameraRotation;
			Rasterised(window, modelTriangles, cameraPosition, Rotation, focalLength, float(HEIGHT)*2/3, Colour(255, 255, 255)); 
		}
		else if (event.key.keysym.sym == SDLK_z) {
			//downwards
			window.clearPixels();
			clearDepthBuffer();
			y += 0.3;
			float radiany = glm::radians(y);
			glm::mat3 cameraRotation = glm::mat3(1, 0, 0, 0, cos(radiany), sin(radiany), 0, -sin(radiany), cos(radiany));
			cameraPosition = cameraPosition * cameraRotation;
			Rotation = Rotation * cameraRotation;
			Rasterised(window, modelTriangles, cameraPosition, Rotation, focalLength, float(HEIGHT)*2/3, Colour(255, 255, 255));
		}
		else if (event.key.keysym.sym == SDLK_x) {
			//upwards
			window.clearPixels();
			clearDepthBuffer();
			y -= 0.3;
			float radiany = glm::radians(y);
			glm::mat3 cameraRotation = glm::mat3(1, 0, 0, 0, cos(radiany), -sin(radiany), 0, sin(radiany), cos(radiany));
			cameraPosition = cameraPosition * cameraRotation;
			Rotation = Rotation * cameraRotation;
			Rasterised(window, modelTriangles, cameraPosition, Rotation, focalLength, float(HEIGHT)*2/3, Colour(255, 255, 255));
		}
		else if (event.key.keysym.sym == SDLK_0) {
			Rasterised(window, modelTriangles, cameraPosition, cameraOrientation, focalLength, float(HEIGHT) * 2 / 3, Colour(255, 255, 255));
		}
		else if (event.key.keysym.sym == SDLK_c) {
			clearDepthBuffer();
			window.clearPixels();
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

	// Test Week 4 Task 2
	//std::string filename = "cornell-Box.obj";
	//float scalingFactor = 0.35;

	//std::vector<ModelTriangle> triangles = loadOBJGeometry(filename, scalingFactor);

	//// Print out the loaded triangles
	//for (const ModelTriangle& triangle : triangles) {
	//	std::cout << "Triangle: " << triangle << std::endl;
	//}



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

		////Orbit
		//auto modelTriangles = objReader("cornell-box.obj", "cornell-box.mtl", 0.35);
		//float radianx = 0.01;
		//window.clearPixels();
		//clearDepthBuffer();
		// //y axis anticlockwise
		////glm::mat3 cameraRotation = glm::mat3(cos(radianx), 0, sin(radianx), 0, 1, 0, -sin(radianx), 0, cos(radianx));
		// //y axis clockwise
		//glm::mat3 cameraRotation = glm::mat3(cos(radianx), 0, -sin(radianx), 0, 1, 0, sin(radianx), 0, cos(radianx));
		// //x axis anticlockwise
		////glm::mat3 cameraRotation = glm::mat3(1, 0, 0, 0, cos(radianx), -sin(radianx), 0, sin(radianx), cos(radianx));
		// //use only in y axis rotation
		//Rotation = lookAt(cameraPosition);
		// //use only in y axis rotation
		//Rotation = Rotation * cameraRotation;
		//cameraPosition =  cameraRotation * cameraPosition;
		//float focalLength = 2.0;
		//Rasterised(window, modelTriangles, cameraPosition, Rotation, focalLength, float(HEIGHT)*2/3, Colour(255, 255, 255));



		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}