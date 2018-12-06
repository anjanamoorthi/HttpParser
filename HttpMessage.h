/*  @doc
 *  @module <HttpMessage.h> | <Brief description>
 *
 *  Copyright (C) 2017
 *  *  Author : Anjanamoorthi Thamaram Rajan
 *  mail :anjana2020@gmail.com
 *
 */

#ifndef __HttpMessage_h__
#define __HttpMessage_h__

#include "DataTypes.h"

#include <mutex>
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>

// Http headers
static const std::string kHTTPHeaderAccept="accept";
static const std::string kHTTPHeaderAcceptEncoding="accept-encoding";
static const std::string kHTTPHeaderAcceptLanguage="accept-language";
static const std::string kHTTPHeaderAuthorization="authorization";
static const std::string kHTTPHeaderCacheControl="cache-control";
static const std::string kHTTPHeaderConnection="connection";
static const std::string kHTTPHeaderContentDisposition="content-disposition";
static const std::string kHTTPHeaderContentEncoding="content-encoding";
static const std::string kHTTPHeaderContentLength="content-length";
static const std::string kHTTPHeaderContentRange="content-range";
static const std::string kHTTPHeaderContentLocation="content-location";
static const std::string kHTTPHeaderContentType="content-type";
static const std::string kHTTPHeaderCookie="cookie";
static const std::string kHTTPHeaderDate="date";
static const std::string kHTTPHeaderETag="etag";
static const std::string kHTTPHeaderExpect="expect";
static const std::string kHTTPHeaderExpires="expires";
static const std::string kHTTPHeaderHost="host";
static const std::string kHTTPHeaderKeepAlive="keep-alive";
static const std::string kHTTPHeaderLastModified="last-modified";
static const std::string kHTTPHeaderPragma="pragma";
static const std::string kHTTPHeaderProxyAuthenticate="proxy-authenticate";
static const std::string kHTTPHeaderProxyAuthorization="proxy-authorization";
static const std::string kHTTPHeaderProxyConnection="proxy-connection";
static const std::string kHTTPHeaderReferer="referer";
static const std::string kHTTPHeaderSetCookie="set-cookie";
static const std::string kHTTPHeaderTransferEncoding="transfer-encoding";
static const std::string kHTTPHeaderVary="vary";
static const std::string kHTTPHeaderWWWAuthenticate="www-authenticate";
static const std::string kHTTPHeaderXCache="x-cache";
static const std::string kHTTPHeaderXForwardedFor="x-forwarded-for";
static const std::string kHTTPHeaderXHeader="x--header";
static const std::string kHTTPHeaderConnectionID = "connection-id";

static const std::string kVersion = "//V1/";
static const std::string kRegisterServiceID = "RegisterService";
static const std::string kDeregisterServiceID = "DeregisterService";
static const std::string kHealthcheckServiceID = "HealthcheckService";
static const std::string kChangeLogLevel = "ChangeLogLevel";
static const std::string kStatus = "Status";
static const std::string kServices = "GetServices";
static const std::string kTenantId = "TenantId";
static const std::string kReloadAuthInfo = "ReloadAuthInfo"; 
static const std::string kInfoPsk = "/info/psk"; 
static const std::string kIdentifyToken = "/IdentityServer/core/connect/token"; 
static const std::string kRegisterScope = "GSM.Registration";
static const std::string kDeregisterScope = "GSM.Deregistration";


static const int32_t kHttpMsgMaxLength = (1048576);	
static const int32_t kMethodSize = (1024);
static const int32_t kURISize = (8196);
static const int32_t kUserSize = (1024);
static const int32_t kPasswordSize = (1024);
static const int32_t kPathSize = (1024);
static const int32_t kHostSize = (1024);
static const int32_t kPortSize = (1024);
static const int32_t kQStringSize = (1024);
static const int32_t kFragmentSize = (1024);
static const int32_t kProtocolSize = (1024);
static const int32_t kMsgSize = (1024);

/**
* class for Http Message Handler 
*/
class HttpMessage
{
public:
        friend class HttpMessageTest;
	friend class CCsbExtReqResMgr;

	/**
        * parse an incoming Http header field and value 
        * and store it in a Map 
        * @param msg incoming Http Header Line 
        * @return bool true or false
        */

	/**
	* default constructor
	*/	
        HttpMessage(void)
			: fHttpMsg()
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
		, fParseStatus(HTTP_INITIALIZED)
		, fMsgType() {}

	 /**
        * default destructor
        */
        ~HttpMessage(void);

	/**
        * Get HTTP Message in  HttpMessage
        */
        HttpMessage(const uint8_t*,int32_t);

		enum HttpStatus
		{
			HTTP_INITIALIZED,
			HTTP_PARSE_SUCCESS,
			HTTP_PARSE_FAIL,
			HTTP_PARSE_NEED_MORE_DATA
		};


	bool isHttpMessage(void);

	/**
        * Get HTTP Message in  HttpMessage
	* @param uint8_t* HTTP Message
	* @param uint32_t HTTP Message length
	* @return bool true or false 
        */
        bool Append(const uint8_t*,int32_t);

	/**
	* Get Status of given HTTP Message
	* @return enum  SUCCESS , NEED_MORE_DATA or FAIL
	*/
	uint32_t ParseHTTPRequest(void);

	bool isServiceLookup(void);
	std::string GetTenantId(void) const;
	std::string GetServiceName(void) const;
	std::string GetUri(void);
	std::string GetHdrValue(const std::string & fheader);
	HttpStatus GetParseStatus()	{ return fParseStatus; };

	std::string GetMethodString()	{
		return fMsgType;
	}

	/**
        * Parse given HTTP Message
        * @return bool true or false 
        */
        bool Parse(void);

	/**
	* Get HTTP Message Body in  HttpMessage
	*/
		std::string GetHttpBody(void);	
	
		/**
		* Get HTTP Header till \r\n\r\n (inclusive)
		*/
		std::string GetHttpHeaderFull(void);

	/**
        * Get Method name of a given HTTP Message
        * @return string Method Name 
        */
        std::string GetMethod(void);

	/**
        * Get Message Type of a given HTTP Message
        * @return string Message Type 
        */
		std::string GetMsgType()	{
			return fMsgType;
		};


		std::string ParseMsgType(void);

	/**
	* Generates Http Request/Response Messages 
	* @return string Http Message 
	*/
        std::string GenerateHttpMsg(const std::string & fJsonMsg, const std::string & fMsgType);
		std::string GenerateHttpMsg(const std::string & JsonMsg, const std::string & MsgType, const uint32_t connectionID);		
	
	/**
        * parse an incoming Http Request,Response and
        * header line
        * @param msg incoming Http Message
        * @return bool true or false
        */
        bool ParseHttp(const char*);

        /**
        * parse an incoming Http header field and value
        * and store it in a Map
        * @param msg incoming Http Header Line
        * @return bool true or false
        */
        bool ParseHeaders(const char*);

        /**
        * parse an incoming Http request first Line
        * @param msg incoming Http Request Line
        * @param uriStart Starting point of URI
        * @param uriEnd End position of URI
        * @param uriLength length of given URI
        * @return bool true or false
        */
        bool ParseHttpRequestFirstLine(const char*, int, int, int );

         /**
        * parse an incoming Http response line
        * @param msg incoming Http response Line
        * @return bool true or false
        */
        bool ParseHttpResponseFirstLine(const char*);

		
private: //member functions

	/**
        * copy constructor
	* @param HttpMessage class object reference
        */
        HttpMessage(const HttpMessage&);
        HttpMessage& operator= (const HttpMessage&);

private: //member data

	//Http Response Codes
	std::vector<std::string> kHttpResponseMsg;
	
	uint8_t	fHttpMsg[kHttpMsgMaxLength];
	std::string fMsgHdr;
	std::string fMsgBody; 
	uint32_t	fHttpMsgLen;
		char*   fMethod;
        char*   fURI;
        char*   fullUri;
        char*   fUser;
        char*   fPassword;
        char*   fHost;
        char*   fPort;
        char*   fPath;
        char*   fQString;
        char*   fFragment;
        const char*     fProtocol;
        const char*     fMessage;
        int32_t fCode;

	typedef std::map<std::string, std::string>      fHeaderPair;
        typedef std::pair<std::string, std::string> fPair;
        typedef fHeaderPair::iterator   fHeaderIt;
        typedef fHeaderPair::const_iterator     fHeaderCit;
	fHeaderPair fHeaderMap;	
	std::vector<std::string> fMsgVec;

	HttpStatus fParseStatus;
	std::string fMsgType;


}; // HttpMessage

typedef boost::shared_ptr<HttpMessage> HttpMessagePtr;

#endif // __HttpMessage_h__


