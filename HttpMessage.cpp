/*  @doc
 *  @module <HttpMessage.cpp> | <Brief description>
 *
 *  Copyright (C) 2016
 *  Author : Anjanamoorthi Thamaram Rajan
 *  mail :anjana2020@gmail.com
 *
 */

#include "HttpMessage.h"
#include "Utils.h"
#include "Helper.h"
#include "DbgLog.h"
#include <string.h>
#include <iostream>
#include <vector>
#include <algorithm>


static const std::string kHttpresponseCodes[] = { "200 OK", "400 Bad Request" };	/* TODO: server side error codes missing..	*/

HttpMessage::~HttpMessage(void)
{
	fHttpMsg[0] = '\0';
	fMsgHdr = std::string();
	fMsgBody = std::string();// TODO fix mem leaks...
	fMsgVec.clear();
    
	delete [] fMethod;
    delete [] fullUri;
    delete [] fUser;
    delete [] fPassword;
    delete [] fHost;
    delete [] fPort;
    delete [] fPath;
    delete [] fQString;
    delete [] fFragment;
    delete [] fProtocol;
    delete [] fMessage;
}

HttpMessage::HttpMessage(const uint8_t* msg, int32_t len)
		: kHttpResponseMsg(kHttpresponseCodes, std::end(kHttpresponseCodes))
		, fHttpMsg()
		, fMsgHdr{}
		, fMsgBody{}
        , fHttpMsgLen(0)
        , fMethod(new char[kMethodSize])
        , fURI(new char[kURISize])
        , fullUri(fURI)
        , fUser(new char[kUserSize])
        , fPassword(new char[kPasswordSize])
        , fHost(new char[kHostSize])
        , fPort(new char[kPortSize])
        , fPath(new char[kPathSize])
        , fQString(new char[kQStringSize])
        , fFragment(new char[kFragmentSize])
        , fProtocol(new char[kProtocolSize])
        , fMessage(new char[kMsgSize])
        , fCode(0)
		, fHeaderMap()
		, fMsgVec()
{
	if( len <= kHttpMsgMaxLength ) {
		::memcpy(fHttpMsg, msg, len);
		fHttpMsgLen = len;
		fHttpMsg[fHttpMsgLen] = '\0';
	}
	else {
		DbgLog(DBG_ERROR, "Out of Memeory .");
		Helper::Exit(1);
	}	//TODO: what is this??
}

bool HttpMessage::Append(const uint8_t* msg,int32_t len)
{
	if ( (fHttpMsgLen + len) <= kHttpMsgMaxLength ) { 
		::memcpy(fHttpMsg+fHttpMsgLen, msg,len);
		fHttpMsg[fHttpMsgLen+len] = '\0';
	}
	else {
		 DbgLog(DBG_ERROR, "Out of Memeory .");
                Helper::Exit(1);

	}
	return true;
}

uint32_t HttpMessage::ParseHTTPRequest(void)
{
	int i=0,k=0,contentLength=0;
	bool headerFound = false;
	uint8_t Msg[kHttpMsgMaxLength];
	uint8_t TmpMsg[kHttpMsgMaxLength];
	::memset(Msg, 0 ,kHttpMsgMaxLength);
	::memset(TmpMsg , 0, kHttpMsgMaxLength);
	::memcpy(Msg, fHttpMsg, fHttpMsgLen);

	while(true) {
		if ( (Msg[i] == '\r' && Msg[i+1] == '\n' && Msg[i+2] == '\r' && Msg[i+3] == '\n') || (Msg[i] == '\n' && Msg[i+1] == '\n' )  ) {
			TmpMsg[i] = '\0';
			fMsgHdr = ((char*)TmpMsg);
			headerFound =true;
			break;
		} else {
			if ( i != fHttpMsgLen ) {
				TmpMsg[i] = Msg[i];
				i++;
			} else {
				break;
			}
		}
	}

		if(headerFound) {
			i +=4;
			while(true) {
				if ( i != fHttpMsgLen ) {
					TmpMsg[k] = Msg[i];
					i++;k++;
				} else {
					TmpMsg[k] = '\0';
					fMsgBody = ((char*)TmpMsg); 
					break;
				}
			}
			contentLength = fMsgBody.length();
		} else { 
			fParseStatus = HttpMessage::HTTP_PARSE_NEED_MORE_DATA;
			 DbgLog(DBG_ERROR, "Http Parser : Need More Data ."); 
			return HttpMessage::HTTP_PARSE_NEED_MORE_DATA;
		}

		if (Parse()) {
			if ( contentLength >= atoi((GetHdrValue(kHTTPHeaderContentLength)).c_str()))  {
                                this->ParseMsgType();
                                fParseStatus = HttpMessage::HTTP_PARSE_SUCCESS;
                                return HttpMessage::HTTP_PARSE_SUCCESS;
                        } else {
                                fParseStatus = HttpMessage::HTTP_PARSE_NEED_MORE_DATA; 
                                return HttpMessage::HTTP_PARSE_NEED_MORE_DATA;
                        }	
		} else {
			fParseStatus = HttpMessage::HTTP_PARSE_FAIL; 
			return HttpMessage::HTTP_PARSE_FAIL;
		}
	
}

std::string HttpMessage::GetHttpBody(void)
{
	if ( fMsgBody != std::string() ) { 
		return fMsgBody;
	} else {
		return std::string();
	}
}	


std::string HttpMessage::GetHttpHeaderFull(void)
{
	return fMsgHdr;	
}

bool HttpMessage::Parse(void)
{
	bool fStatus;
	//std::string fMsg((char *)fMsgHdr);
	std::ostringstream err;
	if (!Utils::Split(fMsgHdr, "\r\n", fMsgVec, err)) {
		 DbgLog(DBG_ERROR, "CRLF is wrong in HTTP Message.");
		return false;
	}


	for (uint32_t i = 0; i < (uint32_t) fMsgVec.size() ; i++ ) {	
		if ( fMsgVec[i] != "" ) { 
			fStatus =ParseHttp(fMsgVec[i].c_str());
			if ( fStatus == false) {
				return false;
			}
		}
	}
	return true;
}

std::string HttpMessage::GetMethod(void)
{
	return std::string (fMethod);
}

bool HttpMessage::isHttpMessage(void)
{
	int i=0;
	uint8_t Msg[kHttpMsgMaxLength];
	uint8_t TmpMsg[kHttpMsgMaxLength];
	::memset(Msg, 0 ,kHttpMsgMaxLength);
    ::memset(TmpMsg , 0, kHttpMsgMaxLength);
	::memcpy(Msg, fHttpMsg, fHttpMsgLen);

	 while (Msg[i] != '\0' && Msg[i] != ' ' &&  Msg[i] != ':') {
                TmpMsg[i] = Msg[i];
                i++;
        }
        TmpMsg[i] = '\0';
	std::string methodName((char*)TmpMsg);
	if (!methodName.size())	{ 
		return false;
	}
	if ( Msg[i] == ' ' ) {
		if (methodName == "HTTP/1.1"
			|| methodName == "GET"
			|| methodName == "HTTP/1.0"
			|| methodName == "POST"
			|| methodName == "HEAD"
			|| methodName == "CONNECT"
			|| methodName == "DELETE"
			|| methodName == "OPTIONS"
			|| methodName == "TRACE"
			|| methodName == "PUT") {
		return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}	
	 

std::string HttpMessage::ParseMsgType(void)
{
	std::string Path = fPath; 
	if (Path[0] != '\0') {
		std::size_t typeSize = Path.find_last_of("/\\");
		std::string msgType = Path.substr(typeSize + 1);
		fMsgType = msgType;
		return msgType;
	}
	return Path; 

}

/* TODO: finding tenent id can be robust...	*/
bool HttpMessage::isServiceLookup(void)	
{
	std::string QString = fQString;
	if (QString != std::string() ) {
		std::size_t fPos = QString.find("TenantId");
		if ( fPos != std::string::npos ) {
			return true;
		}
	} 
	return false;
}
	
std::string HttpMessage::GetServiceName(void) const
{
	return fPath;
}

std::string HttpMessage::GetUri(void)
{
	std::string uri = fURI;
	uri = "/" + uri;
	return uri;
}

std::string HttpMessage::GetTenantId(void) const
{
	std::string QString = fQString;
	std::size_t spos = QString.find_first_of("TenantId");
	std::size_t epos = QString.find_first_of("&");
	if (spos != std::string::npos && epos != std::string::npos) {
		std::string fTenantId = QString.substr(spos, epos);
		std::size_t pos = fTenantId.find("=");
		if ( pos != std::string::npos ) {
			std::string fResult = fTenantId.substr(pos+1);	 
			return fResult;
		} else {
			return std::string();
		}
	} else {
		return std::string();
	}
}

bool HttpMessage::ParseHttp(const char*  msg)
{
        int i = 0;
        int len = 0;
        char fTmpMethod[256];
        len = strlen(msg); 
        int uriStart=0,uriEnd=0,uriLength=0;
        while (msg[i] != '\0' && msg[i] != ' ' &&  msg[i] != ':') {
                fTmpMethod[i] = msg[i];
                i++;
        }
        fTmpMethod[i] = '\0';
        std::string fMethodName =msg;
        if (msg[i] == '\0')
                return false; //Wrong Format!!
        uriStart = i;
        if (msg[i] == ':' && msg[i+1] == ' ') {    // Received a Header Line
                bool result =ParseHeaders(msg);
                return result;
        }
        fMethodName = fTmpMethod;
        std::size_t pos = fMethodName.find("HTTP");
        if ( pos != std::string::npos ) {      // Received a Response
                bool result=ParseHttpResponseFirstLine(msg);
                return result;
        }
        fMethodName = fTmpMethod;
        if ( (fMethodName == "GET") || (fMethodName == "POST")
                || (fMethodName == "HEAD")
                || (fMethodName == "PUT")
                || (fMethodName == "DELETE")
                || (fMethodName == "OPTIONS")
                || (fMethodName == "TRACE")
                || (fMethodName == "CONNECT") ) { // Received a Request
				strcpy(fMethod, fMethodName.c_str());	//Fixed dangling pointer issue.
                uriStart = i;
                while (*msg == ' ') {
                        ++msg;
                        --len;
                }
                // search from the end(!) for the space that seperates the url from the protocol string
                i = len - 1;
                while (i > 0 && msg[i] != ' ') {
                         i--;
                }
                uriEnd = i-1;
		if (i == uriStart) {
                        uriEnd = len-1;
                } else {
                /*      uint32_t sPos = ++uriEnd;
                        std::string httpVerStr =fMethodName.substr(sPos);
                        std::size_t pos = httpVerStr.find_first_of("HTTP/");
                        if ( pos != std::string::npos ) {
                                std::size_t spos = pos+5;
                                std::size_t epos = httpVerStr.find_first_of(".");
                                if ( epos == std::string::npos ) {
                                        return false;
                                } else {
                                        std::string version = httpVerStr.substr(spos, epos-1);
                                        if ( std::stoi(version) < 0 ) {
                                                return false;
                                        }
                                        version = httpVerStr.substr(epos+1,httpVerStr.length()-1);
                                        if ( std::stoi(version) < 0 ) {
                                                return false;
                                        }
                                }
                        } else {
                                return false;
                        } */
                }


                uriLength = uriEnd - uriStart;
                bool result = ParseHttpRequestFirstLine(msg,uriLength,uriStart,uriEnd);
                return result;
        }
        return true;
}

bool HttpMessage::ParseHeaders(const char* headerMsg)
{
        std::string fToken(headerMsg);
	std::vector<std::string> fTmpVec;
	 std::ostringstream err;
	if (!Utils::Split(fToken, ": ", fTmpVec, err)) {
                return false;
        }

        std::string key = fTmpVec[0];
	std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return std::tolower(c); });
        std::string value = fTmpVec[1];
        fHeaderMap.insert(fPair(key, value));

        return true;
}


std::string HttpMessage::GetHdrValue(const std::string & fheader)
{
	fHeaderCit tmpIt = fHeaderMap.begin();
	while ( tmpIt != fHeaderMap.end() ) {
		if ( tmpIt->first == fheader ) {
			return tmpIt->second;
		}
		tmpIt++;
	}
	return std::string();
}

bool HttpMessage::ParseHttpResponseFirstLine(const char* responseMsg)
{
        std::istringstream ss(responseMsg);
        std::string token;
        std::vector<std::string> fResponseVector;
        while(std::getline(ss, token, ' ')) {
                fResponseVector.push_back(token);
        }
        fProtocol = fResponseVector[0].c_str();

        fCode = atoi( fResponseVector[1].c_str());
        fMessage = fResponseVector[2].c_str();
        return true;
}

bool HttpMessage::ParseHttpRequestFirstLine(const char* requestMsg, int uriLength,int uriStart, int uriEnd)
{
        int i=0;
        for (i = 0; i < uriLength; i++ ) {
                fURI[i] = requestMsg[++uriStart];
        }
        fURI[i]='\0';
        //Read the Scheme from URI
		/*
		char* fScheme = strchr (fURI,':');
        if ( fScheme == NULL ) {
                return false;
        }
        fScheme++; 
        fURI = fScheme;
        for ( i=0; i<2; i++) {
                if ( '/' != *(fURI))
                                return false;
                (fURI)++;
        }
		*/

	if ( '/' != *(fURI))
		return false;
	(fURI)++;
	
        i=0;
        char* tmpURI = fURI;
        //Verify for the optional User name and Password
        while ( '\0' != *(tmpURI) && ':' != *(tmpURI) && '@' != *(tmpURI) ) {
                fUser[i++] = *(tmpURI) ;
                (tmpURI)++;
        }
                fUser[i] = '\0';
        if ( *(tmpURI) == ':' ) {
                (tmpURI)++;
        }
        i=0;
        while (  '\0' != *(tmpURI) && '@' != *(tmpURI) ) {
                fPassword[i++] = *(tmpURI);
                (tmpURI)++;
        }
                fPassword[i] = '\0';
        if ( *(tmpURI) == '@' ) {
                (tmpURI)++;
        } else {
                fUser[0] = '\0';
                fPassword[0] = '\0';
                tmpURI = fURI;
        }
        i=0;
        // Host Address Specification
        while ( '\0' != *(tmpURI) && ':' != *(tmpURI) ) { 
                fHost[i++] = *(tmpURI);
                (tmpURI)++;
        }
        fHost[i] = '\0';
         if ( *(tmpURI) == ':' ) {
                (tmpURI)++;

        i=0;
        while ( '\0' != *(tmpURI) && '/' != *(tmpURI) ) {
                fPort[i++] = *(tmpURI);
                (tmpURI)++;
        }
        fPort[i] = '\0';
	} else { 
		tmpURI = fURI;
	}
        // End of the String
         if ( '\0' == *(tmpURI) ) {
                return true;
        }
	 // skip / symbol
        if ( '/' == *(tmpURI) ) {
                (tmpURI)++;
        }
        i=0;
        // parse path
        while ( '\0' != *(tmpURI) && '#' != *(tmpURI)  && '?' != *(tmpURI) ) {
                fPath[i++] = *(tmpURI) ;
                (tmpURI)++;
        }
                fPath[i] = '\0'; 
        // Query String
        if ( '?' == *(tmpURI) ) {
                (tmpURI)++;
        }
        i=0;
        while ( '\0' != *(tmpURI) && '#' != *(tmpURI ) ) {
                fQString[i++] = *(tmpURI);
                (tmpURI)++;
        }
                fQString[i] = '\0';
        // Fragement Specification
        if ( '#' == *(tmpURI) ) {
                (tmpURI)++;
        }
        i=0;
        while ( '\0' != *(tmpURI) ) {
                fFragment[i++] = *(tmpURI);
                (tmpURI)++;
        }
        fFragment[i] = '\0';
        return true;
}


/* HTTP Request or Response Generator */
std::string HttpMessage::GenerateHttpMsg(const std::string & JsonMsg,const std::string & MsgType)
{
//Request:
//Response:
	if ( MsgType == "Response" ) {
		std::ostringstream out_data;			
		std::size_t Pos = JsonMsg.find("\"ResultCode\": 2000");
		if ( Pos != std::string::npos ) {	
			out_data << "HTTP/1.1 ";
			for( auto tmp : kHttpresponseCodes ) {
				std::size_t pos = tmp.find("200");
				if ( pos != std::string::npos ) {	
					out_data << tmp << "\r\n";
				}
			}
			out_data << kHTTPHeaderContentLength << ": " << JsonMsg.length() << "\r\n\r\n";
			out_data << JsonMsg;
		} else {
			out_data << "HTTP/1.1 ";
			for(auto tmp : kHttpresponseCodes) {
				std::size_t pos = tmp.find("400");
				if ( pos != std::string::npos ) { 
					out_data << tmp << "\r\n";
				}
			}
			out_data << kHTTPHeaderContentLength << ": " << JsonMsg.length() << "\r\n\r\n";
			out_data << JsonMsg;
		}
		return out_data.str();
	}
	
	return std::string();
}


/* HTTP Request or Response Generator */
std::string HttpMessage::GenerateHttpMsg(const std::string & JsonMsg, const std::string & MsgType, const uint32_t connectionID)
{
	//Request:
	//Response:
	if (MsgType == "Response") {
		std::ostringstream out_data;
		std::size_t Pos = JsonMsg.find("\"ResultCode\": 2000");
		if (Pos != std::string::npos) {
			out_data << "HTTP/1.1 ";
			for (auto tmp : kHttpresponseCodes) {
				std::size_t pos = tmp.find("200");
				if (pos != std::string::npos) {
					out_data << tmp << "\r\n";
				}
			}
			/* commented as gsm agent is synchronous for now. */
			//out_data << kHTTPHeaderConnectionID << ": " << connectionID << "\r\n";	//connection identifier since gsm agent is async...
			out_data << kHTTPHeaderContentLength << ": " << JsonMsg.length() << "\r\n\r\n";
			out_data << JsonMsg;
		}
		else {
			out_data << "HTTP/1.1 ";
			for (auto tmp : kHttpresponseCodes) {
				std::size_t pos = tmp.find("400");
				if (pos != std::string::npos) {
					out_data << tmp << "\r\n";
				}
			}
			/* commented as gsm agent is synchronous for now. */
			//out_data << kHTTPHeaderConnectionID << ": " << connectionID << "\r\n";	//connection identifier since gsm agent is async...
			out_data << kHTTPHeaderContentLength << ": " << JsonMsg.length() << "\r\n\r\n";
			out_data << JsonMsg;
		}
		return out_data.str();
	}

	return std::string();
}


