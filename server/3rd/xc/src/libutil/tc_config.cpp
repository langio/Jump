#include <errno.h>
#include <fstream>
#include "util/tc_config.h"
#include "util/tc_common.h"

namespace xutil
{

XC_ConfigDomain::XC_ConfigDomain(const string &sLine)
{
	_name = XC_Common::trim(sLine);
}

XC_ConfigDomain::~XC_ConfigDomain()
{
	destroy();
}

XC_ConfigDomain::XC_ConfigDomain(const XC_ConfigDomain &tcd)
{
    (*this) = tcd;
}

XC_ConfigDomain& XC_ConfigDomain::operator=(const XC_ConfigDomain &tcd)
{
    if(this != &tcd)
    {
        destroy();

        _name  = tcd._name;
        _param = tcd._param;
        _key   = tcd._key;
        _domain= tcd._domain;

        const map<string, XC_ConfigDomain*> & m = tcd.getDomainMap();
        map<string, XC_ConfigDomain*>::const_iterator it = m.begin();
        while(it != m.end())
        {
            _subdomain[it->first] = it->second->clone();
            ++it;
        }
    }
    return *this;
}

XC_ConfigDomain::DomainPath XC_ConfigDomain::parseDomainName(const string& path, bool bWithParam)
{
    XC_ConfigDomain::DomainPath dp;

    if(bWithParam)
    {
    	string::size_type pos1 = path.find_first_of(XC_CONFIG_PARAM_BEGIN);
    	if(pos1 == string::npos)
    	{
    		throw XC_Config_Exception("[XC_Config::parseDomainName] : param path '" + path + "' is invalid!" );
    	}

    	if(path[0] != XC_CONFIG_DOMAIN_SEP)
    	{
    		throw XC_Config_Exception("[XC_Config::parseDomainName] : param path '" + path + "' must start with '/'!" );
    	}

    	string::size_type pos2 = path.find_first_of(XC_CONFIG_PARAM_END);
    	if(pos2 == string::npos)
    	{
    		throw XC_Config_Exception("[XC_Config::parseDomainName] : param path '" + path + "' is invalid!" );
    	}

        dp._domains = XC_Common::sepstr<string>(path.substr(1, pos1-1), XC_Common::tostr(XC_CONFIG_DOMAIN_SEP));
        dp._param = path.substr(pos1+1, pos2 - pos1 - 1);
    }
    else
    {
//    	if(path.length() <= 1 || path[0] != XC_CONFIG_DOMAIN_SEP)
        if(path[0] != XC_CONFIG_DOMAIN_SEP)
    	{
    		throw XC_Config_Exception("[XC_Config::parseDomainName] : param path '" + path + "' must start with '/'!" );
    	}

        dp._domains = XC_Common::sepstr<string>(path.substr(1), XC_Common::tostr(XC_CONFIG_DOMAIN_SEP));
    }

    return dp;
}

XC_ConfigDomain* XC_ConfigDomain::addSubDomain(const string& name)
{
    if(_subdomain.find(name) == _subdomain.end())
    {
        _domain.push_back(name);

        _subdomain[name] = new XC_ConfigDomain(name);
    }
    return _subdomain[name];
}

string XC_ConfigDomain::getParamValue(const string &name) const
{
    map<string, string>::const_iterator it = _param.find(name);
	if( it == _param.end())
	{
		throw XC_ConfigNoParam_Exception("[XC_ConfigDomain::getParamValue] param '" + name + "' not exits!");
    }

	return it->second;
}

XC_ConfigDomain *XC_ConfigDomain::getSubTcConfigDomain(vector<string>::const_iterator itBegin, vector<string>::const_iterator itEnd)
{
    if(itBegin == itEnd)
    {
        return this;
    }

    map<string, XC_ConfigDomain*>::const_iterator it = _subdomain.find(*itBegin);

	//根据匹配规则找不到匹配的子域
	if(it == _subdomain.end())
	{
		return NULL;
	}

	//继续在子域下搜索
	return it->second->getSubTcConfigDomain(itBegin + 1, itEnd);
}

const XC_ConfigDomain *XC_ConfigDomain::getSubTcConfigDomain(vector<string>::const_iterator itBegin, vector<string>::const_iterator itEnd) const
{
    if(itBegin == itEnd)
    {
        return this;
    }

    map<string, XC_ConfigDomain*>::const_iterator it = _subdomain.find(*itBegin);

	//根据匹配规则找不到匹配的子域
	if(it == _subdomain.end())
	{
		return NULL;
	}

	//继续在子域下搜索
	return it->second->getSubTcConfigDomain(itBegin + 1, itEnd);
}

void XC_ConfigDomain::insertParamValue(const map<string, string> &m)
{
    _param.insert(m.begin(),  m.end());

    map<string, string>::const_iterator it = m.begin();
    while(it != m.end())
    {
        size_t i = 0;
        for(; i < _key.size(); i++)
        {
            if(_key[i] == it->first)
            {
                break;
            }
        }

        //没有该key, 则添加到最后
        if(i == _key.size())
        {
            _key.push_back(it->first);
        }

        ++it;
    }
}

void XC_ConfigDomain::setParamValue(const string &name, const string &value)
{
    _param[name] = value;

    //如果key已经存在,则删除
    for(vector<string>::iterator it = _key.begin(); it != _key.end(); ++it)
    {
        if(*it == name)
        {
            _key.erase(it);
            break;
        }
    }

    _key.push_back(name);
}

void XC_ConfigDomain::setParamValue(const string &line)
{
    if(line.empty())
    {
        return;
    }

    string::size_type pos = 0;
    for(; pos <= line.length() - 1; pos++)
    {
        if (line[pos] == '=')
        {
            if(pos > 0 && line[pos-1] == '\\')
            {
                continue;
            }

            string name  = parse(XC_Common::trim(line.substr(0, pos), " \r\n\t"));

            string value;
            if(pos < line.length() - 1)
            {
                value = parse(XC_Common::trim(line.substr(pos + 1), " \r\n\t"));
            }

            setParamValue(name, value);
            return;
        }
    }

    setParamValue(line, "");
}

string XC_ConfigDomain::parse(const string& s)
{
    if(s.empty())
    {
        return "";
    }

    string param;
    string::size_type pos = 0;
    for(; pos <= s.length() - 1; pos++)
    {
        char c;
        if(s[pos] == '\\' && pos < s.length() - 1)
        {
            switch (s[pos+1])
            {
            case '\\':
                c = '\\';
                pos++;
                break;
            case 'r':
                c = '\r';
                pos++;
                break;
            case 'n':
                c = '\n';
                pos++;
                break;
            case 't':
                c = '\t';
                pos++;
                break;
            case '=':
                c = '=';
                pos++;
                break;
            default:
                throw XC_Config_Exception("[XC_ConfigDomain::parse] '" + s + "' is invalid, '" + XC_Common::tostr(s[pos]) + XC_Common::tostr(s[pos+1]) + "' couldn't be parse!" );
            }

            param += c;
        }
        else if (s[pos] == '\\')
        {
            throw XC_Config_Exception("[XC_ConfigDomain::parse] '" + s + "' is invalid, '" + XC_Common::tostr(s[pos]) + "' couldn't be parse!" );
        }
        else
        {
            param += s[pos];
        }
    }

    return param;
}

string XC_ConfigDomain::reverse_parse(const string &s)
{
    if(s.empty())
    {
        return "";
    }

    string param;
    string::size_type pos = 0;
    for(; pos <= s.length() - 1; pos++)
    {
        string c;
        switch (s[pos])
        {
        case '\\':
            param += "\\\\";
            break;
        case '\r':
            param += "\\r";
            break;
        case '\n':
            param += "\\n";
            break;
        case '\t':
            param += "\\t";
            break;
            break;
        case '=':
            param += "\\=";
            break;
        case '<':
        case '>':
            throw XC_Config_Exception("[XC_ConfigDomain::reverse_parse] '" + s + "' is invalid, couldn't be parse!" );
        default:
            param += s[pos];
        }
    }

    return param;
}

string XC_ConfigDomain::getName() const
{
	return _name;
}

void XC_ConfigDomain::setName(const string& name)
{
    _name = name;
}

vector<string> XC_ConfigDomain::getKey() const
{
    return _key;
}

vector<string> XC_ConfigDomain::getSubDomain() const
{
    return _domain;
}

void XC_ConfigDomain::destroy()
{
    _param.clear();
    _key.clear();
    _domain.clear();

    map<string, XC_ConfigDomain*>::iterator it = _subdomain.begin();
    while(it != _subdomain.end())
    {
        delete it->second;
        ++it;
    }

    _subdomain.clear();
}

string XC_ConfigDomain::tostr(int i) const
{
    string sTab;
    for(int k = 0; k < i; ++k)
    {
        sTab += "\t";
    }

    ostringstream buf;

    buf << sTab << "<" << reverse_parse(_name) << ">" << endl;;

    for(size_t n = 0; n < _key.size(); n++)
    {
        map<string, string>::const_iterator it = _param.find(_key[n]);

        assert(it != _param.end());

        //值为空, 则不打印出=
        if(it->second.empty())
        {
            buf << "\t" << sTab << reverse_parse(_key[n]) << endl;
        }
        else
        {
            buf << "\t" << sTab << reverse_parse(_key[n]) << "=" << reverse_parse(it->second) << endl;
        }
    }

    ++i;

    for(size_t n = 0; n < _domain.size(); n++)
    {
        map<string, XC_ConfigDomain*>::const_iterator itm = _subdomain.find(_domain[n]);

        assert(itm != _subdomain.end());

        buf << itm->second->tostr(i);
    }


	buf << sTab << "</" << reverse_parse(_name) << ">" << endl;

	return buf.str();
}

/********************************************************************/
/*		XC_Config implement										    */
/********************************************************************/

XC_Config::XC_Config() : _root("")
{
}

XC_Config::XC_Config(const XC_Config &tc)
: _root(tc._root)
{

}

XC_Config& XC_Config::operator=(const XC_Config &tc)
{
    if(this != &tc)
    {
        _root = tc._root;
    }

    return *this;
}

void XC_Config::parse(istream &is)
{
    _root.destroy();

    stack<XC_ConfigDomain*> stkTcCnfDomain;
    stkTcCnfDomain.push(&_root);

    string line;
    while(getline(is, line))
	{
		line = XC_Common::trim(line, " \r\n\t");

		if(line.length() == 0)
		{
			continue;
		}

		if(line[0] == '#')
		{
			continue;
		}
		else if(line[0] == '<')
		{
			string::size_type posl = line.find_first_of('>');

			if(posl == string::npos)
			{
				throw XC_Config_Exception("[XC_Config::parse]:parse error! line : " + line);
			}

			if(line[1] == '/')
			{
				string sName(line.substr(2, (posl - 2)));

				if(stkTcCnfDomain.size() <= 0)
                {
					throw XC_Config_Exception("[XC_Config::parse]:parse error! <" + sName + "> hasn't matched domain.");
                }

                if(stkTcCnfDomain.top()->getName() != sName)
				{
					throw XC_Config_Exception("[XC_Config::parse]:parse error! <" + stkTcCnfDomain.top()->getName() + "> hasn't match <" + sName +">.");
				}

                //弹出
				stkTcCnfDomain.pop();
			}
			else
			{
				string name(line.substr(1, posl - 1));

                stkTcCnfDomain.push(stkTcCnfDomain.top()->addSubDomain(name));
			}
		}
		else
		{
            stkTcCnfDomain.top()->setParamValue(line);
		}
	}

	if(stkTcCnfDomain.size() != 1)
	{
		throw XC_Config_Exception("[XC_Config::parse]:parse error : hasn't match");
    }
}

void XC_Config::parseFile(const string &sFileName)
{
    if(sFileName.length() == 0)
    {
		throw XC_Config_Exception("[XC_Config::parseFile]:file name is empty");
    }

    ifstream ff;
    ff.open(sFileName.c_str());
	if (!ff)
	{
		throw XC_Config_Exception("[XC_Config::parseFile]:fopen fail: " + sFileName, errno);
    }

    parse(ff);
}

void XC_Config::parseString(const string& buffer)
{
    istringstream iss;
    iss.str(buffer);

    parse(iss);
}

string XC_Config::operator[](const string &path)
{
    XC_ConfigDomain::DomainPath dp = XC_ConfigDomain::parseDomainName(path, true);

	XC_ConfigDomain *pTcConfigDomain = searchTcConfigDomain(dp._domains);

	if(pTcConfigDomain == NULL)
	{
		throw XC_ConfigNoParam_Exception("[XC_Config::operator[]] path '" + path + "' not exits!");
	}

	return pTcConfigDomain->getParamValue(dp._param);
}

string XC_Config::get(const string &sName, const string &sDefault) const
{
    try
    {
        XC_ConfigDomain::DomainPath dp = XC_ConfigDomain::parseDomainName(sName, true);

    	const XC_ConfigDomain *pTcConfigDomain = searchTcConfigDomain(dp._domains);

    	if(pTcConfigDomain == NULL)
    	{
    		throw XC_ConfigNoParam_Exception("[XC_Config::get] path '" + sName + "' not exits!");
    	}

    	return pTcConfigDomain->getParamValue(dp._param);
    }
    catch ( XC_ConfigNoParam_Exception &ex )
    {
        return sDefault;
    }
    return sDefault;
}

bool XC_Config::getDomainMap(const string &path, map<string, string> &m) const
{
    XC_ConfigDomain::DomainPath dp = XC_ConfigDomain::parseDomainName(path, false);

	const XC_ConfigDomain *pTcConfigDomain = searchTcConfigDomain(dp._domains);

	if(pTcConfigDomain == NULL)
	{
		return false;
	}

	m = pTcConfigDomain->getParamMap();

    return true;
}

map<string, string> XC_Config::getDomainMap(const string &path) const
{
    map<string, string> m;

    XC_ConfigDomain::DomainPath dp = XC_ConfigDomain::parseDomainName(path, false);

	const XC_ConfigDomain *pTcConfigDomain = searchTcConfigDomain(dp._domains);

	if(pTcConfigDomain != NULL)
	{
        m = pTcConfigDomain->getParamMap();
    }

    return m;
}

vector<string> XC_Config::getDomainKey(const string &path) const
{
    vector<string> v;

    XC_ConfigDomain::DomainPath dp = XC_ConfigDomain::parseDomainName(path, false);

	const XC_ConfigDomain *pTcConfigDomain = searchTcConfigDomain(dp._domains);

	if(pTcConfigDomain != NULL)
	{
        v = pTcConfigDomain->getKey();
    }

    return v;
}

bool XC_Config::getDomainVector(const string &path, vector<string> &vtDomains) const
{
    XC_ConfigDomain::DomainPath dp = XC_ConfigDomain::parseDomainName(path, false);

    //根域, 特殊处理
    if(dp._domains.empty())
    {
        vtDomains = _root.getSubDomain();
        return !vtDomains.empty();
    }

	const XC_ConfigDomain *pTcConfigDomain = searchTcConfigDomain(dp._domains);

	if(pTcConfigDomain == NULL)
	{
        return false;
	}

    vtDomains = pTcConfigDomain->getSubDomain();

	return true;
}

vector<string> XC_Config::getDomainVector(const string &path) const
{
    XC_ConfigDomain::DomainPath dp = XC_ConfigDomain::parseDomainName(path, false);

    //根域, 特殊处理
    if(dp._domains.empty())
    {
        return _root.getSubDomain();
    }

	const XC_ConfigDomain *pTcConfigDomain = searchTcConfigDomain(dp._domains);

	if(pTcConfigDomain == NULL)
	{
        return vector<string>();
	}

    return pTcConfigDomain->getSubDomain();
}


XC_ConfigDomain *XC_Config::newTcConfigDomain(const string &sName)
{
	return new XC_ConfigDomain(sName);
}

XC_ConfigDomain *XC_Config::searchTcConfigDomain(const vector<string>& domains)
{
	return _root.getSubTcConfigDomain(domains.begin(), domains.end());
}

const XC_ConfigDomain *XC_Config::searchTcConfigDomain(const vector<string>& domains) const
{
	return _root.getSubTcConfigDomain(domains.begin(), domains.end());
}

int XC_Config::insertDomain(const string &sCurDomain, const string &sAddDomain, bool bCreate)
{
    XC_ConfigDomain::DomainPath dp = XC_ConfigDomain::parseDomainName(sCurDomain, false);

	XC_ConfigDomain *pTcConfigDomain = searchTcConfigDomain(dp._domains);

	if(pTcConfigDomain == NULL)
	{
        if(bCreate)
        {
            pTcConfigDomain = &_root;

            for(size_t i = 0; i < dp._domains.size(); i++)
            {
                pTcConfigDomain = pTcConfigDomain->addSubDomain(dp._domains[i]);
            }
        }
        else
        {
            return -1;
        }
	}

    pTcConfigDomain->addSubDomain(sAddDomain);

    return 0;
}

int XC_Config::insertDomainParam(const string &sCurDomain, const map<string, string> &m, bool bCreate)
{
    XC_ConfigDomain::DomainPath dp = XC_ConfigDomain::parseDomainName(sCurDomain, false);

	XC_ConfigDomain *pTcConfigDomain = searchTcConfigDomain(dp._domains);

	if(pTcConfigDomain == NULL)
	{
        if(bCreate)
        {
            pTcConfigDomain = &_root;

            for(size_t i = 0; i < dp._domains.size(); i++)
            {
                pTcConfigDomain = pTcConfigDomain->addSubDomain(dp._domains[i]);
            }
        }
        else
        {
            return -1;
        }
	}

    pTcConfigDomain->insertParamValue(m);

    return 0;
}

string XC_Config::tostr() const
{
    string buffer;

    map<string, XC_ConfigDomain*> msd = _root.getDomainMap();
    map<string, XC_ConfigDomain*>::const_iterator it = msd.begin();
    while (it != msd.end())
    {
        buffer += it->second->tostr(0);
        ++it;
    }

    return buffer;
}

void XC_Config::joinConfig(const XC_Config &cf, bool bUpdate)
{
    string buffer;
    if(bUpdate)
    {
        buffer = tostr() + cf.tostr();
    }
    else
    {
        buffer = cf.tostr() + tostr();
    }
    parseString(buffer);
}

}

