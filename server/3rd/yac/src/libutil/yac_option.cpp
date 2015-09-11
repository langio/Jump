#include "util/yac_option.h"
#include "util/yac_common.h"

namespace util
{

void YAC_Option::decode(int argc, char *argv[])
{
    _mParam.clear();

    vector<string> v;
    for(int i = 1; i < argc; i++)
    {
        v.push_back(argv[i]);
    }

    for(size_t i = 0; i < v.size(); i++)
    {
        if(v[i].length() > 2 && v[i].substr(0,2) == "--")
        {
            parse(v[i]);
        }
        else
        {
            _vSingle.push_back(v[i]);
        }
    }
}

void YAC_Option::parse(const string &s)
{
    string::size_type pos = s.find('=');
    if( pos != string::npos)
    {
        _mParam[s.substr(2, pos-2)] = s.substr(pos+1);
    }
    else
    {
        _mParam[s.substr(2, pos-2)] = "";
    }
}

string YAC_Option::getValue(const string &sName)
{
    if(_mParam.find(sName) != _mParam.end())
    {
        return _mParam[sName];
    }
    return "";
}

bool YAC_Option::hasParam(const string &sName)
{
    return _mParam.find(sName) != _mParam.end();
}

vector<string>& YAC_Option::getSingle()
{
    return _vSingle;
}

map<string, string>& YAC_Option::getMulti()
{
    return _mParam;
}

}


