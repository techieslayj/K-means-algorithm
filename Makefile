all:
	g++ -fopenmp -o kmean.exe Jpegfile.cpp kmeans.cpp JpegLib/libjpeg.a -std=c++11
