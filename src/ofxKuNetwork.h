#pragma once

#include "ofMain.h"
#include "ofxNetwork_ku.h"


//Synchronous data sender
class ofxKuNetworkTcpClient
{
public:
	void setup( const string &addr, int port, int packetSize = 1024, bool enabled=true );	
	void close();
	
	void clearBuffer();
	void putU8Array(const unsigned char *v, int n);
	void putInt(int value);
	void putFloat(float value);
	void putIntVector(const vector<int> &v);
	void putFloatVector(const vector<float> &v);
	void putU8Vector(const vector<unsigned char> &v);
	void putPixels(const ofPixels &pix);
	void send();

	bool send( unsigned char *data, int dataSize, int frameNumber );
	void update();

	bool enabled() { return enabled_; }
	bool connected() { return _connected; }
private:
	bool enabled_;

	ofxTCPClient_ku tcpClient;
	int _port;
	string _addr;
	int _packetSize;

	bool _connected;
	float _connectTime;
	void reconnect();

	vector<unsigned char> buffer_;	//buffer for sending
	int frameNumber_;
};


//Data receiver, working as syncronous or asynchronous 
class ofxKuNetworkTcpServer : public ofThread{
public:

	void setup(int port, int packetSize = 1024, bool threaded = true, int maxBufferSize=10000000, bool enabled=true);
	//if threaded == true, it is asynchronous mode

	bool enabled() { return enabled_; }

	void close();
	void receive();
	void restart();
	
	bool isDataNew();	//need to call it in order to read data

	bool getU8Array(unsigned char *v, int n);	
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
