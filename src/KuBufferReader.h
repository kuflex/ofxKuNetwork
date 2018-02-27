#pragma once

//Class for using ofxKuNetwork sent data outsize openFrameworks projects
//(Currently we ise it for Unreal Engine)

#include <string>
#include <vector>
using namespace std;

struct KuBufferReader {
	void setup_no_copy(unsigned char *buffer, int size);
	int getInt();								
	float getFloat();
	bool getIntVector(vector<int> &v);
	bool getFloatVector(vector<float> &v);
	bool getU8Vector(vector<unsigned char> &v);
private:
	unsigned char *buffer_;
	int bufferSize_;
	int bufferIndex_;	
};
