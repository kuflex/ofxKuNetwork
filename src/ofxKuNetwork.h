#pragma once

#include "ofMain.h"
#include "ofxNetwork_ku.h"
#include "KuBufferReader.h"


//Synchronous data sender
class ofxKuNetworkTcpClient
{
public:
	ofxKuNetworkTcpClient();
	~ofxKuNetworkTcpClient();
	void setup( const string &addr, int port, int packetSize = 1024, bool enabled=true );
	void setDataPushMode(bool d);		//if true - collect data to buffer if even enabled=false
										//false by default
	void close();

	bool enabled() { return enabled_; }
	bool connected() { return _connected; }
	bool dataPushMode() { return dataPushMode_; }
	int frameNumber() { return frameNumber_; }

	int bufferSize() { return buffer_.size(); }
	vector<unsigned char> &buffer() { return buffer_; } 
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


private:
	bool enabled_;
	bool dataPushMode_;
	bool dataPushing();

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
	ofxKuNetworkTcpServer();
	~ofxKuNetworkTcpServer();
	void setup(int port, int packetSize = 1024, bool threaded = true, int maxBufferSize=10000000, bool enabled=true);
	//if threaded == true, it is asynchronous mode
	
	//use this for parsing the buffer
	void setupForParsingBuffer(unsigned char *buffer, int size);
	void setupForParsingBuffer(vector<unsigned char> &buffer, int size=-1);

	bool enabled() { return enabled_; }
	bool dataParsingMode() { return dataParsingMode_; }

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

	float time_received_;	//last time of receiving data

	void receive0();
	vector<char> _buffer;
	int maxN;
	int _N;					//Current count of received bytes
	KuBufferReader reader_;	//We use it for parsing buffer. Hint: you can use this class outside openFrameworks.

	bool _wantRestart;
	void startTCP();

	//reading data
	bool isDataNew_;
	vector<unsigned char> buffer_;
	int bufferSize_;
	int bufferIndex_;
	void shiftBuffer( char *buffer, int &len, int newLen );
};
