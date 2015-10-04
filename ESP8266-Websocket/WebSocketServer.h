/*
Websocket-Arduino, a websocket implementation for Arduino
Copyright 2011 Per Ejeklint

Based on previous implementations by
Copyright 2010 Ben Swanson
and
Copyright 2010 Randall Brewer
and
Copyright 2010 Oliver Smith

Some code and concept based off of Webduino library
Copyright 2009 Ben Combee, Ran Talbott

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

-------------
Now based off
http://www.whatwg.org/specs/web-socket-protocol/

- OLD -
Currently based off of "The Web Socket protocol" draft (v 75):
http://tools.ietf.org/html/draft-hixie-thewebsocketprotocol-75
*/


#ifndef WEBSOCKETSERVER_H_
#define WEBSOCKETSERVER_H_

#include <Arduino.h>
#include <Stream.h>
#include "String.h"
#include "Server.h"
#include "Client.h"

// CRLF characters to terminate lines/handshakes in headers.
#define CRLF "\r\n"

// Amount of time (in ms) a user may be connected before getting disconnected 
// for timing out (i.e. not sending any data to the server).
#define TIMEOUT_IN_MS 10000
#define BUFFER_LENGTH 32

// ACTION_SPACE is how many actions are allowed in a program. Defaults to 
// 5 unless overwritten by user.
#ifndef CALLBACK_FUNCTIONS
#define CALLBACK_FUNCTIONS 1
#endif

// Don't allow the client to send big frames of data. This will flood the Arduinos
// memory and might even crash it.
#ifndef MAX_FRAME_LENGTH
#define MAX_FRAME_LENGTH 256
#endif

#define SIZE(array) (sizeof(array) / sizeof(*array))

class WebSocketServer {
public:

    // Handle connection requests to validate and process/refuse
    // connections.
    bool handshake(Client &client);
    
    // Get data off of the stream
    String getData();

    // Write data to the stream
    void sendData(const char *str);
    void sendData(String str);
    void sendData(unsigned char byte);
    void sendData(char *str, int size,unsigned char header );

private:
    Client *socket_client;
    unsigned long _startMillis;

    const char *socket_urlPrefix;

    String origin;
    String host;
    bool hixie76style;

    // Discovers if the client's header is requesting an upgrade to a
    // websocket connection.
    bool analyzeRequest(int bufferLength);

#ifdef SUPPORT_HIXIE_76
    String handleHixie76Stream();
#endif
    String handleStream();    
    
    // Disconnect user gracefully.
    void disconnectStream();
    
    int timedRead();

    void sendEncodedData(char *str);
    void sendEncodedData(String str);
    void sendEncodedData(char byte);
    void sendEncodedData(char *str, int size, unsigned char header);
};



#endif