#include "ofxKuNetwork.h"

//----------------------------------------------------------------------
//Implementation of some simple protocol

const string ofxKuNetwork_PacketMarker = "[ofxKuNetwork data packet]"; //[Data packet]
const int ofxKuNetwork_PacketMarkerSize = 27; //13;
const int ofxKuNetwork_PacketHeaderSize = ofxKuNetwork_PacketMarkerSize + 3 * 4;	//frame, size, src32

struct ofxKuNetwork_PacketHeader {
	bool valid;
	int frame, size, src32;

	char buffer[ ofxKuNetwork_PacketHeaderSize ];
	int bufferSize;
	void setup( int frame0, int size0, int src320, bool fillBuffer )
	{
		valid = true;
		frame = frame0;
		size = size0;
		src32 = src320;
		if ( fillBuffer ) {
			//Filling buffer
			int k=0;
			memcpy( buffer + k, &ofxKuNetwork_PacketMarker[0], ofxKuNetwork_PacketMarkerSize );
			k += ofxKuNetwork_PacketMarkerSize;
			memcpy( buffer + k, &frame, 4 );
			k+=4;
			memcpy( buffer + k, &size, 4 );
			k+=4;
			memcpy( buffer + k, &src32, 4 );
			k+=4;

			bufferSize = ofxKuNetwork_PacketHeaderSize;
		}
	}

	//Searching and parsing header
	int findHeader( char *data, int len ) {
		for (int k=0; k<len - ofxKuNetwork_PacketHeaderSize; k++) {
			if ( data[k] == ofxKuNetwork_PacketMarker[0] ) {
				bool ok = true;
				for (int i=1; i<ofxKuNetwork_PacketMarkerSize; i++) {
					if ( data[k+i] != ofxKuNetwork_PacketMarker[i] ) {
						ok = false;
						break;
					}
				}
				if ( ok ) {	ok = parse( data + k ); }
				if ( ok ) {
					return k;
				}
			}
		}
		return -1;
	}

private:
	bool parse( char *data )	//It is suggested here that size of data is enough for parsing
	{
		int k=0;
		memcpy( buffer + k, &data + k, ofxKuNetwork_PacketMarkerSize );
		k += ofxKuNetwork_PacketMarkerSize;
		memcpy( &frame, data + k, 4 );
		k+=4;
		memcpy( &size, data + k, 4 );
		k+=4;
		memcpy( &src32, data + k, 4 );
		k+=4;
		return ( frame >= 0 && size >= 0 && size < 10000000 && src32 >= 0 );	//TODO Parameter "10000000" 
	}
};

//-------------------------------------------------------------------
ofxKuNetworkTcpClient::ofxKuNetworkTcpClient() {
	enabled_ = false;
	dataPushMode_ = false;	//false by default
}

//-------------------------------------------------------------------
ofxKuNetworkTcpClient::~ofxKuNetworkTcpClient() {
	close();
}

//-------------------------------------------------------------------
void ofxKuNetworkTcpClient::setup( const string &addr, int port, int packetSize, bool enabled )
{
	enabled_ = enabled;

	_addr = addr;
	_port = port;
	_packetSize = packetSize;

	if (enabled_) {
		tcpClient.TCPClientRef().SetTimeoutSend( 1 );	//TODO Parameter "1" - wait for send timeout
		tcpClient.TCPClientRef().SetTimeoutAccept( 1 );	//TODO Parameter
	}
	reconnect();

	frameNumber_ = 0;
}

//-------------------------------------------------------------------
void ofxKuNetworkTcpClient::close()
{
	if (!enabled_) return;
	if ( connected() ) {
		tcpClient.close();
	}
}

//-------------------------------------------------------------------
void ofxKuNetworkTcpClient::reconnect()
{
	if (!enabled_) return;
	cout << "Reconnect sender " << _addr << ", port " << _port << endl;
	bool blocking = true;
	_connected = tcpClient.setup( _addr, _port, blocking);		
	_connectTime = ofGetElapsedTimef();
	if ( _connected ) { cout << " ok" << endl; }
	else { cout << " failed" << endl; }
}

//-------------------------------------------------------------------
void ofxKuNetworkTcpClient::update()
{
	if (!enabled_) return;
	//Reconnecting
	if ( !connected() ) {			
		float deltaTime = ofGetElapsedTimef() - _connectTime;
		if( deltaTime > 1.0f ) {			//TODO Parameter - reconnecting time
			reconnect();
		} 
	}
}

//-------------------------------------------------------------------
bool ofxKuNetworkTcpClient::send( unsigned char *data, int size, int frameNumber )
{
	if (!enabled_) return false;
	update();	
	bool res = connected();
	//Header
	if ( res ) {
		ofxKuNetwork_PacketHeader header;
		header.setup( frameNumber, size, 0, true );
		res = tcpClient.sendRawBytes( header.buffer, header.bufferSize );
	}
	//Data
	int N = 0;
	if ( res ) {
		while ( N < size && res ) {
			int n = size - N;
			n = min( n, _packetSize );
			res = tcpClient.sendRawBytes( (char *)data + N, n );
			N += n;
		}
	}

	if ( !res && connected () ) {		//If sending was failed, then reconnect
		cout << "Data send error..." << endl;
		tcpClient.close();
		_connected = false;
	}

	return res;
}

//-------------------------------------------------------------------
//if true - collect data to buffer if even enabled=false
//false by default
void ofxKuNetworkTcpClient::setDataPushMode(bool d) {		
	dataPushMode_ = d;
}

//-------------------------------------------------------------------
bool ofxKuNetworkTcpClient::dataPushing() {
	return (enabled_ || dataPushMode_);
}

//-------------------------------------------------------------------
void ofxKuNetworkTcpClient::clearBuffer() {
	if (!dataPushing()) return;
	buffer_.clear();
}

//-------------------------------------------------------------------
void ofxKuNetworkTcpClient::putU8Array(const unsigned char *v, int n) {
	if (!dataPushing()) return;
	int m = buffer_.size();
	buffer_.resize(m + n);
	for (int i = 0; i < n; i++) {
		buffer_[m + i] = v[i];
	}
}

//-------------------------------------------------------------------
void ofxKuNetworkTcpClient::putInt(int value) {
	if (!dataPushing()) return;
	putU8Array((unsigned char*)&value, sizeof(value));
}

//-------------------------------------------------------------------
void ofxKuNetworkTcpClient::putFloat(float value) {
	if (!dataPushing()) return;
	putU8Array((unsigned char*)&value, sizeof(value));
}

//-------------------------------------------------------------------
void ofxKuNetworkTcpClient::putIntVector(const vector<int> &v) {
	if (!dataPushing()) return;
	if (v.size() > 0) {
		putInt(v.size() * sizeof(v[0]));
		putU8Array((unsigned char*)&v[0], sizeof(v[0])*v.size());
	}
}

//-------------------------------------------------------------------
void ofxKuNetworkTcpClient::putFloatVector(const vector<float> &v) {
	if (!dataPushing()) return;
	if (v.size() > 0) {
		putInt(v.size() * sizeof(v[0]));
		putU8Array((unsigned char*)&v[0], sizeof(v[0])*v.size());
	}
}

//-------------------------------------------------------------------
void ofxKuNetworkTcpClient::putU8Vector(const vector<unsigned char> &v) {
	if (!dataPushing()) return;
	if (v.size() > 0) {
		putInt(v.size() * sizeof(v[0]));
		putU8Array((unsigned char*)&v[0], sizeof(v[0])*v.size());
	}
}

//-------------------------------------------------------------------
void ofxKuNetworkTcpClient::putPixels(const ofPixels &pix) {
	if (!dataPushing()) return;
	putInt(pix.getWidth());
	putInt(pix.getHeight());
	putInt(pix.getNumChannels());
	int n = pix.getWidth() * pix.getHeight() * pix.getNumChannels(); //getTotalBytes();

	//as putting vector<unsigned char>
	putInt(n);
	putU8Array(pix.getPixels(), n);
}


//-------------------------------------------------------------------
void ofxKuNetworkTcpClient::send() {
	if (!dataPushing()) return;
	if (buffer_.size() > 0) {
		send(&buffer_[0], buffer_.size(), frameNumber_++);
		buffer_.clear();
	}
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
//-------------------------------------------------------------------
//-------------------------------------------------------------------
//-------------------------------------------------------------------
ofxKuNetworkTcpServer::ofxKuNetworkTcpServer() {
	enabled_ = false;
	dataParsingMode_ = false;
}

//-------------------------------------------------------------------
ofxKuNetworkTcpServer::~ofxKuNetworkTcpServer() {
	close();
}

//-------------------------------------------------------------------
void ofxKuNetworkTcpServer::setup( int port, int packetSize, bool threaded, int maxBufferSize, bool enabled )		
{
	enabled_ = enabled;
	dataParsingMode_ = false;
	_threaded = threaded;
	_wantRestart = false;

	_port = port;
	_packetSize = packetSize;

	_frame = -1;
	_size = 0;

	maxN = maxBufferSize; //1000000;		
	_buffer.resize( maxN );
	_N = 0;

	isDataNew_ = false;
	bufferIndex_ = 0;

	time_received_ = ofGetElapsedTimef();

	if (enabled_) {
		_data.resize( maxN );
		startTCP();

		if ( _threaded ) {
			startThread( true, false );   //blocking, verbose
		}
	}
}


//-------------------------------------------------------------------
void ofxKuNetworkTcpServer::setupForParsingBuffer(vector<unsigned char> &buffer, int size) {
	if (size < 0) size = buffer.size();
	setupForParsingBuffer(&buffer[0], size);
}

//-------------------------------------------------------------------
void ofxKuNetworkTcpServer::setupForParsingBuffer(unsigned char *buffer, int size) {
	enabled_ = false;
	dataParsingMode_ = true;

	_size = size;

	maxN = _size;		
	_N = 0;

	bufferSize_ = _size;
	buffer_.resize(_size);
	memcpy(&buffer_[0], buffer, _size);

	bufferIndex_ = 0;

	reader_.setup_no_copy(&buffer_[0], _size);
}

//-------------------------------------------------------------------
bool ofxKuNetworkTcpServer::parsing() {
	return (enabled_ || dataParsingMode_);
}

//-------------------------------------------------------------------
void ofxKuNetworkTcpServer::startTCP()
{
	if (!enabled_) return;
	cout << "ofxKuNetworkTcpServer - start receiver, port " << _port << endl;
	bool blocking = true;
	TCP.setup(_port, blocking);
	int timeoutReceiveSec = 1;
	TCP.TCPServerRef().SetTimeoutReceive( timeoutReceiveSec );	
}

//-------------------------------------------------------------------
void ofxKuNetworkTcpServer::close()
{
	if (!enabled_) return;
	disconnectAll();			//TODO for some reason it don't works, if connection is valid, "waitForThread" wait infinity time.

	if ( _threaded ) {
		long wait_ms = 10000;	//WARNING - is not closed, then can be crash!
		waitForThread(true, wait_ms);
	}
	TCP.close();
}

//-------------------------------------------------------------------
void ofxKuNetworkTcpServer::restart()	
{
	if (!enabled_) return;
	_wantRestart = true;
}

//-------------------------------------------------------------------
void ofxKuNetworkTcpServer::threadedFunction()
{
	if (!enabled_) return;
	while( isThreadRunning() != 0 ){
		//Receive
		receive0();
		
		//if not received data for a long time - then wait a little
		float time = ofGetElapsedTimef();
		if (TCP.getNumClients() == 0 || time - time_received_ >= 1.0) {	//TODO parameter of waiting
			ofSleepMillis(200);			//TODO parameter of sleeping
		}

		//Restarting - just disconnect all clients
		if ( _wantRestart ) {
			_wantRestart = false;
			disconnectAll();
		}
	}

}


//-------------------------------------------------------------------
void ofxKuNetworkTcpServer::receive()
{
	if (!enabled_) return;
	if ( !_threaded ) {
		receive0();
	}
}

//-------------------------------------------------------------------
//Shortening buffer to length newLen
void ofxKuNetworkTcpServer::shiftBuffer( char *buffer, int &len, int newLen )
{
	if ( newLen < len ) {
		int start = len - newLen;
		for (int i=0; i<newLen; i++) {
			buffer[ i ] = buffer[ i + start ];
		}
		len = newLen;
	}
}

//-------------------------------------------------------------------
void ofxKuNetworkTcpServer::receive0()
{
	if (!enabled_) return;
	char *buffer = &_buffer[0];
	for(int k = 0; k < TCP.getNumClients(); k++){		
		//TODO here is the problem: if several clients sent data, it will be shuffled!
		//So, need to collect data for each client separately
		if ( !TCP.isClientConnected(k) ) { 
			continue;
		}

		int free = maxN - _N;
		if ( free > 0 ) {
			free = min( free, _packetSize );
			int rec = TCP.receiveRawBytes( k, (char *)(buffer + _N), free ); 
			if ( rec >= 0 ) {
				if (rec > 0) {
					time_received_ = ofGetElapsedTimef();

					_N += rec;
					//cout << "rec " << rec << endl;
					//Searching header
					ofxKuNetwork_PacketHeader header;
					int headerPos = header.findHeader((char *)buffer, _N);
					if (headerPos >= 0) {

						//cout << "reading frame " << header.frame << endl;

						//Header is found, now receive data

						int headerEnd = headerPos + ofxKuNetwork_PacketHeaderSize;
						int N = _N - headerEnd;

						//Shifting buffer to data begin
						shiftBuffer(buffer, _N, N);

						int size = header.size;

						float lastTime = ofGetElapsedTimef();	//Time of last successfull receiving
						while (_N < size) {
							rec = TCP.receiveRawBytes(k, (char *)(buffer + _N), free);
							if (rec > 0) {
								_N += rec;
								lastTime = ofGetElapsedTimef();
							}
							else {
								float time = ofGetElapsedTimef();
								if (time >= lastTime + 1.0) {		//TODO Parameter - connection is lost
									cout << "Reading data error frame " << header.frame << endl;
									break;
								}
							}
						}
						if (_N >= size) {
							if (_threaded) lock();
							//Data was read successfull
							memcpy(&_data[0], buffer, size);
							_frame = header.frame;
							_size = header.size;
							isDataNew_ = true;
							//cout << _frame << endl;
							if (_threaded) unlock();
						}
					}
					else {
						//Shortening current buffer to continue searching for header
						shiftBuffer(buffer, _N, ofxKuNetwork_PacketHeaderSize);
					}
				}


			}
			else {
				cout << "Network: Receive error - no data" << endl;
				disconnectClient( k );
				_N = 0;						//Reset read data
				return;						
			}
		}
	}
}

//-------------------------------------------------------------------
void ofxKuNetworkTcpServer::disconnectClient( int id )	//отключить клиента
{
	if (!enabled_) return;
	cout << "\tDisconnect client " << id + 1 << endl;
	if ( id < TCP.getNumClients() ) {
		TCP.disconnectClient( id );
	}
}

//-------------------------------------------------------------------
void ofxKuNetworkTcpServer::disconnectAll() {
	cout << "\tDisconnecting all clients" << endl;
	TCP.disconnectAllClients();
	//for (int k = 0; k < TCP.getNumClients(); k++) {
	//	if (TCP.isClientConnected(k)) {
	//		disconnectClient(k);
	//	}
	//}
}


//-------------------------------------------------------------------
bool ofxKuNetworkTcpServer::isDataNew() {
	if (!enabled_) return false;
	if (_threaded) lock();
	bool result = isDataNew_;
	isDataNew_ = false;
	if (result) {
		//copy buffer for reading
		//TODO current implementation is unsafe, need use mutex, because thread can overwrite _size and _data during copy!
		bufferSize_ = _size;
		buffer_.resize(bufferSize_);
		for (int i = 0; i < bufferSize_; i++) {
			buffer_[i] = _data[i];
		}
		reader_.setup_no_copy(&buffer_[0], _size);
	}
	if (_threaded) unlock();
	return result;
}

//-------------------------------------------------------------------
bool ofxKuNetworkTcpServer::getU8Array(unsigned char *v, int n) {
	if (!parsing()) return false;
	return reader_.getU8Array(v, n);	
}

//-------------------------------------------------------------------
int ofxKuNetworkTcpServer::getInt() {
	if (!parsing()) return 0;
	return reader_.getInt();
}

//-------------------------------------------------------------------
float ofxKuNetworkTcpServer::getFloat() {
	if (!parsing()) return 0;
	return reader_.getFloat();
}

//-------------------------------------------------------------------
bool ofxKuNetworkTcpServer::getIntVector(vector<int> &v) {
	if (!parsing()) return false;
	return reader_.getIntVector(v);
}

//-------------------------------------------------------------------
bool ofxKuNetworkTcpServer::getFloatVector(vector<float> &v) {
	if (!parsing()) return false;
	return reader_.getFloatVector(v);
}

//-------------------------------------------------------------------
bool ofxKuNetworkTcpServer::getU8Vector(vector<unsigned char> &v) {
	if (!parsing()) return false;
	return reader_.getU8Vector(v);
}

//-------------------------------------------------------------------
ofPixels ofxKuNetworkTcpServer::getPixels() {
	ofPixels pix;
	if (!parsing()) return pix;

	int w = getInt();
	int h = getInt();
	int channels = getInt();
	vector<unsigned char> data;
	if (getU8Vector(data)) {
		if (data.size() == w*h*channels) {
			pix.setFromPixels(&data[0], w, h, channels);	
		}
	}

	return pix;
}

//-------------------------------------------------------------------
