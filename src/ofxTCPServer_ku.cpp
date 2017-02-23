#include "ofxTCPServer_ku.h"
#include "ofxTCPClient_ku.h"
#include "ofUtils.h"

//--------------------------
ofxTCPServer_ku::ofxTCPServer_ku(){
	connected	= false;
	idCount		= 0;
	port		= 0;
	str			= "";
	messageDelimiter = "[/TCP]";
	bClientBlocking = false;
}

//--------------------------
ofxTCPServer_ku::~ofxTCPServer_ku(){
	close();
}

//--------------------------
void ofxTCPServer_ku::setVerbose(bool _verbose){
	ofLogWarning("ofxTCPServer_ku") << "setVerbose(): is deprecated, replaced by ofLogWarning and ofLogError";
}

//--------------------------
bool ofxTCPServer_ku::setup(int _port, bool blocking){
	if( !TCPServer.Create() ){
		ofLogError("ofxTCPServer_ku") << "setup(): couldn't create server";
		return false;
	}
	if( !TCPServer.Bind(_port) ){
		ofLogError("ofxTCPServer_ku") << "setup(): couldn't bind to port " << _port;
		return false;
	}

	connected		= true;
	port			= _port;
	bClientBlocking = blocking;

	std::unique_lock<std::mutex> lck(mConnectionsLock);
	startThread();
    serverReady.wait(lck);

	return true;
}

//--------------------------
void ofxTCPServer_ku::setMessageDelimiter(std::string delim){
	if(delim != ""){
		messageDelimiter = delim;
	}
}

//--------------------------
bool ofxTCPServer_ku::close(){
    stopThread();
	if( !TCPServer.Close() ){
		ofLogWarning("ofxTCPServer_ku") << "close(): couldn't close connections";
		waitForThread(false); // wait for the thread to finish
		return false;
    }else{
        ofLogVerbose("ofxTCPServer_ku") << "Closing server";
		waitForThread(false); // wait for the thread to finish
		return true;
	}
}

ofxTCPClient_ku & ofxTCPServer_ku::getClient(int clientID){
	return *TCPConnections.find(clientID)->second;
}

//--------------------------
bool ofxTCPServer_ku::disconnectClient(int clientID){
	std::unique_lock<std::mutex> lck( mConnectionsLock );
	if( !isClientSetup(clientID) ){
		ofLogWarning("ofxTCPServer_ku") << "disconnectClient(): client " << clientID << " doesn't exist";
		return false;
	}else if(getClient(clientID).close()){
		TCPConnections.erase(clientID);
		return true;
	}
	return false;
}

//--------------------------
bool ofxTCPServer_ku::disconnectAllClients(){
	std::unique_lock<std::mutex> lck( mConnectionsLock );
    TCPConnections.clear();
    return true;
}

//--------------------------
bool ofxTCPServer_ku::send(int clientID, std::string message){
	std::unique_lock<std::mutex> lck( mConnectionsLock );
	if( !isClientSetup(clientID) ){
		ofLogWarning("ofxTCPServer_ku") << "send(): client " << clientID << " doesn't exist";
		return false;
	}else{
        auto ret = getClient(clientID).send(message);
		if(!getClient(clientID).isConnected()) TCPConnections.erase(clientID);
        return ret;
	}
}

//--------------------------
bool ofxTCPServer_ku::sendToAll(std::string message){
	std::unique_lock<std::mutex> lck( mConnectionsLock );
	if(TCPConnections.size() == 0) return false;

	std::vector<int> disconnect;
	for(auto & conn: TCPConnections){
		if(conn.second->isConnected()) conn.second->send(message);
		else disconnect.push_back(conn.first);
	}
	for(int i=0; i<(int)disconnect.size(); i++){
    	TCPConnections.erase(disconnect[i]);
    }
	return true;
}

//--------------------------
std::string ofxTCPServer_ku::receive(int clientID){
	std::unique_lock<std::mutex> lck( mConnectionsLock );
	if( !isClientSetup(clientID) ){
		ofLogWarning("ofxTCPServer_ku") << "receive(): client " << clientID << " doesn't exist";
		return "client " + ofToString(clientID) + "doesn't exist";
	}
	
	if( !getClient(clientID).isConnected() ){
        TCPConnections.erase(clientID);
		return "";
	}

	return getClient(clientID).receive();
}

//--------------------------
bool ofxTCPServer_ku::sendRawBytes(int clientID, const char * rawBytes, const int numBytes){
	std::unique_lock<std::mutex> lck( mConnectionsLock );
	if( !isClientSetup(clientID) ){
		ofLogWarning("ofxTCPServer_ku") << "sendRawBytes(): client " << clientID << " doesn't exist";
		
		return false;
	}
	else{
		return getClient(clientID).sendRawBytes(rawBytes, numBytes);
	}
}

//--------------------------
bool ofxTCPServer_ku::sendRawBytesToAll(const char * rawBytes, const int numBytes){
	std::unique_lock<std::mutex> lck( mConnectionsLock );
	if(TCPConnections.size() == 0 || numBytes <= 0) return false;

	for(auto & conn: TCPConnections){
		if(conn.second->isConnected()){
			conn.second->sendRawBytes(rawBytes, numBytes);
		}
	}
	return true;
}


//--------------------------
bool ofxTCPServer_ku::sendRawMsg(int clientID, const char * rawBytes, const int numBytes){
	std::unique_lock<std::mutex> lck( mConnectionsLock );
	if( !isClientSetup(clientID) ){
		ofLogWarning("ofxTCPServer_ku") << "sendRawMsg(): client " << clientID << " doesn't exist";
		return false;
	}
	else{
		return getClient(clientID).sendRawMsg(rawBytes, numBytes);
	}
}

//--------------------------
bool ofxTCPServer_ku::sendRawMsgToAll(const char * rawBytes, const int numBytes){
	std::unique_lock<std::mutex> lck( mConnectionsLock );
	if(TCPConnections.empty() || numBytes <= 0) return false;

	for(auto & conn: TCPConnections){
		if(conn.second->isConnected()){
			conn.second->sendRawMsg(rawBytes, numBytes);
		}
	}
	return true;
}

//--------------------------
int ofxTCPServer_ku::getNumReceivedBytes(int clientID){
	std::unique_lock<std::mutex> lck( mConnectionsLock );
	if( !isClientSetup(clientID) ){
		ofLogWarning("ofxTCPServer_ku") << "getNumReceivedBytes(): client " << clientID << " doesn't exist";
		return 0;
	}

	return getClient(clientID).getNumReceivedBytes();
}

//--------------------------
int ofxTCPServer_ku::receiveRawBytes(int clientID, char * receiveBytes,  int numBytes){
	std::unique_lock<std::mutex> lck( mConnectionsLock );
	if( !isClientSetup(clientID) ){
		ofLogWarning("ofxTCPServer_ku") << "receiveRawBytes(): client " << clientID << " doesn't exist";
		return 0;
	}

	return getClient(clientID).receiveRawBytes(receiveBytes, numBytes);
}


//--------------------------
int ofxTCPServer_ku::peekReceiveRawBytes(int clientID, char * receiveBytes,  int numBytes){
	std::unique_lock<std::mutex> lck( mConnectionsLock );
	if( !isClientSetup(clientID) ){
		ofLog(OF_LOG_WARNING, "ofxTCPServer_ku: client " + ofToString(clientID) + " doesn't exist");
		return 0;
	}

	return getClient(clientID).peekReceiveRawBytes(receiveBytes, numBytes);
}

//--------------------------
int ofxTCPServer_ku::receiveRawMsg(int clientID, char * receiveBytes,  int numBytes){
	std::unique_lock<std::mutex> lck( mConnectionsLock );
	if( !isClientSetup(clientID) ){
		ofLogWarning("ofxTCPServer_ku") << "receiveRawMsg(): client " << clientID << " doesn't exist";
		return 0;
	}

	return getClient(clientID).receiveRawMsg(receiveBytes, numBytes);
}

//--------------------------
int ofxTCPServer_ku::getClientPort(int clientID){
	std::unique_lock<std::mutex> lck( mConnectionsLock );
	if( !isClientSetup(clientID) ){
		ofLogWarning("ofxTCPServer_ku") << "getClientPort(): client " << clientID << " doesn't exist";
		return 0;
	}
	else return getClient(clientID).getPort();
}

//--------------------------
std::string ofxTCPServer_ku::getClientIP(int clientID){
	std::unique_lock<std::mutex> lck( mConnectionsLock );
	if( !isClientSetup(clientID) ){
		ofLogWarning("ofxTCPServer_ku") << "getClientIP(): client " << clientID << " doesn't exist";
		return "000.000.000.000";
	}
	else return getClient(clientID).getIP();
}

//--------------------------
int ofxTCPServer_ku::getNumClients(){
	return TCPConnections.size();
}

//--------------------------
int ofxTCPServer_ku::getLastID(){
	return idCount;
}

//--------------------------
int ofxTCPServer_ku::getPort(){
	return port;
}

//--------------------------
bool ofxTCPServer_ku::isConnected(){
	return connected;
}

//--------------------------
bool ofxTCPServer_ku::isClientSetup(int clientID){
	return TCPConnections.find(clientID)!=TCPConnections.end();
}

//--------------------------
bool ofxTCPServer_ku::isClientConnected(int clientID){
	std::unique_lock<std::mutex> lck( mConnectionsLock );
	return isClientSetup(clientID) && getClient(clientID).isConnected();
}


void ofxTCPServer_ku::waitConnectedClient(){
	std::unique_lock<std::mutex> lck( mConnectionsLock );
	if(TCPConnections.empty()){
		serverReady.wait(lck);
	}
}

void ofxTCPServer_ku::waitConnectedClient(int ms){
	std::unique_lock<std::mutex> lck( mConnectionsLock );
	if(TCPConnections.empty()){
		serverReady.wait_for(lck, std::chrono::milliseconds(ms));
	}
}

//don't call this
//--------------------------
void ofxTCPServer_ku::threadedFunction(){

	ofLogVerbose("ofxTCPServer_ku") << "listening thread started";
	while( isThreadRunning() ){
		
		int acceptId;
		for(acceptId = 0; acceptId <= idCount; acceptId++){
			if(!isClientConnected(acceptId)) break;
		}
		
		if(acceptId == TCP_MAX_CLIENTS){
			ofLogWarning("ofxTCPServer_ku") << "no longer accepting connections, maximum number of clients reached: " << TCP_MAX_CLIENTS;
			break;
		}

		if( !TCPServer.Listen(TCP_MAX_CLIENTS) ){
			if(isThreadRunning()) ofLogError("ofxTCPServer_ku") << "listening failed";
		}

        {
			std::unique_lock<std::mutex> lck( mConnectionsLock );
            serverReady.notify_one();
        }
		
		//	we need to lock here, but can't as it blocks...
		//	so use a temporary to not block the lock 
		std::shared_ptr<ofxTCPClient_ku> client(new ofxTCPClient_ku);
		if( !TCPServer.Accept( client->TCPClient ) ){
			if(isThreadRunning()) ofLogError("ofxTCPServer_ku") << "couldn't accept client " << acceptId;
		}else{
			std::unique_lock<std::mutex> lck( mConnectionsLock );
			//	take owenership of socket from NewClient
			TCPConnections[acceptId] = client;
            TCPConnections[acceptId]->setupConnectionIdx(acceptId, bClientBlocking);
			TCPConnections[acceptId]->setMessageDelimiter(messageDelimiter);
			ofLogVerbose("ofxTCPServer_ku") << "client " << acceptId << " connected on port " << TCPConnections[acceptId]->getPort();
			if(acceptId == idCount) idCount++;
			serverReady.notify_all();
		}
	}
	idCount = 0;
	std::unique_lock<std::mutex> lck( mConnectionsLock );
	TCPConnections.clear();
	connected = false;
	ofLogVerbose("ofxTCPServer_ku") << "listening thread stopped";
}





