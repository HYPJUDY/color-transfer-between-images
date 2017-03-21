/* Histogram Equalization on Color Images
*  HYPJUDY 2017/03/18
*/

#include <iostream>
#include <vector>
#include <string>
#include "CImg.h"
using namespace std;
using namespace cimg_library;

CImg<float> RGB2Lab(CImg<unsigned int> RGBimg) {
	if (RGBimg._spectrum != 3) { // Number of colour channels
		cerr << "RGB2Lab(): Instance is not a RGB image." << endl;
	}
	CImg<float> Labimg(RGBimg._width, RGBimg._height, 1, 3, 0); // w,h,depth,spectrum,initVal
	float R, G, B, L, M, S, l, alpha, beta;
	cimg_forXY(RGBimg, x, y) {
		R = cimg::max(1.0 * RGBimg(x, y, 0) / 255, 1.0 / 255); // must be 1.0f, or 1/255 will be zero!!
		G = cimg::max(1.0 * RGBimg(x, y, 1) / 255, 1.0 / 255);
		B = cimg::max(1.0 * RGBimg(x, y, 2) / 255, 1.0 / 255);

		// RGB -> LMS
		L = 0.3811f*R + 0.5783f*G + 0.0402f*B;
		M = 0.1967f*R + 0.7244f*G + 0.0782f*B;
		S = 0.0241f*R + 0.1288f*G + 0.8444f*B;
		// Convert the data to logarithmic space
		L = log10(L); // log(x), x > 0
		M = log10(M);
		S = log10(S);
		// LMS -> Lab
		l = 1.0 / sqrt(3)*L + 1.0 / sqrt(3)*M + 1.0 / sqrt(3)*S;
		alpha = 1.0 / sqrt(6)*L + 1.0 / sqrt(6)*M - 2 / sqrt(6)*S;
		beta = 1.0 / sqrt(2)*L - 1.0 / sqrt(2)*M + 0 * S;

		Labimg(x, y, 0) = l;
		Labimg(x, y, 1) = alpha;
		Labimg(x, y, 2) = beta;
	}
	return Labimg;
}

CImg<float> transferOfStatistics(CImg<float> Lab_source, CImg<float> Lab_target) {
	CImg<float> Lab_result(Lab_source._width, Lab_source._height, 1, 3, 0);
	CImg<float> mean_source(Lab_source._spectrum, 1, 1, 1, 0);
	CImg<float> std_source(Lab_source._spectrum, 1, 1, 1, 0);
	CImg<float> mean_target(Lab_target._spectrum, 1, 1, 1, 0);
	CImg<float> std_target(Lab_target._spectrum, 1, 1, 1, 0);
	/* Compute the means and standard deviations
	/* for each axis(L,a,b three axes in total) separately in Lab space */
	// source
	unsigned long n_source = Lab_source._width * Lab_source._height;
	cimg_forXYC(Lab_source, x, y, c)
		mean_source(c) += Lab_source(x, y, c);
	cimg_forX(mean_source, c)
		mean_source(c) /= n_source;
	cimg_forXYC(Lab_source, x, y, c)
		std_source(c) += pow(Lab_source(x, y, c) - mean_source(c), 2.0);
	cimg_forX(mean_source, c)
		std_source(c) = sqrt(std_source(c) / n_source);
	
	// target
	unsigned long n_target = Lab_target._width * Lab_target._height;
	cimg_forXYC(Lab_target, x, y, c)
		mean_target(c) += Lab_target(x, y, c);
	cimg_forX(mean_target, c)
		mean_target(c) /= n_target;
	cimg_forXYC(Lab_target, x, y, c)
		std_target(c) += pow(Lab_target(x, y, c) - mean_target(c), 2.0);
	cimg_forX(mean_target, c)
		std_target(c) = sqrt(std_target(c) / n_target);
	
	/* Transfer by adjust the mean and standard deviations along each of the three axes */
	cimg_forXYC(Lab_source, x, y, c) {
		Lab_result(x, y, c) = Lab_source(x, y, c) - mean_source(c);
		/* PureColorGuidedStyle method: Input a pure color target image and tune the 
		*  scale value to get a result image more conveniently and controllable. */
		float scale_val = 0.5;
		Lab_result(x, y, c) = Lab_result(x, y, c) * cimg::max(scale_val,
			(std_target(c) / std_source(c)));
		Lab_result(x, y, c) = Lab_result(x, y, c) + mean_target(c);
	}
	return Lab_result;
}


CImg<unsigned int> Lab2RGB(CImg<float> Labimg) {
	if (Labimg._spectrum != 3) {
		cerr << "Lab2RGB(): Instance is not a Lab image." << endl;
	}
	CImg<unsigned int> RGBimg(Labimg._width, Labimg._height, 1, 3, 0);
	float R, G, B, L, M, S, l, alpha, beta;
	cimg_forXY(Labimg, x, y) {
		l = Labimg(x, y, 0);
		alpha = Labimg(x, y, 1);
		beta = Labimg(x, y, 2);

		// Lab -> LMS
		L = sqrt(3.0) / 3.0 * l + sqrt(6) / 6.0 * alpha + sqrt(2) / 2.0 * beta;
		M = sqrt(3.0) / 3.0 * l + sqrt(6) / 6.0 * alpha - sqrt(2) / 2.0 * beta;
		S = sqrt(3.0) / 3.0 * l - sqrt(6) / 3.0 * alpha - 0 * beta;
		// Raising the pixel values to the power ten to go back to linear space
		L = pow(10.0, L);
		M = pow(10.0, M);
		S = pow(10.0, S);
		// LMS -> RGB
		R = 4.4679*L - 3.5873*M + 0.1193*S;
		G = -1.2186*L + 2.3809*M - 0.1624*S;
		B = 0.0497*L - 0.2439*M + 1.2045*S;

		RGBimg(x, y, 0) = cimg::max(cimg::min(R * 255, 255), 0); // range 0-255!!
		RGBimg(x, y, 1) = cimg::max(cimg::min(G * 255, 255), 0);
		RGBimg(x, y, 2) = cimg::max(cimg::min(B * 255, 255), 0);
	}
	return RGBimg;
}

CImg<unsigned int> colorTransfer(CImg<unsigned int> source, CImg<unsigned int> target) {
	CImg<float> Lab_result = transferOfStatistics(RGB2Lab(source), RGB2Lab(target));
	CImg<unsigned> RGBimg = Lab2RGB(Lab_result);
	return RGBimg;
}

int main() {
	CImg<unsigned int> source_img, target_img;
	vector<const char*> num = { "0", "1", "2", "3", "4", "5", "6", "7",
		"8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18",
		"19", "20", "21", "22", "23", "24", "25", "26", "27", "28" , "29"
		, "30" , "31" , "32", "33" };
	for (int i = 0; i <= 14; ++i) {
		// load source image
		char sourcePath[80];
		strcpy(sourcePath, "images/");
		strcat(sourcePath, num[i]);
		strcat(sourcePath, "_source.bmp");
		source_img.load_bmp(sourcePath);

		// load target image
		char targetPath[80];
		strcpy(targetPath, "images/");
		strcat(targetPath, num[i]);
		strcat(targetPath, "_target.bmp");
		target_img.load_bmp(targetPath);

		// perform image transfer between two images and write to file
		CImg<unsigned int> result_img = colorTransfer(source_img, target_img);
		char resultPath[80];
		strcpy(resultPath, "images/");
		strcat(resultPath, num[i]);
		strcat(resultPath, "_result.bmp");
		result_img.save(resultPath);
	}

	return 0;
}