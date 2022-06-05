#define _CRT_SECURE_NO_DEPRECATE
#include<stdio.h>
#include<stdlib.h>
#include <iostream>
#include <omp.h>
#include <chrono>
#include <cmath>
//in our model sample image 
//PPM header is : 
//P6
//800 800 //Number of columns(width) Number of rows (Height)
//255  //Max channel value

typedef struct {
	unsigned char red, green, blue;
} PPMPixel;

typedef struct {
	int x, y;
	PPMPixel* data;
} PPMImage;

#define RGB_COMPONENT_COLOR 255

static PPMImage* readPPM(const char* filename)
{
	char buff[16];
	PPMImage* img;
	FILE* fp;
	int c, rgb_comp_color;
	//open PPM file for reading
	fp = fopen(filename, "rb");//reading a binary file
	if (!fp) {
		fprintf(stderr, "Unable to open file '%s'\n", filename);
		exit(1);
	}

	//read image format
	if (!fgets(buff, sizeof(buff), fp)) {
		perror(filename);
		exit(1);
	}

	//check the image format can be P3 or P6. P3:data is in ASCII format P6: data is in byte format
	if (buff[0] != 'P' || buff[1] != '6') {
		fprintf(stderr, "Invalid image format (must be 'P6')\n");
		exit(1);
	}

	//alloc memory form image
	img = (PPMImage*)malloc(sizeof(PPMImage));
	if (!img) {
		fprintf(stderr, "Unable to allocate memory\n");
		exit(1);
	}

	//check for comments
	c = getc(fp);
	while (c == '#') {
		while (getc(fp) != '\n');
		c = getc(fp);
	}

	ungetc(c, fp);//last character read was out back
	//read image size information
	if (fscanf(fp, "%d %d", &img->x, &img->y) != 2) {//if not reading widht and height of image means if there is no 2 values
		fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
		exit(1);
	}

	//read rgb component
	if (fscanf(fp, "%d", &rgb_comp_color) != 1) {
		fprintf(stderr, "Invalid rgb component (error loading '%s')\n", filename);
		exit(1);
	}

	//check rgb component depth
	if (rgb_comp_color != RGB_COMPONENT_COLOR) {
		fprintf(stderr, "'%s' does not have 8-bits components\n", filename);
		exit(1);
	}

	while (fgetc(fp) != '\n');
	//memory allocation for pixel data for all pixels
	img->data = (PPMPixel*)malloc(img->x * img->y * sizeof(PPMPixel));

	if (!img) {
		fprintf(stderr, "Unable to allocate memory\n");
		exit(1);
	}

	//read pixel data from file
	if (fread(img->data, 3 * img->x, img->y, fp) != img->y) { //3 channels, RGB  //size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
		/*ptr − This is the pointer to a block of memory with a minimum size of size*nmemb bytes.
		  size − This is the size in bytes of each element to be read.
		  nmemb − This is the number of elements, each one with a size of size bytes.
		  stream − This is the pointer to a FILE object that specifies an input stream.
		*/
		fprintf(stderr, "Error loading image '%s'\n", filename);
		exit(1);
	}

	fclose(fp);
	return img;
}
void writePPM(const char* filename, PPMImage* img)
{
	FILE* fp;
	//open file for output
	fp = fopen(filename, "wb");//writing in binary format
	if (!fp) {
		fprintf(stderr, "Unable to open file '%s'\n", filename);
		exit(1);
	}

	//write the header file
	//image format
	fprintf(fp, "P6\n");

	//image size
	fprintf(fp, "%d %d\n", img->x, img->y);

	// rgb component depth
	fprintf(fp, "%d\n", RGB_COMPONENT_COLOR);

	// pixel data
	fwrite(img->data, 3 * img->x, img->y, fp);
	fclose(fp);
}

void changeColorPPMSerial(PPMImage* img)//Negating image
{
	if (img) {
		for (int i = 0; i < img->x * img->y; i++) {
			float newColor = img->data[i].red * 0.21 + img->data[i].green * 0.72 + img->data[i].blue * 0.07;
			img->data[i].red = newColor;
			img->data[i].green = newColor;
			img->data[i].blue = newColor;
		}
	}
}

void changeColorPPMParallel(PPMImage* img)//Negating image
{
	if (img) {
#pragma omp parallel for
		for (int i = 0; i < img->x * img->y; i++) {
			float newColor = img->data[i].red * 0.21 + img->data[i].green * 0.72 + img->data[i].blue * 0.07;
			img->data[i].red = newColor;
			img->data[i].green = newColor;
			img->data[i].blue = newColor;
		}
	}
}

PPMPixel calculateSoberPixel(PPMImage* img, int i, int j) {
	PPMPixel p1, p2, p3, p4, p6, p7, p8, p9;
	p1 = img->data[(i - 1) * img->x + (j - 1)];
	p2 = img->data[(i - 1) * img->x + j];
	p3 = img->data[(i - 1) * img->x + (j + 1)];
	p4 = img->data[i * img->x + (j - 1)];
	p6 = img->data[i * img->x + (j + 1)];
	p7 = img->data[(i + 1) * img->x + (j - 1)];
	p8 = img->data[(i + 1) * img->x + j];
	p9 = img->data[(i + 1) * img->x + (j + 1)];

	int x = (p1.red + (p2.red + p2.red) + p3.red - p7.red - (p8.red + p8.red) - p9.red);
	int y = (p3.red + (p6.red + p6.red) + p9.red - p1.red - (p4.red + p4.red) - p7.red);
	int value = (int)std::sqrt(x * x + y * y);

	if (value < 0) value = 0;
	if (value > 255) value = 255;
	return PPMPixel{ (unsigned char)value,(unsigned char)value,(unsigned char)value };
}

void applySobelFilterSerial(PPMImage* img)
{
	PPMPixel* data = (PPMPixel*)malloc(img->x * img->y * sizeof(PPMPixel));
	if (img) {
		for (int i = 0; i < img->y; i++) {
			for (int j = 0; j < img->x; j++) {
				if (i == 0 || j == 0 || i == img->x - 1 || j == img->y - 1) {
					data[i * img->x + j] = PPMPixel{ 0,0,0 };
					continue;
				}
				data[i * img->x + j] = calculateSoberPixel(img, i, j);
			}
		}
		img->data = data;
	}
}

void applySobelFilterParallel(PPMImage* img)
{
	PPMPixel* data = (PPMPixel*)malloc(img->x * img->y * sizeof(PPMPixel));
	if (img) {
#pragma omp parallel for
		for (int i = 0; i < img->y; i++) {
			for (int j = 0; j < img->x; j++) {
				if (i == 0 || j == 0 || i == img->x - 1 || j == img->y - 1) {
					data[i * img->x + j] = PPMPixel{ 0,0,0 };
					continue;
				}
				data[i * img->x + j] = calculateSoberPixel(img, i, j);
			}
		}
		img->data = data;
	}
}



void serial() {
	PPMImage* image;
	image = readPPM("sample.ppm");
	auto start = std::chrono::high_resolution_clock::now();
	changeColorPPMSerial(image);
	applySobelFilterSerial(image);
	auto end = std::chrono::high_resolution_clock::now();
	std::cout << "time passed as microseconds in serial version is " <<
		std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;

	writePPM("model_serial.ppm", image);
}

void parallel() {
	PPMImage* image;
	image = readPPM("sample.ppm");
	auto start = std::chrono::high_resolution_clock::now();
	changeColorPPMParallel(image);
	applySobelFilterParallel(image);
	auto end = std::chrono::high_resolution_clock::now();
	std::cout << "time passed as microseconds in parallel version is " <<
		std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
	writePPM("model_parallel.ppm", image);
}

int main() {
	serial();
	omp_set_num_threads(8);
	parallel();
	printf("Press any key...");
	getchar();
}