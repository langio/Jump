#include "util/tc_file.h"
#include <string.h>

namespace xutil
{

ifstream::pos_type XC_File::getFileSize(const string &sFullFileName)
{
    ifstream ifs(sFullFileName.c_str());
    ifs.seekg(0, ios_base::end);
    return ifs.tellg();
}

bool XC_File::isAbsolute(const string &sFullFileName)
{
    if(sFullFileName.empty())
    {
        return false;
    }

    unsigned i = 0;
    while(isspace(sFullFileName[i]))
    {
        ++i;
    }

    return sFullFileName[i] == '/';
}

bool XC_File::isFileExist(const string &sFullFileName, mode_t iFileType)
{
    struct stat f_stat;

    if (lstat(sFullFileName.c_str(), &f_stat) == -1)
    {
        return false;
    }

    if (!(f_stat.st_mode & iFileType))
    {
        return false;
    }

    return true;
}

bool XC_File::isFileExistEx(const string &sFullFileName, mode_t iFileType)
{
    struct stat f_stat;

    if (stat(sFullFileName.c_str(), &f_stat) == -1)
    {
        return false;
    }

    if (!(f_stat.st_mode & iFileType))
    {
        return false;
    }

    return true;
}

bool XC_File::makeDir(const string &sDirectoryPath, mode_t iFlag)
{
    int iRetCode = mkdir(sDirectoryPath.c_str(), iFlag);
    if(iRetCode < 0 && errno == EEXIST)
    {
        return isFileExistEx(sDirectoryPath, S_IFDIR);
    }

    return iRetCode == 0;
}

bool XC_File::makeDirRecursive(const string &sDirectoryPath, mode_t iFlag)
{
    string simple = simplifyDirectory(sDirectoryPath);

    string::size_type pos = 0;
    for(; pos != string::npos; )
    {
        pos = simple.find("/", pos + 1);
        string s;
        if(pos == string::npos)
        {
            s = simple.substr(0, simple.size());
            return makeDir(s.c_str(), iFlag);
        }
        else
        {
            s = simple.substr(0, pos);
            if(!makeDir(s.c_str(), iFlag)) return false;
        }
    }
    return true;
}

int XC_File::setExecutable(const string &sFullFileName, bool canExecutable)
{
    struct stat f_stat;

    if (stat(sFullFileName.c_str(), &f_stat) == -1)
    {
        return -1;
    }

    return chmod(sFullFileName.c_str(), canExecutable ? f_stat.st_mode | S_IXUSR : f_stat.st_mode & ~S_IXUSR);
}

bool XC_File::canExecutable(const string &sFullFileName)
{
    struct stat f_stat;

    if (stat(sFullFileName.c_str(), &f_stat) == -1)
    {
        return false;
    }

    return f_stat.st_mode & S_IXUSR;
}

int XC_File::removeFile(const string &sFullFileName, bool bRecursive)
{
    string path = simplifyDirectory(sFullFileName);

    if(isFileExist(path, S_IFDIR))
    {
        if(bRecursive)
        {
            vector<string> files;
            listDirectory(path, files, false);
            for(size_t i = 0; i < files.size(); i++)
            {
                removeFile(files[i], bRecursive);
            }

            if(path != "/")
            {
                if(::rmdir(path.c_str()) == -1)
                {
                    return -1;
                }
                return 0;
            }
        }
        else
        {
            if(::rmdir(path.c_str()) == -1)
            {
                return -1;
            }
        }
    }
    else
    {
        if(::remove(path.c_str()) == -1)
        {
            return -1;
        }
    }

    return 0;
}

string XC_File::simplifyDirectory(const string& path)
{
    string result = path;

    string::size_type pos;

    pos = 0;
    while((pos = result.find("//", pos)) != string::npos)
    {
        result.erase(pos, 1);
    }

    pos = 0;
    while((pos = result.find("/./", pos)) != string::npos)
    {
        result.erase(pos, 2);
    }

    while(result.substr(0, 4) == "/../")
    {
        result.erase(0, 3);
    }

    if(result == "/.")
    {
       return result.substr(0, result.size() - 1);
    }

    if(result.size() >= 2 && result.substr(result.size() - 2, 2) == "/.")
    {
        result.erase(result.size() - 2, 2);
    }

    if(result == "/")
    {
        return result;
    }

    if(result.size() >= 1 && result[result.size() - 1] == '/')
    {
        result.erase(result.size() - 1);
    }

    if(result == "/..")
    {
        result = "/";
    }

    return result;
}

string XC_File::load2str(const string &sFullFileName)
{
    ifstream ifs(sFullFileName.c_str());

    return string(istreambuf_iterator<char>(ifs), istreambuf_iterator<char>());
}

void XC_File::save2file(const string &sFullFileName, const string &sFileData)
{
    ofstream ofs((sFullFileName).c_str());
    ofs << sFileData;
    ofs.close();
}

int XC_File::save2file(const string &sFullFileName, const char *sFileData, size_t length)
{
	FILE *fp = fopen(sFullFileName.c_str(), "wb");
	if(fp == NULL)
	{
		return -1;
	}

	size_t ret = fwrite((void*)sFileData, 1, length, fp);
	fclose(fp);

	if(ret == length)
	{
		return 0;
	}
	return -1;
}

string XC_File::getExePath()
{
    string proc = "/proc/self/exe";
    char buf[2048] = "\0";
    int count = readlink(proc.c_str(), buf, sizeof(buf)/sizeof(char));

    if ( count < 0 )
    {
        throw XC_File_Exception("[XC_File::getExePath] could not get exe path error", errno);
    }

    buf[count] = '\0';
    return buf;
}

string XC_File::extractFileName(const string &sFullFileName)
{
    if(sFullFileName.length() <= 0)
    {
        return "";
    }

    string::size_type pos = sFullFileName.rfind('/');
    if(pos == string::npos)
    {
        return sFullFileName;
    }

    return sFullFileName.substr(pos + 1);
}

string XC_File::extractFilePath(const string &sFullFileName)
{
    if(sFullFileName.length() <= 0)
    {
        return "./";
    }

    string::size_type pos = 0;

    for(pos = sFullFileName.length(); pos != 0 ; --pos)
    {
        if(sFullFileName[pos-1] == '/')
        {
            return sFullFileName.substr(0, pos);
        }
    }

    return "./";
}

string XC_File::extractFileExt(const string &sFullFileName)
{
    string::size_type pos;
    if ((pos = sFullFileName.rfind('.')) == string::npos)
    {
        return string("");
    }

    return sFullFileName.substr(pos+1);
}

string XC_File::excludeFileExt(const string &sFullFileName)
{
    string::size_type pos;
    if ((pos = sFullFileName.rfind('.')) == string::npos)
    {
        return sFullFileName;
    }

    return sFullFileName.substr(0, pos);
}

string XC_File::replaceFileExt(const string &sFullFileName, const string &sExt)
{
    return excludeFileExt(sFullFileName) + "." + sExt;
}

string XC_File::extractUrlFilePath(const string &sUrl)
{
    string sLowerUrl = XC_Common::lower(sUrl);
    string::size_type pos = sLowerUrl.find("http://");

    if(pos == 0)
    {
        pos += strlen("http://");
    }
    else if(pos == string::npos)
    {
        pos = 0;
    }

    for( ; pos < sUrl.length(); ++pos)
    {
        if(sUrl[pos] == '/')
        {
            if(pos < sUrl.length() - 1)
            {
                pos++;
                break;
            }
            else
            {
                return "";
            }
        }
    }

    if(pos == string::npos || pos == sUrl.length())
    {
        pos = 0;
    }

    return sUrl.substr(pos);
}

size_t XC_File::scanDir(const string &sFilePath, vector<string> &vtMatchFiles, FILE_SELECT f, int iMaxSize )
{
    vtMatchFiles.clear();

    struct dirent **namelist;
    int n = scandir(sFilePath.c_str(), &namelist, f, alphasort);

    if (n < 0)
    {
        return 0;
    }
    else
    {
        while(n-- )
        {
            if(iMaxSize > 0 && vtMatchFiles.size() >= (size_t)iMaxSize )
            {
                free(namelist[n]);
                break;
            }
            else
            {
                vtMatchFiles.push_back(namelist[n]->d_name);
                free(namelist[n]);
            }
        }
        free(namelist);
    }

    return vtMatchFiles.size();
}

void XC_File::listDirectory(const string &path, vector<string> &files, bool bRecursive)
{
    vector<string> tf;
    scanDir(path, tf, 0, 0);

    for(size_t i = 0; i < tf.size(); i++)
    {
        if(tf[i] == "." || tf[i] == "..")
            continue;

        string s = path + "/" + tf[i];

        if(isFileExist(s, S_IFDIR))
        {
            files.push_back(simplifyDirectory(s));
            if(bRecursive)
            {
                listDirectory(s, files, bRecursive);
            }
        }
        else
        {
            files.push_back(simplifyDirectory(s));
        }
    }
}

void XC_File::copyFile(const string &sExistFile, const string &sNewFile,bool bRemove)
{
    if(XC_File::isFileExist(sExistFile, S_IFDIR))
    {    
        XC_File::makeDir(sNewFile);  
        vector<string> tf;
        XC_File::scanDir(sExistFile,tf, 0, 0);
        for(size_t i = 0; i <tf.size(); i++)
        {
            if(tf[i] == "." || tf[i] == "..")
            continue;
            string s = sExistFile + "/" + tf[i];
            string d = sNewFile + "/" + tf[i];  
            copyFile(s, d,bRemove);
        }
    }
    else
    {   
        if(bRemove) std::remove(sNewFile.c_str());
        std::ifstream fin(sExistFile.c_str());
        if(!fin)
        {      
            throw XC_File_Exception("[XC_File::copyFile] error", errno);
        }
        std::ofstream fout(sNewFile.c_str());
        if(!fout )
        {  
             throw XC_File_Exception("[XC_File::copyFile] error", errno); 
        }
        struct stat f_stat;
        if (stat(sExistFile.c_str(), &f_stat) == -1)
        {
             throw XC_File_Exception("[XC_File::copyFile] error", errno);
        }
        chmod(sNewFile.c_str(),f_stat.st_mode);
        fout<<fin.rdbuf();
        fin.close();
        fout.close();
        
    }  
}

}

