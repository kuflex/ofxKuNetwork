# ofxKuNetwork - openFrameworks addon for sending arrays over network

It's based on ofxNetwork core addon and uses TCP for transmission.

It supports int, float, vector<int>, vector<float>, ofPixels values. 

## Installation

Download and unzip as *ofxKuNetwork* to *addons* folder of your openFrameworks installation.

Addon works in openFrameworks 0.9.3, OSX/Windows/Linux.

For working with openFrameworks 0.7.4 - see addon's branch dedicated to 0.7.4. In this case, please don not add to the project files 
 	*ofxTCPClient_ku.cpp*, *ofxTCPClient_ku.h*,	*ofxTCPServer_ku.cpp*, *ofxTCPServer_ku.h*.
  Also, you need to add the following lines to *public:* declarations of *ofxNetwork* addon's files:
* ofxTCPClient.h: ofxTCPManager	&TCPClientRef() { return TCPClient; }	//Added by Kuflex
* ofxTCPServer.h: ofxTCPManager	&TCPServerRef() { return TCPServer; }	//Added by Kuflex

## Usage

Please, see example *example-basic* included in addon.

