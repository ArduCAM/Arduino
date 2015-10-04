#define DEBUGGING
//#define SUPPORT_HIXIE_76

//MS:
#include <string.h>

#include "global.h"
#include "WebSocketServer.h"

#ifdef SUPPORT_HIXIE_76
#include "MD5.c"
#endif

#include "sha1.h"
#include "base64.h"


bool WebSocketServer::handshake(Client &client) {

    socket_client = &client;

    // If there is a connected client->
    if (socket_client->connected()) {
        // Check request and look for websocket handshake
#ifdef DEBUGGING
            Serial.println(F("Client connected"));
#endif
        if (analyzeRequest(BUFFER_LENGTH)) {
#ifdef DEBUGGING
                Serial.println(F("Websocket established"));
#endif

                return true;

        } else {
            // Might just need to break until out of socket_client loop.
#ifdef DEBUGGING
            Serial.println(F("Disconnecting client"));
#endif
            disconnectStream();

            return false;
        }
    } else {
        return false;
    }
}

bool WebSocketServer::analyzeRequest(int bufferLength) {
    // Use String library to do some sort of read() magic here.
    String temp ="";

    int bite;
    bool foundupgrade = false;
    String oldkey[2];
    unsigned long intkey[2];
    String newkey;

    hixie76style = false;
    
#ifdef DEBUGGING
    Serial.println(F("Analyzing request headers"));
#endif

    // TODO: More robust string extraction
    while ((bite = socket_client->read()) != -1) {
//				Serial.print(bite);
        temp += (char)bite;

        if ((char)bite == '\n') {
#ifdef DEBUGGING
            Serial.print("Got Line: " + temp);
#endif
            // TODO: Should ignore case when comparing and allow 0-n whitespace after ':'. See the spec:
            // http://www.w3.org/Protocols/rfc2616/rfc2616-sec4.html
            if (!foundupgrade && temp.startsWith("Upgrade: WebSocket")) {
                // OK, it's a websockets handshake for sure
                foundupgrade = true;
                hixie76style = true;
            } else if (!foundupgrade && temp.startsWith("Upgrade: websocket")) {
                foundupgrade = true;
                hixie76style = false;
            } else if (temp.startsWith("Origin: ")) {
                origin = temp.substring(8,temp.length() - 2); // Don't save last CR+LF
            } else if (temp.startsWith("Host: ")) {
                host = temp.substring(6,temp.length() - 2); // Don't save last CR+LF
            } else if (temp.startsWith("Sec-WebSocket-Key1: ")) {
                oldkey[0]=temp.substring(20,temp.length() - 2); // Don't save last CR+LF
            } else if (temp.startsWith("Sec-WebSocket-Key2: ")) {
                oldkey[1]=temp.substring(20,temp.length() - 2); // Don't save last CR+LF
            } else if (temp.startsWith("Sec-WebSocket-Key: ")) {
                newkey=temp.substring(19,temp.length() - 2); // Don't save last CR+LF
            }
            temp = "";		
        }
        

        if (!socket_client->available()) {
          delay(20);
        }
        
    }
		
    if (!socket_client->connected()) {
        return false;
    }

		
    temp += 0; // Terminate string

    // Assert that we have all headers that are needed. If so, go ahead and
    // send response headers.
    if (foundupgrade == true) {

#ifdef SUPPORT_HIXIE_76
        if (hixie76style && host.length() > 0 && oldkey[0].length() > 0 && oldkey[1].length() > 0) {
            // All ok, proceed with challenge and MD5 digest
            char key3[9] = {0};
            // What now is in temp should be the third key
            temp.toCharArray(key3, 9);

            // Process keys
            for (int i = 0; i <= 1; i++) {
                unsigned int spaces =0;
                String numbers;
                
                for (int c = 0; c < oldkey[i].length(); c++) {
                    char ac = oldkey[i].charAt(c);
                    if (ac >= '0' && ac <= '9') {
                        numbers += ac;
                    }
                    if (ac == ' ') {
                        spaces++;
                    }
                }
                char numberschar[numbers.length() + 1];
                numbers.toCharArray(numberschar, numbers.length()+1);
                intkey[i] = strtoul(numberschar, NULL, 10) / spaces;		
            }
            
            unsigned char challenge[16] = {0};
            challenge[0] = (unsigned char) ((intkey[0] >> 24) & 0xFF);
            challenge[1] = (unsigned char) ((intkey[0] >> 16) & 0xFF);
            challenge[2] = (unsigned char) ((intkey[0] >>  8) & 0xFF);
            challenge[3] = (unsigned char) ((intkey[0]      ) & 0xFF);	
            challenge[4] = (unsigned char) ((intkey[1] >> 24) & 0xFF);
            challenge[5] = (unsigned char) ((intkey[1] >> 16) & 0xFF);
            challenge[6] = (unsigned char) ((intkey[1] >>  8) & 0xFF);
            challenge[7] = (unsigned char) ((intkey[1]      ) & 0xFF);
            
            memcpy(challenge + 8, key3, 8);
            
            unsigned char md5Digest[16];
            MD5(challenge, md5Digest, 16);
            
            socket_client->print(F("HTTP/1.1 101 Web Socket Protocol Handshake\r\n"));
            socket_client->print(F("Upgrade: WebSocket\r\n"));
            socket_client->print(F("Connection: Upgrade\r\n"));
            socket_client->print(F("Sec-WebSocket-Origin: "));        
            socket_client->print(origin);
            socket_client->print(CRLF);
            
            // The "Host:" value should be used as location
            socket_client->print(F("Sec-WebSocket-Location: ws://"));
            socket_client->print(host);
            socket_client->print(socket_urlPrefix);
            socket_client->print(CRLF);
            socket_client->print(CRLF);
            
            socket_client->write(md5Digest, 16);

            return true;
        }
#endif

        if (!hixie76style && newkey.length() > 0) {

            // add the magic string
            newkey += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

            uint8_t *hash;
            char result[21];
            char b64Result[30];

            SHA1Context sha;
            int err;
            uint8_t Message_Digest[20];
            
            err = SHA1Reset(&sha);
            err = SHA1Input(&sha, reinterpret_cast<const uint8_t *>(newkey.c_str()), newkey.length());
            err = SHA1Result(&sha, Message_Digest);
            hash = Message_Digest;

            for (int i=0; i<20; ++i) {
                result[i] = (char)hash[i];
            }
            result[20] = '\0';

            base64_encode(b64Result, result, 20);
            socket_client->print(F("HTTP/1.1 101 Web Socket Protocol Handshake\r\n"));
            socket_client->print(F("Upgrade: websocket\r\n"));
            socket_client->print(F("Connection: Upgrade\r\n"));
            socket_client->print(F("Sec-WebSocket-Accept: "));
            socket_client->print(b64Result);
            socket_client->print(CRLF);
            socket_client->print(CRLF);
            //delay(20000);
						//Serial.println("response completed");
            return true;
        } else {
            // something went horribly wrong
            return false;
        }
    } else {
        // Nope, failed handshake. Disconnect
#ifdef DEBUGGING
        Serial.println(F("Header mismatch"));
#endif
        return false;
    }
}

#ifdef SUPPORT_HIXIE_76
String WebSocketServer::handleHixie76Stream() {
    int bite;
    int frameLength = 0;
    // String to hold bytes sent by client to server.
    String socketString;

    if (socket_client->connected() && socket_client->available()) {
        bite = timedRead();

        if (bite != -1) {
            if (bite == 0)
                continue; // Frame start, don't save

            if ((uint8_t) bite == 0xFF) {
                // Frame end. Process what we got.
                return socketString;
                
            } else {
                socketString += (char)bite;
                frameLength++;            

                if (frameLength > MAX_FRAME_LENGTH) {
                    // Too big to handle!
#ifdef DEBUGGING
                    Serial.print("Client send frame exceeding ");
                    Serial.print(MAX_FRAME_LENGTH);
                    Serial.println(" bytes");
#endif
                    return;
                }  
            }           
        }
    }

    return socketString;
}

#endif

String WebSocketServer::handleStream() {
    uint8_t msgtype;
    uint8_t bite;
    unsigned int length;
    uint8_t mask[4];
    uint8_t index;
    unsigned int i;

    // String to hold bytes sent by client to server.
    String socketString;

    if (socket_client->connected() && socket_client->available()) {

        msgtype = timedRead();
        if (!socket_client->connected()) {
            return socketString;
        }

        length = timedRead() & 127;
        if (!socket_client->connected()) {
            return socketString;
        }

        index = 6;

        if (length == 126) {
            length = timedRead() << 8;
            if (!socket_client->connected()) {
                return socketString;
            }
            
            length |= timedRead();
            if (!socket_client->connected()) {
                return socketString;
            }   

        } else if (length == 127) {
#ifdef DEBUGGING
            Serial.println(F("No support for over 16 bit sized messages"));
#endif
            while(1) {
                // halt, can't handle this case
            }
        }

        // get the mask
        mask[0] = timedRead();
        if (!socket_client->connected()) {
            return socketString;
        }

        mask[1] = timedRead();
        if (!socket_client->connected()) {

            return socketString;
        }

        mask[2] = timedRead();
        if (!socket_client->connected()) {
            return socketString;
        }

        mask[3] = timedRead();
        if (!socket_client->connected()) {
            return socketString;
        }

        for (i=0; i<length; ++i) {
            socketString += (char) (timedRead() ^ mask[i % 4]);
            if (!socket_client->connected()) {
                return socketString;
            }
        }
    }

    return socketString;
}

void WebSocketServer::disconnectStream() {
#ifdef DEBUGGING
    Serial.println(F("Terminating socket"));
#endif

    if (hixie76style) {
#ifdef SUPPORT_HIXIE_76
        // Should send 0xFF00 to server to tell it I'm quitting here.
        socket_client->write((uint8_t) 0xFF);
        socket_client->write((uint8_t) 0x00); 
#endif       
    } else {

        // Should send 0x8700 to server to tell it I'm quitting here.
        socket_client->write((uint8_t) 0x87);
        socket_client->write((uint8_t) 0x00);
    }   
    
    socket_client->flush();
    delay(10);
    socket_client->stop();
}

String WebSocketServer::getData() {
    String data;

    if (hixie76style) {
#ifdef SUPPORT_HIXIE_76
        data = handleHixie76Stream();
#endif
    } else {
        data = handleStream();
    }

    return data;
}

void WebSocketServer::sendData(const char *str) {
#ifdef DEBUGGING
    Serial.print(F("Sending data: "));
    Serial.println(str);
#endif
    if (socket_client->connected()) {
        if (hixie76style) {
            socket_client->write(0x00); // Frame start
            socket_client->print(str);
            socket_client->write(0xFF); // Frame end            
        } else {
            sendEncodedData(str);
        }         
    }
}

void WebSocketServer::sendData(String str) {
#ifdef DEBUGGING
    Serial.print(F("Sending data: "));
    Serial.println(str);
#endif
    if (socket_client->connected()) {
        if (hixie76style) {
            socket_client->write(0x00); // Frame start
            socket_client->print(str);
            socket_client->write(0xFF); // Frame end        
        } else {
            sendEncodedData(str);
        }
    }
}

void WebSocketServer::sendData(unsigned char byte) {
#ifdef DEBUGGING
    //Serial.print(F("Sending data: "));
    //Serial.println(str);
#endif
    if (socket_client->connected()) {
        if (hixie76style) {
            socket_client->write(0x00); // Frame start
            socket_client->write(byte);
            socket_client->write(0xFF); // Frame end        
        } else {
            sendEncodedData(byte);
        }
    }
}

void WebSocketServer::sendData(char *str, int size, unsigned char header) {

    if (socket_client->connected()) {
        if (hixie76style) {
            socket_client->write(0x00); // Frame start
            socket_client->print(str);
            socket_client->write(0xFF); // Frame end            
        } else {
            sendEncodedData(str,size,header);
        }         
    }
}

int WebSocketServer::timedRead() {
  while (!socket_client->available()) {
    delay(20);  
  }

  return socket_client->read();
}

void WebSocketServer::sendEncodedData(char byte) {
    int size = 1;

    // string type
    socket_client->write(0x82);

    // NOTE: no support for > 16-bit sized messages

    socket_client->write((uint8_t) size);

    socket_client->write(byte);

}

void WebSocketServer::sendEncodedData(char *str, int size, unsigned char header) {

    // string type
    socket_client->write(header);

    // NOTE: no support for > 16-bit sized messages
    if (size > 125) {
        socket_client->write(126);
        socket_client->write((uint8_t) (size >> 8));
        socket_client->write((uint8_t) (size & 0xFF));
        
    } else {
        socket_client->write((uint8_t) size);
    }

    //for (int i=0; i<size; ++i) {
        //socket_client->write(str[i]);
        socket_client->write((const uint8_t*)str,size);
    //}
}

void WebSocketServer::sendEncodedData(char *str) {
    int size = strlen(str);

    // string type
    socket_client->write(0x81);

    // NOTE: no support for > 16-bit sized messages
    if (size > 125) {
        socket_client->write(126);
        socket_client->write((uint8_t) (size >> 8));
        socket_client->write((uint8_t) (size & 0xFF));
    } else {
        socket_client->write((uint8_t) size);
    }

    for (int i=0; i<size; ++i) {
        socket_client->write(str[i]);
    }
}

void WebSocketServer::sendEncodedData(String str) {
    int size = str.length() + 1;
    char cstr[size];

    str.toCharArray(cstr, size);

    sendEncodedData(cstr);
}
