#include <iostream>
#include "Jpegfile.h"
#include <omp.h>
#include <cmath>
#include <cstdlib>
#include <omp.h>

using namespace std;



// width and height of the image ".jpg" and the pointer to the image
UINT height;
UINT nThreads;
UINT width;


// sets .jpg photo to memory BYTE
BYTE *dataBuf = JpegFile::JpegFileToRGB("imageofyourchoice.jpg", &width, &height);  // reading image
BYTE *dataBuf2 = JpegFile::JpegFileToRGB("imageofyourchoice.jpg", &width, &height); // reading image that will become our altered image

//// K-means algorithm


// function that picks generators at random points in photo
void pickgenerator(int k, double **generator)
{
	for (int i = 0; i < k; ++i)
	{
		generator[i] = new double[3];
	}
	//generates pixels from original loaded photo
	for (int i = 0; i < k; ++i)
	{

		int row = rand() % (height - 1);
		int col = rand() % (width - 1);

		BYTE *pRed, *pGrn, *pBlu;
		pRed = (dataBuf + row * width * 3 + col * 3);
		pGrn = (dataBuf + row * width * 3 + col * 3 + 1);
		pBlu = (dataBuf + row * width * 3 + col * 3 + 2);

		generator[i][0] = *pRed;
		generator[i][1] = *pGrn;
		generator[i][2] = *pBlu;
	}
}

int main()
{
	// number of colors used for altered pixelized image (default 3 if want more "complete" photo can change to higher value)
	
	int k = 3;
	
	// number of times loop runs through 'k generators'
	int iterations = 20;
	
	
	//Parallelized part for openmp
	/////////////////////////////////////////////////////////////////
	//cout << "Enter the number of threads to use \n\n";
	//cin >> nThreads;
	
	// will tell user processing time for image load and altered image write out
	double start = omp_get_wtime();
	/////////////////////////////////////////////////////////////////
	
	// set up generator and avg generators as double pointers that will store doubles
	
	double *distgenerator = new double[k];   // distance array
	double **generator = new double *[k];	// generator array
	double **avggenerator = new double *[k]; // average color array


	UINT row,col;	// assigns row and col as unsigned ints

	///////////////////////////////////////////////
	// calls our previously declared function to generate 3 random RGB pixel arrays
	
	pickgenerator(k, generator);
	
	// initializes our generator that takes account of average of red, blue, green pixels in image
	for (int i = 0; i < k; i++)
	{
		avggenerator[i] = new double[4]{0}; // for this to be accepted must use C++11 
	}
	
	// another parallelization part: 
//#pragma omp parallel for num_threads(nThreads) reduction(+: col, k, row)

	// begin program by choosing how many times (iterations) the program will get pixels from photo
	for (int repeat = 0; repeat < iterations; repeat++)
	{
		for (row = 0; row < height; row++)
		{
			for (col = 0; col < width; col++)
			{
				double RGB[4];
	
				//getting color of pixels for this "default image" generator
				
				BYTE *pRed, *pGrn, *pBlu;
				pRed = (dataBuf + row * width * 3 + col * 3);
				pGrn = (dataBuf + row * width * 3 + col * 3 + 1);
				pBlu = (dataBuf + row * width * 3 + col * 3 + 2);
	
				RGB[0] = *pRed;
				RGB[1] = *pGrn;
				RGB[2] = *pBlu;
				RGB[3] = 1;
	
				// calculating distance sqrt(r^2 + g^2 + b^2)
				for (int i = 0; i < k; i++)
				{
					distgenerator[i] = sqrt(pow(RGB[0] - generator[i][0], 2) + pow(RGB[1] - generator[i][1], 2) + pow(RGB[2] - generator[i][2], 2));
					
				}
				
	
				// finding the min distance between colors
				int index = 0;
				for (int i = 1; i < k; i++)
				{
					if (distgenerator[index] > distgenerator[i])
					{
						index = i;
					}
				}
				
				// summation of our clustered average generators and our RGB arrays from image
				for( int i = 0; i < 4; i++) {
					
				avggenerator[index][i] += RGB[i];
			}
				
		}
	}
	// taking averages and then applying them to our generator for use for construction of the altered kmeans image
		for (int i = 0; i < k; i++)
		{
			avggenerator[i][0] /= avggenerator[i][3];
			avggenerator[i][1] /= avggenerator[i][3];
			avggenerator[i][2] /= avggenerator[i][3];
			
			generator[i][0] = avggenerator[i][0];
			generator[i][1] = avggenerator[i][1];
			generator[i][2] = avggenerator[i][2];
	
			avggenerator[i][0] = 0;
			avggenerator[i][1] = 0;
			avggenerator[i][2] = 0;	
			avggenerator[i][3] = 0;	
			
		}
	}

	// kmeans image to another jpg file
	for(row = 0; row < height; row++) {
		for(col = 0; col < width; col++) {
			
				BYTE *pRed, *pGrn, *pBlu;
				pRed = (dataBuf2 + row * width * 3 + col * 3);
				pGrn = (dataBuf2 + row * width * 3 + col * 3 + 1);
				pBlu = (dataBuf2 + row * width * 3 + col * 3 + 2);
			
			for(int i = 0; i < k; i++){
					distgenerator[i] = sqrt(pow(*pRed - generator[i][0], 2) + pow(*pGrn - generator[i][1], 2) + pow(*pBlu - generator[i][2], 2));
				}
			
				int index = 0;
				for (int i = 1; i < k; i++)
					{
					if (distgenerator[index] > distgenerator[i])
					{
					index = i;
					}
				}
			
					*pRed = generator[index][0];
					*pGrn = generator[index][1];
					*pBlu = generator[index][2];
		}
	}

	double end = omp_get_wtime() - start;
				
	
	JpegFile::RGBToJpegFile("imageofyourchoice.jpg", dataBuf2, width, height, true, 75);
	
	cout << "The processing time that this thread count took was " << end << " seconds." << endl;
	
	// important to delete our buffers (images) and the initialized arrays to avoid additional overhead
	delete dataBuf;
	delete dataBuf2;
	for (int i = 0; i < k; ++i)
	{
		delete[] generator[i];
		delete[] avggenerator[i];
	}
	delete[] generator;
	delete[] distgenerator;
	delete[] avggenerator;
	
	return 0;
}
