#include "util/tc_http.h"
#include "util/tc_common.h"
#include "util/tc_clientsocket.h"

namespace xutil
{

bool XC_URL::isValid() const
{
    return !_sURL.empty();
}

string XC_URL::getURL() const
{
    return _sURL;
}

string XC_URL::type2String() const
{
    switch(_iURLType)
    {
    case HTTP:
        return "http";
    case HTTPS:
        return "https";
    case FTP:
        return "ftp";
    }

    return "http";
}

string XC_URL::getDefaultPort() const
{
    switch(_iURLType)
    {
    case HTTP:
        return "80";
    case HTTPS:
        return "433";
    case FTP:
        return "21";
    }

    return "80";
}

bool XC_URL::isDefaultPort() const
{
    return getFragment("port") == getDefaultPort();
}

string XC_URL::toURL() const
{
    string sUserName    = getFragment("user");
    string sPassword    = getFragment("pass");
    string sDomain      = getFragment("domain");
    string sPort        = getFragment("port");
    string sUrlPath     = getFragment("path");
    string sUrlQuery    = getFragment("query");
    string sUrlRef      = getFragment("ref");

    string sURL = type2String() + "://";

    if(!sUserName.empty())
        sURL += sUserName;

    if(!sUserName.empty() && !sPassword.empty())
        sURL += ":" + sPassword;

    if(!sUserName.empty())
        sURL += "@";

    sURL += sDomain;

    if(!isDefaultPort())
        sURL += ":" + sPort;

    sURL += getRequest();

    return sURL;
}

string XC_URL::getRequest() const
{
    string sURL;

    string sUrlPath     = getFragment("path");

    string sUrlQuery    = getFragment("query");

    string sUrlRef      = getFragment("ref");

    if(!sUrlPath.empty())
        sURL += sUrlPath;

    if(!sUrlQuery.empty())
        sURL += "?" + sUrlQuery;

    if(!sUrlRef.empty())
        sURL += "#" + sUrlRef; 

    return sURL;    
}

bool XC_URL::parseURL(const string &sURL)
{
    string originRequest  = XC_Common::trim(sURL, " ");

    if(originRequest.empty())
    {
        return false;
    }

    clear();

	string sUrlPath;

	if(strncasecmp(originRequest.c_str(), "http://" ,7) == 0)
	{
        //http开头
        _iURLType  = HTTP;
		sUrlPath   = originRequest.substr(7);
	}
    else if(strncasecmp(originRequest.c_str(), "https://" ,8) == 0)
    {
        //https开头
        _iURLType  = HTTPS;
        sUrlPath   = originRequest.substr(8);
    }
    else if(strncasecmp(originRequest.c_str(), "ftp://", 6) == 0)
    {
        //ftps开头
        _iURLType  = FTP;
        sUrlPath   = originRequest.substr(6);
    }
    else
    {
        //默认用http
        _iURLType  = HTTP;
        sUrlPath   = originRequest;
    }

    //完善url, www.qq.com?a=b ==> www.qq.com/?a=b
    //完善url, www.qq.com#abc ==> www.qq.com/#a=b
	string::size_type index = sUrlPath.find("/");
    if(index == string::npos)
    {
        index = sUrlPath.find("?");
        if(index != string::npos)
        {
            sUrlPath.insert(index, "/");
        }
        else
        {
            index = sUrlPath.find("#");

            if(index != string::npos)
                sUrlPath.insert(index, "/");
        }
    }

    //获取authority, urlPath去掉host部分
    string sUrlAuthority;
    index = sUrlPath.find("/");
	if(index == string::npos)
	{
		sUrlAuthority= sUrlPath.substr(0, index);
		sUrlPath     = "";
	}
	else
	{
		sUrlAuthority= sUrlPath.substr(0, index);
		sUrlPath     = sUrlPath.substr(index);
	}

    string sUrlQuery;

    index = sUrlPath.find("?");
    if(index != string::npos)
    {
        sUrlQuery = sUrlPath.substr(index + 1);

        sUrlPath  = sUrlPath.substr(0, index);
    }

    string sUrlRef;

    index = sUrlQuery.rfind("#");
    if(index != string::npos)
    {
        sUrlRef     = sUrlQuery.substr(index + 1);

        sUrlQuery   = sUrlQuery.substr(0, index);
    }

    if(sUrlPath.empty())
    {
        sUrlPath = "/";
    }

    string sUrlUserInfo;
    string sUrlHost;
    string sUserName;
    string sPassword;
    string sDomain;
    string sPort;

    //////解析Authority
    index = sUrlAuthority.find("@");
    if(index != string::npos)
    {
        sUrlHost    = sUrlAuthority.substr(index + 1);

        sUrlUserInfo= sUrlAuthority.substr(0, index);
    }
    else
    {
        sUrlHost    = sUrlAuthority;
    }

    //////解析User:Pass
    index = sUrlUserInfo.find(":");
    if(index != string::npos)
    {
        sPassword   = sUrlUserInfo.substr(index + 1);

        sUserName   = sUrlUserInfo.substr(0, index);
    }
    else
    {
        sUserName   = sUrlUserInfo;
    }

    //////解析sUrlHost:Port
    index = sUrlHost.find(":");
    if(index != string::npos)
    {
        sPort       = sUrlHost.substr(index + 1);

        sDomain     = sUrlHost.substr(0, index);
    }
    else
    {
        sPort       = getDefaultPort();

        sDomain     = sUrlHost;
    }

    _fragment["scheme"] = type2String();
    _fragment["user"]   = sUserName;
    _fragment["pass"]   = sPassword;
    _fragment["domain"] = XC_Common::lower(sDomain);
    _fragment["port"]   = sPort;
    _fragment["path"]   = sUrlPath;
    _fragment["query"]  = sUrlQuery;
    _fragment["ref"]    = sUrlRef;

    _sURL = toURL();

    //域名或者IP必须包含一个点
    if(sDomain.find(".") == string::npos)
    {
        return false;
    }

    return true;
}

void XC_URL::specialize()
{
    //规整化路径
    string sUrlPath = simplePath(getPath());

    _fragment["path"]   = sUrlPath;

    _sURL = toURL();
}

void XC_URL::clear()
{
    _fragment.clear();

    _sURL.clear();
}

string XC_URL::getScheme() const
{
    return type2String();
}

string XC_URL::getUserName() const
{
    return getFragment("user");
}

string XC_URL::getPassword() const
{
    return getFragment("pass");
}

string XC_URL::getDomain() const
{
    return getFragment("domain");
}

string XC_URL::getPort() const
{
    return getFragment("port");
}

string XC_URL::getPath() const
{
    return getFragment("path");
}

string XC_URL::getQuery() const
{
    return getFragment("query");
}

string XC_URL::getRef() const
{
    return getFragment("ref");
}

string XC_URL::getFragment(const string& frag) const
{
    map<string, string>::const_iterator it = _fragment.find(frag);
    if(it == _fragment.end())
    {
        return "";
    }

    return it->second;
}

string XC_URL::getRelativePath() const
{
    string sURL = getPath();

	string::size_type pos;

	pos   = sURL.rfind("/");

	if(pos == string::npos)
	{
        return "/";
	}

    string sRelative;

	if(pos != 0)
	{
		sRelative = sURL.substr(0, pos + 1);
	}
	else
	{
		sRelative = sURL;
	}

    return sRelative;
}

string XC_URL::getRootPath() const
{
    string sUserName    = getFragment("user");
    string sPassword    = getFragment("pass");
    string sDomain      = getFragment("domain");
    string sPort        = getFragment("port");

    string sURL = type2String() + "://";

    if(!sUserName.empty())
        sURL += sUserName;

    if(!sUserName.empty() && !sPassword.empty())
        sURL += ":" + sPassword;

    if(!sUserName.empty())
        sURL += "@";

    sURL += sDomain;

    if(!isDefaultPort())
        sURL += ":" + sPort;

    sURL += "/";

    return sURL;
}

XC_URL XC_URL::buildWithRelativePath(const string &sRelativePath) const
{
    string sURL;

	if(!sRelativePath.empty() && sRelativePath[0] == '/')
	{
	    sURL = sRelativePath.substr(1); //如果链接是用"/"开头的相对地址，那么应该用Host+相对地址
	}
	else if(sRelativePath[0] == '#')
	{
		//#
		sURL = getPath().substr(1);

        if(!getQuery().empty())
            sURL += "?" + getQuery();

        sURL += sRelativePath;

	}
	else
	{
		//相对地址
		sURL = getRelativePath().substr(1) + sRelativePath;
	}

    sURL = getRootPath() + simplePath("/" + sURL).substr(1);

    XC_URL url;

    url.parseURL(sURL);

    return url;
}

string XC_URL::simplePath(const string &sPath) const
{
    //所有./都去掉
    size_t pos      = 0;
	string sNewPath = sPath;

    while (true)
    {
        size_t dotPos = sNewPath.find("./", pos);

        if (dotPos != string::npos)
        {
            if ((dotPos == 0) || (sNewPath.at(dotPos - 1) == '/'))
            {
                sNewPath.erase(dotPos, 2);
            }
            else
            {
                pos = dotPos + 2;
            }
        }
        else
        {
            break;
        }
    }

    //如果路径是以.结尾的, 则.去掉
    if (((sNewPath.length() >= 2) && (sNewPath.substr(sNewPath.length()-2) == "/.")) || (sNewPath == "."))
    {
        sNewPath.erase(sNewPath.length() - 1);
    }

    //处理/../的形式
    pos = 0;
    size_t startPos = 0;

    while (1)
    {
        size_t slashDot = sNewPath.find("/../", pos);

        if (slashDot != string::npos)
        {
            if (0 == slashDot)
            {
                sNewPath.erase(0, 3);
                continue;
            }

            if ( (slashDot > 1) && (sNewPath.substr(slashDot - 2, 2) == "..") )
            {
                pos = slashDot + 4;
                continue;
            }

            startPos = sNewPath.rfind('/', slashDot - 1);

            if (startPos == string::npos) startPos = 0;

            sNewPath.erase(startPos, slashDot + 4 - startPos - 1);
        }
        else
        {
            break;
        }
    }

    //处理/..结尾的情况
    if ((sNewPath.size() >= 3) && (sNewPath.substr(sNewPath.size() - 3, 3) == "/.."))
    {
        size_t slashDot = sNewPath.size() - 3;
        if (!((slashDot > 1) && (sNewPath.substr(slashDot - 2, 2) == "..")))
        {
            startPos = sNewPath.rfind ('/', slashDot - 1);
            if (startPos == string::npos) startPos = 0;
            sNewPath.erase (startPos + 1);
        }
    }

	return sNewPath;
}

////////////////////////////////////////////////////////////////////

string XC_Http::getHeader(const string& sHeader) const
{
    http_header_type::const_iterator it = _headers.find(sHeader);
    if(it == _headers.end())
    {
        return "";
    }

    return it->second;
}

string XC_Http::getContentType() const
{
    return getHeader("Content-Type");
}

string XC_Http::getHost() const
{
    return getHeader("Host");
}

size_t XC_Http::getContentLength() const
{
    string s = getHeader("Content-Length");
    if(s.empty())
    {
        return 0;
    }

    return XC_Common::strto<size_t>(s);
}

string XC_Http::genHeader() const
{
	ostringstream sHttpHeader;

	for(http_header_type::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
	{
        if(it->second != "")
        {
            sHttpHeader << it->first << ": " << it->second << "\r\n";
        }
	}

	return sHttpHeader.str();
}

vector<string> XC_Http::getHeaderMulti(const string &sHeadName) const
{
    vector<string> v;

    http_header_type::const_iterator itEnd = _headers.end();

    for( http_header_type::const_iterator it = _headers.begin(); it != itEnd; ++it)
    {
        if(strcasecmp(it->first.c_str(), sHeadName.c_str()) == 0)
        {
            v.push_back(it->second);
        }
    }
    
    return v;
}

string XC_Http::getLine(const char** ppChar)
{
	string sTmp;

	sTmp.reserve(512);

	while((**ppChar) != '\r' && (**ppChar) != '\n' && (**ppChar) != '\0')
	{
		sTmp.append(1, (**ppChar));
		(*ppChar)++;
	}

	if((**ppChar) == '\r')
    {
		(*ppChar)++;   /* pass the char '\n' */
    }

	(*ppChar)++;

	return sTmp;
}


string XC_Http::getLine(const char** ppChar, int iBufLen)
{
	string sTmp;

	sTmp.reserve(512);

	int iCurIndex= 0;
	while( (**ppChar) != '\r' && (**ppChar) != '\n' && (**ppChar) != '\0')
	{
		if ( iCurIndex < iBufLen )
		{
			sTmp.append(1, (**ppChar));
			(*ppChar)++;
			iCurIndex++;
		}
		else
		{
			//MTT_ERRDAY << "parseHttp WARN: iCurIndex < iBufLen 1 " << endl;
			break;
		}
	}

	if( (**ppChar) == '\r')
	{
		if ( iCurIndex < iBufLen )
		{
			(*ppChar)++;   /* pass the char '\n' */
			iCurIndex++;
		}
		else
		{
			//MTT_ERRDAY << "parseHttp WARN: iCurIndex < iBufLen 2 " << endl;
		}
	}

	if( iCurIndex < iBufLen )
	{
		(*ppChar)++;
		iCurIndex++;
	}
	else
	{
		//MTT_ERRDAY << "parseHttp WARN: iCurIndex < iBufLen 3 " << endl;
	}

	return sTmp;
}


const char* XC_Http::parseHeader(const char* szBuffer, http_header_type &sHeader)
{
    sHeader.clear();

	const char **ppChar = &szBuffer;

	while(true)
	{
		string sLine = getLine(ppChar);

		if(sLine.empty()) break;

        //如果是第一行, 则忽略掉
        if(strncasecmp(sLine.c_str(), "GET ", 4) ==0 
           || strncasecmp(sLine.c_str(), "POST ", 5) ==0
           || strncasecmp(sLine.c_str(), "HTTP/", 5) ==0)
        {
            continue;
        }

		string::size_type index = sLine.find(":");
		if(index != string::npos)
		{                                                          
            sHeader.insert(multimap<string, string>::value_type(XC_Common::trim(sLine.substr(0, index), " "), XC_Common::trim(sLine.substr(index + 1), " ")));
		}
	}

	return *ppChar;
}

void XC_Http::reset()
{
    _headers.clear();
    _headLength = 0;
    _content.clear();
    _bIsChunked = false;
}

/********************* XC_HttpCookie ***********************/

bool XC_HttpCookie::matchDomain(const string &sCookieDomain, const string &sDomain)
{
    string sCookieDomainNew = XC_Common::lower(sCookieDomain);

    //没有点的自动加点
    if(sCookieDomainNew.find(".") != 0)
    {
        sCookieDomainNew = "." + sCookieDomainNew;
    }

    string::size_type pos = sCookieDomainNew.find(".");

    //sCookieDomain串至少有两个点
    if(pos == string::npos || (pos == 0 && sCookieDomainNew.rfind(".") == 0)) 
    {
        return false;
    }

    string sLowerDomain = XC_Common::lower(sDomain);

    //后边是子域名
    if(sDomain.length() >= sCookieDomainNew.length() && 
       sLowerDomain.compare(sDomain.length() - sCookieDomainNew.length(), sCookieDomainNew.length(), sCookieDomainNew) == 0)
    {
        return true;
    }

    //去掉.相等
    if(sLowerDomain == sCookieDomainNew.substr(1))
    {
        return true;
    }

    return false;
}

size_t XC_HttpCookie::matchPath(const string &sCookiePath, const string &sPath)
{
    if(sCookiePath.empty() || sPath.empty()) return 0;

    //都在最后加/再匹配
    string sCookiePath1 = (sCookiePath.at(sCookiePath.length() - 1) == '/') ? sCookiePath:sCookiePath + "/";

    string sPath1 = (sPath.at(sPath.length() - 1) == '/') ? sPath:sPath + "/";

    if(sPath1.find(sCookiePath1) == 0)
    {
        return sCookiePath1.length();
    }

    return 0;
}

void XC_HttpCookie::clear()
{
    _cookies.clear();
}

bool XC_HttpCookie::fixDomain(const string &sDomain, string &sFixDomain)
{
    if(sDomain.empty())
    {
        return false;
    }

    sFixDomain = sDomain;

    //自动加.
    if(sDomain.at(0) != '.')
    {
        sFixDomain = "." + sDomain;
    }

    //domain至少两段
    if(sFixDomain.find(".") == sFixDomain.rfind("."))
        return false;
    
    return true;
}

void XC_HttpCookie::addCookie(const Cookie &cookie, list<Cookie> &cookies)
{
    string sDomain;

    Cookie cookieNew = cookie;

    if(!fixDomain(cookieNew._domain, sDomain))
        return;

    cookieNew._domain = sDomain;

    if(cookieNew._path.empty())
        return;

    list<Cookie>::iterator it = cookies.begin();

    string sName; 
    if(cookieNew._data.size() >= 1)
    {
        sName = cookieNew._data.begin()->first;
    }

    while(it != cookies.end())
    {
        //检查Cookie是否过期
        if(isCookieExpires(*it))
        {
            cookies.erase(it++);
        }
        else if(strcasecmp(it->_domain.c_str(), cookieNew._domain.c_str()) == 0
            && strcmp(it->_path.c_str(), cookieNew._path.c_str()) == 0 
            && it->_isSecure == cookieNew._isSecure)
        {
            if(it->_expires == cookieNew._expires)
            {
                //domain/path/expires都匹配的情况下, 覆盖老cookie的数据
                cookieNew._data.insert(it->_data.begin(), it->_data.end());

                cookies.erase(it++);
            }
            else
            {
                //超时间不一样,但存在同样的key，需删除
                if(it->_data.find(sName) != it->_data.end())
                {
                    it->_data.erase(sName);
                }
                ++it;
            }
        }
        else
        {
           ++it;
        }
    }

    cookies.push_back(cookieNew);
}

void XC_HttpCookie::addCookie(const Cookie &cookie)
{
    addCookie(cookie, _cookies);
}

void XC_HttpCookie::addCookie(const list<Cookie> &cookie)
{
    list<Cookie>::const_iterator it = cookie.begin();

    while(it != cookie.end())
    {
        addCookie(*it);

        ++it;
    }
}

void XC_HttpCookie::addCookie(const string &sRspURL, const vector<string> &vCookies)
{
    XC_URL cURL;

    cURL.parseURL(sRspURL);

    string sRspURLDomain= XC_Common::lower(cURL.getDomain());

    string sRspURLPath  = cURL.getPath();

    for(size_t i = 0; i < vCookies.size(); i++)
    {
        //处理一行SetCookie
        vector<string> v = XC_Common::sepstr<string>(vCookies[i], ";");

        Cookie cookie;

        cookie._isSecure = false;
        cookie._expires  = 0;

        //解析Cookie数据
        for(size_t j = 0; j < v.size(); ++j)
        {
    		string::size_type index = v[j].find("=");

            string name;

            string value;

    		if(index != string::npos)
    		{                                                          
                name  = XC_Common::trim(v[j].substr(0, index), " ");

                value = XC_Common::trim(v[j].substr(index + 1));
    		}
            else
            {
                name  = XC_Common::trim(v[j]);
            }
            if(strcasecmp(name.c_str(), "secure") == 0)
            {
                cookie._isSecure = true;
            }
            else if(strcasecmp(name.c_str(), "expires") == 0)
            {
                struct tm stTm;
                //兼容时间格式
                //expires=Mon, 09-May-41 08:39:32 GMT
                //expires=Thu, 01-Jan-1970 01:00:00 GMT
                value = XC_Common::replace(value, "-", " ");
                if(value.length() == 27  && value.at(11) == ' ' && value.at(14) == ' ')
                {
                    int year = XC_Common::strto<int> (value.substr(12, 2));
                    if(year >= 69 && year <= 99)
                        value = value.substr(0, 12) + "19" + value.substr(12);
                    else
                        value = value.substr(0, 12) + "20" + value.substr(12);
                }

                XC_Common::strgmt2tm(value, stTm);

                cookie._expires = timegm(&stTm); 
            }
            else if(strcasecmp(name.c_str(), "path") == 0)
            {
                cookie._path = value;
            }
            else if(strcasecmp(name.c_str(), "domain") == 0)
            {
                cookie._domain = value;
            }
            else if(strcasecmp(name.c_str(), "httponly") == 0)
            {
                //TODO
                //cookie._isHttpOnly = true;
            }
            else
                cookie._data.insert(http_cookie_data::value_type(name, value));
        }

        ///修正和匹配domain/////////////////////////////////////////
        if(cookie._domain.empty())
            cookie._domain = sRspURLDomain;

        if(!fixDomain(cookie._domain, cookie._domain))
            continue;

        //匹配域名
        if(!matchDomain(cookie._domain, sRspURLDomain))
            continue;

        //修改和匹配path/////////////////////////////
        if(cookie._path.empty())
        {
            string sCookiePath;

            //缺省是全路径
            string sRequest = sRspURLPath;

            string::size_type pos = sRequest.rfind("/");

            if(pos == string::npos)
                sCookiePath = "/";
            else
                sCookiePath = sRequest.substr(0, pos+1);

            cookie._path = sCookiePath;
        }

        //URL在Path范围内,Cookie 有效
        if(!matchPath(cookie._path, sRspURLPath))
            continue;

        //添加Cookie
        addCookie(cookie);
    }
}

bool XC_HttpCookie::isCookieExpires(const Cookie &cookie) const
{
    //过期了
    if(cookie._expires !=0 && cookie._expires < time(NULL))
        return true;

    return false;
}

size_t XC_HttpCookie::isCookieMatch(const Cookie &cookie, const XC_URL &tURL) const
{
    //域名没有匹配
    if(!matchDomain(cookie._domain, tURL.getDomain()))
        return 0;

    //路径没有匹配
    size_t len = matchPath(cookie._path, tURL.getPath());
    if(len == 0)
        return 0;

    //安全的cookie,不安全的URL
    if(cookie._isSecure && (tURL.getType() != XC_URL::HTTPS))
        return 0;

    return len;
}

void XC_HttpCookie::getCookieForURL(const string &sReqURL, list<XC_HttpCookie::Cookie> &cookies)
{
    XC_URL tURL;

    tURL.parseURL(sReqURL);

    cookies.clear();

    list<Cookie>::iterator it = _cookies.begin();

    while(it != _cookies.end())
    {
        //检查Cookie是否过期
        if(isCookieExpires(*it))
        {
            _cookies.erase(it++);
            continue;
        }

        size_t len = isCookieMatch(*it, tURL);
        if(len == 0)
        {
            ++it;
            continue;
        }

        cookies.push_back(*it);

        ++it;
    }
}

void XC_HttpCookie::getCookieForURL(const string &sReqURL, string &sCookie)
{
    list<Cookie> cookies;

    sCookie.clear();

    getCookieForURL(sReqURL, cookies);

    list<Cookie>::iterator it = cookies.begin();
    while(it != cookies.end())
    {
        http_cookie_data::iterator itd = it->_data.begin();

        while(itd != it->_data.end())
        {
            //被删除的cookie不输出
            if(itd->first != "" && itd->second != "" && XC_Common::lower(itd->second) != "null" 
                    && XC_Common::lower(itd->second) != "deleted")
                sCookie += itd->first + "=" + itd->second + "; ";

            ++itd;
        }

        ++it;
    }

    //去掉末尾的 "; "
    if(sCookie.length() >= 2)
        sCookie = sCookie.substr(0, sCookie.length()-2); 
}

list<XC_HttpCookie::Cookie> XC_HttpCookie::getSerializeCookie(time_t t)
{
    list<Cookie> cookies;

    list<Cookie>::iterator it = _cookies.begin();

    while(it != _cookies.end())
    {
        if(isCookieExpires(*it))
        {
            _cookies.erase(it++);
            continue;
        }
        else if(it->_expires != 0)  //非当前会话的
        {
            cookies.push_back(*it);

            ++it;
        }
        else
            ++it;
    }

    return cookies;
}

list<XC_HttpCookie::Cookie> XC_HttpCookie::getAllCookie()
{
    deleteExpires(time(NULL));

    return _cookies;
}

void XC_HttpCookie::deleteExpires(time_t t, bool bErase)
{
    list<Cookie>::iterator it = _cookies.begin();

    while(it != _cookies.end())
    {
        if(bErase && it->_expires ==0)
        {
            _cookies.erase(it++);
            continue;
        }
        else if(isCookieExpires(*it))
        {
            _cookies.erase(it++);
            continue;
        }
        else
            ++it;
    }
}

/********************* XC_HttpResponse ***********************/

void XC_HttpResponse::parseResponseHeader(const char* szBuffer)
{
	const char **ppChar = &szBuffer;

	_headerLine = XC_Common::trim(getLine(ppChar));

    string::size_type pos = _headerLine.find(' ');

    if(pos != string::npos)
    {
        _version    = _headerLine.substr(0, pos);

        string left = XC_Common::trim(_headerLine.substr(pos));

        string::size_type pos1 = left.find(' ');

        if(pos1 != string::npos)
        {
            _status  = XC_Common::strto<int>(left.substr(0, pos));

            _about   = XC_Common::trim(left.substr(pos1 + 1));
        }
        else
        {
            _status  = XC_Common::strto<int>(left);

            _about   = "";
        }

        parseHeader(*ppChar, _headers);
        return;
    }
    else
    {   
        _version = _headerLine;
        _status  = 0;
        _about   = "";
    }

//    throw XC_HttpResponse_Exception("[XC_HttpResponse_Exception::parseResponeHeader] http response format error : " + _headerLine);
}

void XC_HttpResponse::reset()
{
    XC_Http::reset();

    _status = 200;
    _about  = "OK";
    _version = "HTTP/1.1";
    
    _iTmpContentLength = 0;
}

vector<string> XC_HttpResponse::getSetCookie() const
{
    return getHeaderMulti("Set-Cookie");
}

bool XC_HttpResponse::incrementDecode(string &sBuffer)
{
    //解析头部
    if(_headLength == 0)
    {
        string::size_type pos = sBuffer.find("\r\n\r\n");

        if(pos == string::npos)
        {
            return false;
        }

        parseResponseHeader(sBuffer.c_str());

        http_header_type::const_iterator it = _headers.find("Content-Length");
        if(it != _headers.end())
        {
            _iTmpContentLength = getContentLength();
        }
        else
        {
            //没有指明ContentLength, 接收到服务器关闭连接
            _iTmpContentLength = -1;
        }

        _headLength = pos + 4;

        sBuffer = sBuffer.substr(_headLength);

        //重定向就认为成功了
        if((_status == 301 || _status == 302) && !getHeader("Location").empty())
        {
            return true;
        }

        //是否是chunk编码
        _bIsChunked = (getHeader("Transfer-Encoding") == "chunked");

        //删除头部里面
        eraseHeader("Transfer-Encoding");
    }

    if(_bIsChunked)
    {
        while(true)
        {
            string::size_type pos   = sBuffer.find("\r\n");
            if(pos == string::npos)
                return false;

            //查找当前chunk的大小
            string sChunkSize       = sBuffer.substr(0, pos);
            int iChunkSize          = strtol(sChunkSize.c_str(), NULL, 16);

            if(iChunkSize <= 0)     break;      //所有chunk都接收完毕

            if(sBuffer.length() >= pos + 2 + (size_t)iChunkSize + 2)   //接收到一个完整的chunk了
            {
                //获取一个chunk的内容
                _content += sBuffer.substr(pos + 2, iChunkSize);

                //删除一个chunk
                sBuffer   =  sBuffer.substr(pos + 2 + iChunkSize + 2);
            }
            else                                
            {
                //没有接收完整的chunk
                return false;
            }

            setContentLength(getContent().length());
        }

        sBuffer = "";

        if(_iTmpContentLength == 0 || _iTmpContentLength == (size_t)-1)
        {
            setContentLength(getContent().length());
        }

        return true;
    }
    else
    {   
        if(_iTmpContentLength == 0)
        {
            _content += sBuffer;
            sBuffer   = "";

            //自动填写content-length
            setContentLength(getContent().length());

            return true;
        }
        else if(_iTmpContentLength == (size_t)-1)
        {
            _content += sBuffer;
            sBuffer   = "";

            //自动填写content-length
            setContentLength(getContent().length());

            return false;
        }
        else
        {
            //短连接模式, 接收到长度大于头部为止
            _content += sBuffer;
            sBuffer   = "";

            size_t iNowLength = getContent().length();

            //头部的长度小于接收的内容, 还需要继续增加解析后续的buffer
            if(_iTmpContentLength > iNowLength)
                return false;

            return true;
        }
    }

    return true;
}

bool XC_HttpResponse::decode(const string &sBuffer)
{
    string::size_type pos = sBuffer.find("\r\n\r\n");

    if(pos == string::npos)
    {
        return false;
    }

    string tmp = sBuffer;

    incrementDecode(tmp);

    //body内容长度为0或者没有content-length  且 非chunk模式, 则认为包收全了, 直接返回
    if((_iTmpContentLength == 0 || _iTmpContentLength == (size_t)-1) && !_bIsChunked)
        return true;
        
    return getContentLength() + getHeadLength() <= sBuffer.length();
}

bool XC_HttpResponse::decode(const char *sBuffer, size_t iLength)
{
    assert(sBuffer != NULL);

    const char *p = strstr(sBuffer, "\r\n\r\n");
    if( p == NULL)
    {
        return false;
    }

    string tmp(sBuffer, iLength);

    incrementDecode(tmp);

    //body内容长度为0或者没有content-length  且 非chunk模式, 则认为包收全了, 直接返回
    if((_iTmpContentLength == 0 || _iTmpContentLength == (size_t)-1) && !_bIsChunked)
        return true;

    return (getContentLength() + getHeadLength() <= iLength);
}

string XC_HttpResponse::encode() const
{
    ostringstream os;
	os << _version << " " << _status << " " << _about << "\r\n";
	os << genHeader();
	os <<  "\r\n";

    os << _content;

    return os.str();
}

void XC_HttpResponse::encode(vector<char> &buffer) const
{
    buffer.clear();

    string s = encode();

    buffer.resize(s.length());
    memcpy(&buffer[0], s.c_str(), s.length());
}

void XC_HttpResponse::setResponse(int status, const string& about, const string& body)
{
    _status = status;
    _about  = about;
    _content= body;

    ostringstream os;

	os << _version << " " << _status << " " << _about;

    _headerLine = os.str();

    setHeader("Content-Length", XC_Common::tostr(_content.length()));
}

void XC_HttpResponse::setResponse(int status, const string& about, const char *sBuffer, size_t iLength)
{
    _status = status;
    _about  = about;

    ostringstream os;

	os << _version << " " << _status << " " << _about;

    _headerLine = os.str();

    if(sBuffer != NULL && iLength > 0)
    {
        _content.assign(sBuffer, iLength);
    }
    else
    {
        _content.clear();
    }


    setHeader("Content-Length", XC_Common::tostr(_content.length()));
}

void XC_HttpResponse::setResponse(const char *sBuffer, size_t iLength)
{
    setResponse(200, "OK", sBuffer, iLength);
}

/********************* XC_HttpRequest ***********************/

void XC_HttpRequest::reset()
{
    XC_Http::reset();

    _httpURL.clear();
}

string XC_HttpRequest::requestType2str(int iRequestType) const
{
    if(iRequestType == REQUEST_GET)
    {
        return "GET";
    }
    else if(iRequestType == REQUEST_POST)
    {
        return "POST";
    }

    assert(true);
    return "";
}

vector<string> XC_HttpRequest::getCookie()
{
    vector<string> v;

    http_header_type::const_iterator itEnd = _headers.end();

    for( http_header_type::const_iterator it = _headers.begin(); it != itEnd; ++it)
    {
        if(it->first == "Cookie")
        {
            v.push_back(it->second);
        }
    }

    return v;
}

void XC_HttpRequest::parseURL(const string& sUrl)
{
    _httpURL.parseURL(sUrl);

    //设置Host
    if(getHeader("Host").empty())
    {
        string sPort = _httpURL.isDefaultPort() ? "" : ":" + _httpURL.getPort();

        //缺省端口可以不用放进去
        setHost(_httpURL.getDomain() + sPort);
    }
}

void XC_HttpRequest::encode(int iRequestType, ostream &os)
{
	os << requestType2str(iRequestType) << " " << _httpURL.getRequest() << " HTTP/1.1\r\n";
	os << genHeader();
	os << "\r\n";
}

void XC_HttpRequest::setGetRequest(const string &sUrl, bool bNewCreateHost)
{
    if(bNewCreateHost)
    {
        eraseHeader("Host");
    }

    parseURL(sUrl);

    _requestType    = REQUEST_GET;

    _content        = "";

    eraseHeader("Content-Length");
}

void XC_HttpRequest::setPostRequest(const string &sUrl, const string &sPostBody, bool bNewCreateHost)
{
    if(bNewCreateHost)
    {
        eraseHeader("Host");
    }

    parseURL(sUrl);

    _requestType    = REQUEST_POST;

    _content        = sPostBody;

    setHeader("Content-Length", XC_Common::tostr(_content.length()));
}

void XC_HttpRequest::setPostRequest(const string &sUrl, const char *sBuffer, size_t iLength, bool bNewCreateHost)
{
    assert(sBuffer != NULL);

    if(bNewCreateHost)
    {
        eraseHeader("Host");
    }

    parseURL(sUrl);

    _requestType    = REQUEST_POST;

    if(iLength > 0)
    {
        _content.assign(sBuffer, iLength);
    }
    else
    {
        _content.clear();
    }

    setHeader("Content-Length", XC_Common::tostr(_content.length()));
}

string XC_HttpRequest::encode()
{
//    assert(_requestType == REQUEST_GET || _requestType == REQUEST_POST || !_originRequest.empty());

    ostringstream os;

    if(_requestType == REQUEST_GET)
    {
        encode(REQUEST_GET, os);
    }
    else
    {
        setContentLength(_content.length());
        encode(REQUEST_POST, os);
        os << _content;
    }

    return os.str();
}

void XC_HttpRequest::encode(vector<char> &buffer)
{
//    assert(_requestType == REQUEST_GET || _requestType == REQUEST_POST || !_originRequest.empty());

    buffer.clear();

    string s = encode();
    buffer.resize(s.length());
    memcpy(&buffer[0], s.c_str(), s.length());
}

bool XC_HttpRequest::decode(const string &sBuffer)
{
    return decode(sBuffer.c_str(), sBuffer.length());
}

bool XC_HttpRequest::decode(const char *sBuffer, size_t iLength)
{
    assert(sBuffer != NULL);

    if(strncasecmp(sBuffer, "GET " ,4) !=0 && strncasecmp(sBuffer, "POST " ,5) !=0)
    {
        throw XC_HttpRequest_Exception("[XC_HttpRequest::decode] protocol not support, only support GET and POST");
    }

    if(strstr(sBuffer, "\r\n\r\n") == NULL)
    {
        return false;
    }

    _headLength = parseRequestHeader(sBuffer);

   bool bChunk = (getHeader("Transfer-Encoding") == "chunked");

    if(bChunk)
    {
        string sTmp(sBuffer + _headLength, iLength - _headLength);
        while(true)
        {
            string::size_type pos   = sTmp.find("\r\n");
            if(pos == string::npos)
                return false;

            //查找当前chunk的大小
            string sChunkSize       = sTmp.substr(0, pos);
            int iChunkSize          = strtol(sChunkSize.c_str(), NULL, 16);

            if(iChunkSize <= 0)     break;      //所有chunk都接收完毕

            if(sTmp.length() >= pos + 2 + (size_t)iChunkSize + 2)   //接收到一个完整的chunk了
            {
                //获取一个chunk的内容
                _content += sTmp.substr(pos + 2, iChunkSize);

                //删除一个chunk
                sTmp   =  sTmp.substr(pos + 2 + iChunkSize + 2);
            }
            else                                
            {
                //没有接收完整的chunk
                return false;
            }

            setContentLength(getContent().length());
        }
    }
    else
    {
        _content.assign((sBuffer + _headLength), iLength - _headLength);
//        _content    = sBuffer.substr(_headLength);
    }

    return (getContentLength() + getHeadLength() == iLength);
}


bool XC_HttpRequest::checkRequest(const char* sBuffer, size_t iLen)
{
   if(strncasecmp(sBuffer, "GET " ,4) !=0 && strncasecmp(sBuffer, "POST " ,5) !=0)
   {
	   throw runtime_error("[XC_HttpRequest::checkRequest] protocol not support, only support GET and POST");
   }

   const char *p = strstr(sBuffer, "\r\n\r\n");
   if( p == NULL)
   {
	   return false;
   }

   size_t pos = p - sBuffer;

   size_t iHeadLen= pos + 4;
   
   const char **ppChar	= &sBuffer;

   bool bChunk = false;

   //找到\r\n\r\n之前的长度表示
   while(true)
   {
	   size_t iMoveLen= (*ppChar) - sBuffer;
		if ( iMoveLen >= iHeadLen )
	   {
		   //MTT_ERRDAY << "parseHttp WARN: iMoveLen >= iLen" << "|" << iMoveLen << "|" <<iLen<<endl;
		   break;
	   }
	   
	   size_t len = 0;
	   string sLine = getLine(ppChar, iHeadLen-iMoveLen);
	   if(sLine == "")
	   {
		   len = 0;
	   }
	   else if(strncasecmp(sLine.c_str(), "Transfer-Encoding:", 18) == 0)
	   {
		   bChunk = (xutil::XC_Common::trim(sLine.substr(18)) == "chunked");
		   if(bChunk) break;
	   }
	   else if(strncasecmp(sLine.c_str(), "Content-Length:", 15) != 0)
	   {
		   continue;
	   }
	   else
	   {
		   len = xutil::XC_Common::strto<size_t>(XC_Common::trim(sLine.substr(15), " "));
	   }

	   if(len + pos + 4 <= iLen)
	   {
		   return true;
	   }
	   else
	   {
		   break;

	   }
   }

/*
	if(bChunk)
	{
		string sTmp = string(p + 4, iLen - iHeadLen);
		while(true)
	    {
	        string::size_type pos   = sTmp.find("\r\n");
	        if(pos == string::npos)
	        {
	            return false;
			}
			
	        //查找当前chunk的大小
	        string sChunkSize       = sTmp.substr(0, pos);
	        int iChunkSize          = strtol(sChunkSize.c_str(), NULL, 16);

	        if(iChunkSize <= 0)
	        {
	        	return true; //所有chunk都接收完毕
			}
	        if(sTmp.length() >= pos + 2 + (size_t)iChunkSize + 2)   //接收到一个完整的chunk了
	        {
	            //删除一个chunk
	            sTmp   =  sTmp.substr(pos + 2 + iChunkSize + 2);
	        }
	        else                           
	        {
				return false;
	        }
	    }
    }
*/
    if(bChunk)
    {
        int	remain_len = iLen - iHeadLen;
        int move_len = 0;
        const char * pCur = p + 4;
        while(true)
        {
            p = strstr(pCur , "\r\n");
            if( p == NULL )
            {
                return false;
            }

            //查找当前chunk的大小
            int iChunkSize = strtol(string(pCur, p - pCur).c_str(), NULL, 16);
             if(iChunkSize <= 0)
            {
                return true; //所有chunk都接收完毕
            }

            move_len = (p - pCur) + 2 + iChunkSize + 2;
            if( remain_len >= move_len )   //接收到一个完整的chunk了
            {
                //移动到下一个chunk
                remain_len -= move_len;
                pCur = p + 2 + iChunkSize + 2;
            }
            else                           
            {
                return false;
            }
        }
    }

    return false;
//	 throw XC_HttpRequest_Exception("[XC_HttpRequest::checkRequest] Content-Length not exists.");

//	 return true;
}


size_t XC_HttpRequest::parseRequestHeader(const char* szBuffer)
{
    const char *szBuffer_copy = szBuffer;
    const char **ppChar = &szBuffer;

    /* parse the first line and get status */
    string sLine = getLine(ppChar);

    string::size_type pos = sLine.find(" ");
    if(pos == string::npos)
    {
        throw XC_HttpRequest_Exception("[XC_HttpRequest::parseRequestHeader] http request format error: " + sLine);
    }

    string sMethod = XC_Common::trim(sLine.substr(0, pos));

    //解析请求类型
    if(sMethod == "GET")
    {
        _requestType = REQUEST_GET;
    }
    else if(sMethod == "POST")
    {
        _requestType = REQUEST_POST;
    }
    else
    {
        throw XC_HttpRequest_Exception("[XC_HttpRequest::parseRequestHeader] http request command error: " + sMethod);
    }

    string::size_type pos1 = sLine.rfind(" ");
    if(pos1 == string::npos || pos1 <= pos)
    {
        throw XC_HttpRequest_Exception("[XC_HttpRequest::parseRequestHeader] http request format error: " + sLine);
    }

    //URL地址
    string sURL = XC_Common::trim(sLine.substr(pos+1, pos1 - pos));
    
    //HTTP协议版本
    string sVersion = XC_Common::upper(XC_Common::trim(sLine.substr(pos1+1)));

    if(sVersion != "HTTP/1.1" || sVersion != "HTTP/1.0")
    {
        sVersion == "HTTP/1.1";
    }

    size_t n = parseHeader(*ppChar, _headers) - szBuffer_copy;

    if(strncasecmp(sURL.c_str(), "https://", 8) !=0 )
    {
        if(strncasecmp(sURL.c_str(), "http://", 7) !=0 )
        {
            sURL = "http://" + getHost() + sURL;
        }
    }

    parseURL(sURL);

    return n;
}

void XC_HttpRequest::getHostPort(string &sDomain, uint32_t &iPort)
{
    sDomain = _httpURL.getDomain();

    iPort   = XC_Common::strto<uint32_t>(_httpURL.getPort());
}

int XC_HttpRequest::doRequest(XC_HttpResponse &stHttpRsp, int iTimeout)
{
    //只支持短连接模式
    setConnection("close");

    string sSendBuffer = encode();

    string sHost;
    uint32_t iPort;

    getHostPort(sHost, iPort);

    XC_TCPClient tcpClient;
    tcpClient.init(sHost, iPort, iTimeout);

    int iRet = tcpClient.send(sSendBuffer.c_str(), sSendBuffer.length());
    if(iRet != XC_ClientSocket::EM_SUCCESS)
    {
        return iRet;
    }

    stHttpRsp.reset();

    string sBuffer;

    char *sTmpBuffer = new char[10240];
    size_t iRecvLen  = 10240;

    while(true)
    {
        iRecvLen = 10240;

        iRet = tcpClient.recv(sTmpBuffer, iRecvLen);

        if(iRet == XC_ClientSocket::EM_SUCCESS)
            sBuffer.append(sTmpBuffer, iRecvLen);

        switch(iRet)
        {
        case XC_ClientSocket::EM_SUCCESS:
            if(stHttpRsp.incrementDecode(sBuffer)) 
            {
                delete []sTmpBuffer;
                return XC_ClientSocket::EM_SUCCESS;
            }
            continue;
        case XC_ClientSocket::EM_CLOSE:
            delete []sTmpBuffer;
            stHttpRsp.incrementDecode(sBuffer);
            return XC_ClientSocket::EM_SUCCESS;
        default:
            delete []sTmpBuffer;
            return iRet;
        }
    }

    assert(true);

    return 0;
}

}


