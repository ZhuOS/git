//ResourceHost.cpp
#include "ResourceHost.h"

ResourceHost::ResourceHost(const string &basePath )
{
	baseDiskPath = basePath;
}
string ResourceHost::getMimeType(const string &ext)
{
	unordered_map<string,string>::const_iterator it = mimeMap.find(ext);
	if(it == mimeMap.end())
		return "test/html";	
	return it->second;
}

Resource* ResourceHost::getResource(std::string uri)
{
	string path = baseDiskPath + uri;//加入判断
	
	struct stat statbuf;
	stat(path.c_str(), &statbuf);
	if( S_ISDIR(statbuf.st_mode) ){		//directory
		perror("path is directory");
		return NULL;//
	}
	else if( S_ISREG(statbuf.st_mode) )	//file
		return readFile( path );
	else{
		perror("unknown path");
		return NULL;
	}
		
}

Resource* ResourceHost::readFile(const string &path)
{
	ifstream infile;
	infile.open(path);
	if( !infile ){
			perror("unable to open file");
			return NULL;
	}
	string buff;
	infile>>buff;
	char* dat = new char[buff.size()];
	Resource *res = new Resource(path, false);
	res->setData( (byte*)dat, buff.size() );
	//
	string ext="";
	string mity="";
	size_t bpos= path.find_last_of(".");
	if( !bpos == string::npos){
		ext = path.substr(bpos+1);
		mity = getMimeType(ext);
	}else{
		perror("MimeType Not Found");
		return NULL;
	}
	res->setMimeType(mity);
	return res;
	
}










