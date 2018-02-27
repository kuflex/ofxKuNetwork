#include "KuBufferReader.h"

//----------------------------------------------------------------------
KuBufferReader::KuBufferReader() {
	buffer_ = 0;
	bufferSize_ = 0;
	bufferIndex_ = 0;
}

//----------------------------------------------------------------------
void KuBufferReader::setup_no_copy(unsigned char *buffer, int size) {
	buffer_ = buffer;
	bufferSize_ = size;
	bufferIndex_ = 0;
}

//----------------------------------------------------------------------
bool KuBufferReader::getU8Array(unsigned char *v, int n) {
	if (n + bufferIndex_ <= bufferSize_) {
		for (int i = 0; i < n; i++) {
			v[i] = buffer_[i + bufferIndex_];
		}
		bufferIndex_ += n;
		return true;
	}
	return false;
}

//-------------------------------------------------------------------
int KuBufferReader::getInt() {
	int v;
	if (getU8Array((unsigned char*)&v, sizeof(v))) {
		return v;
	}
	return 0;
}

//-------------------------------------------------------------------
float KuBufferReader::getFloat() {
	float v;
	if (getU8Array((unsigned char*)&v, sizeof(v))) {
		return v;
	}
	return 0;
}

//-------------------------------------------------------------------
bool KuBufferReader::getIntVector(vector<int> &v) {
	int n = getInt();
	if (n > 0 && n + bufferIndex_ <= bufferSize_) {
		v.resize(n/sizeof(int));
		return getU8Array((unsigned char *)&v[0], n);
	}
	v.clear();
	return false;
}

//-------------------------------------------------------------------
bool KuBufferReader::getFloatVector(vector<float> &v) {
	int n = getInt();
	if (n > 0 && n + bufferIndex_ <= bufferSize_) {
		v.resize(n/sizeof(float));
		return getU8Array((unsigned char *)&v[0], n);
	}
	v.clear();
	return false;
}

//-------------------------------------------------------------------
bool KuBufferReader::getU8Vector(vector<unsigned char> &v) {
	int n = getInt();
	if (n > 0 && n + bufferIndex_ <= bufferSize_) {
		v.resize(n);
		return getU8Array((unsigned char *)&v[0], sizeof(v[0])*n);
	}
	v.clear();
	return false;
}

//-------------------------------------------------------------------
