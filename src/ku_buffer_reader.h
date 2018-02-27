#pragma once



bool ku_buffe_getU8Array(unsigned char *v, int n);	
	int getInt();								
	float getFloat();
	bool getIntVector(vector<int> &v);
	bool getFloatVector(vector<float> &v);
	bool getU8Vector(vector<unsigned char> &v);
	ofPixels getPixels();
	
	int frame() { return _frame; }		//id of last received data
	int size()  { return _size; }		//size of last received data
	vector<unsigned char> &data() { return _data; }	//Last received data. 
				//Note, data.size() can be greater than size()

	void threadedFunction();


private:
	bool enabled_;
	bool dataParsingMode_;
	bool parsing();

	bool _threaded;

	int _port;
	int _packetSize;

	ofxTCPServer_ku TCP;					
	void disconnectClient( int id );	//Disconnect client

	int _frame;
	int _size;
	vector<unsigned char> _data;

	void receive0();
	vector<char> _buffer;
	int maxN;
	int _N;			//Current count of received bytes

	bool _wantRestart;
	void startTCP();

	//reading data
	bool isDataNew_;
	vector<unsigned char> buffer_;
	int bufferSize_;
	int bufferIndex_;
	void shiftBuffer( char *buffer, int &len, int newLen );
};
