//ResourceHost.cpp
#include "ResourceHost.h"


ResourceHost::ResourceHost(const string &basePath )
{
	baseDiskPath = basePath;
}
string ResourceHost::getMimeType(const string &ext)
{
	unordered_map<string,string>::const_iterator it = mimeMap.find(ext);
	if(it == mimeMap.end()){
		cout<<"use default mimetype";	
		return "test/html";	
	}
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
	else if( S_ISREG(statbuf.st_mode) ){	//file
		//cout<<"resourceHost uri is file"<<endl;	//test	
		return readFile( path );
	}
	else{
		perror("unknown path");
		return NULL;
	}
		
}
//string buff;		//不知为啥，在运行中，buff内存会变来变去，不好玩######指针 坑坑啊 
Resource* ResourceHost::readFile(const string &path)
{	
	cout<<"ResourceHost readFile begin path: "<<path<<endl;//test
	/*第一种
	ifstream infile;
	infile.open(path.c_str(), std::ios::binary);
	if( !infile ){
			perror("	unable to open file");
			return NULL;
	}

	infile>>buff;
	cout<<"	###file data :"<<buff<<endl;					//test 读入数据需要全部读入 修改
	
	PointCharData = (char*)buff.data();				//漏了这一步啊 啊啊啊啊啊
	//PointCharData[buff.size()] = '\0';					//test
	//cout<<"	###dat       :"<<PointCharData<<endl;				//test
	*/
	//*第二种		 一次全部输出				
	std::ifstream infile;  
	int length;  
	infile.open(path.c_str(), std::ios::binary);      // open input file 
	if( !infile ){
			perror("	unable to open file");
			return NULL;
	} 
	infile.seekg(0, std::ios::end);    // go to the end  
	length = infile.tellg();           // report location (this is the length)  
	cout<<length<<endl;
	infile.seekg(0, std::ios::beg);    // go back to the beginning  
	PointCharData = new char[length];    // allocate memory for a buffer of appropriate dimension  
	infile.read(PointCharData, length);       // read the whole file into the buffer  
	infile.close();                    // close file handle  	
    PointCharData[length] = '\0';
    cout<<"     ---PointCharData: "<<PointCharData<<endl;
	
	Resource *res = new Resource(path, false);
	res->setData( (byte*)PointCharData, length );			//有错误

	
	//
	string ext="";
	string mity="";
	size_t bpos= path.find_last_of(".");
	//cout<<"bpos"<<bpos<<endl; 						//test
	if( bpos != string::npos){
		//cout<<"	MimeType Found"<<endl;				//test
		ext = path.substr(bpos+1);
		cout<<"	extration:"<<ext<<endl;					//test
		mity = getMimeType(ext);
	}else{
		perror("	MimeType Not Found");
		return NULL;
	}
	res->setMimeType(mity);
	cout<<"resourceHost readFile success ,resource size:"<<res->getSize()<<endl;//test
	byte* dataBytes = res->getData();				//test
	dataBytes[res->getSize()] = '\0';				//test
	cout<<"	### res get data:"<<dataBytes<<endl;	//test	
	//delete dataBytes;
	return res;
}










