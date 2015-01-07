#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<cstdlib>
#include"util.h"

Face::Face():x(0),y(0),z(0){}

Face::Face(const Face& f):x(f.x),y(f.y),z(f.z){}

Face::Face(GLubyte _x,GLubyte _y,GLubyte _z):x(_x),y(_y),z(_z){}

Face::~Face(){}

GLubyte Face::operator[](int i){
	if(i==0)
		return x;
	else if(i==1)
		return y;
	else return z;
}

Face& Face::operator=(const Face& f){
	this->x=f.x;
	this->y=f.y;
	this->z=f.z;
	return *this;
}

std::string read_file(const char* fname){
	std::stringstream ss;
	std::ifstream fin(fname);
	if(!fin){
		std::cerr<<"Cannot open the file!"<<std::endl;
		exit(1);
	}
	ss<<fin.rdbuf();
	return ss.str();
}

void print_log(GLuint object){
	GLint log_length=0;
	if(glIsShader(object))
		glGetShaderiv(object,GL_INFO_LOG_LENGTH,&log_length);
	else if(glIsProgram(object))
		glGetProgramiv(object,GL_INFO_LOG_LENGTH,&log_length);
	else{
		std::cerr<<"printlog: not a shader or a program"<<std::endl;
		exit(1);
	}
	char* log=new char[log_length];
	if(glIsShader(object))
		glGetShaderInfoLog(object,log_length,NULL,log);
	else if(glIsProgram(object))
		glGetProgramInfoLog(object,log_length,NULL,log);
	std::cerr<<log<<std::endl;
	delete[] log;
}

GLuint create_shader(const char* fname,GLenum type){
	std::string source=read_file(fname);
	const char* sources[]={source.c_str()};
	GLuint result=glCreateShader(type);
	glShaderSource(result,1,sources,NULL);
	glCompileShader(result);
	GLint compile_ok=GL_FALSE;
	glGetShaderiv(result,GL_COMPILE_STATUS,&compile_ok);
	if(compile_ok==GL_FALSE){
		std::cerr<<"Error happens when compiling source code!"<<std::endl;
		print_log(result);
		glDeleteShader(result);
		exit(1);
	}
	return result;
}

GLuint create_program(const char* vfname,const char* ffname){
	GLuint program=glCreateProgram();
	GLuint shader;
	if(vfname!=""){
		shader=create_shader(vfname,GL_VERTEX_SHADER);
		glAttachShader(program,shader);
	}
	if(ffname!=""){
		shader=create_shader(ffname,GL_FRAGMENT_SHADER);
		glAttachShader(program,shader);
	}
	glLinkProgram(program);
	GLint link_ok=GL_FALSE;
	glGetProgramiv(program,GL_LINK_STATUS,&link_ok);
	if(!link_ok){
		std::cerr<<"Error happens when linking the program!"<<std::endl;
		print_log(program);
		glDeleteProgram(program);
		exit(1);
	}
	return program;
}

GLint attributeLocation(GLuint program,const char* aName){
	GLint attribID=glGetAttribLocation(program,aName);
	if(attribID==-1){
		std::cerr<<"Could not bind attribute "<<aName<<std::endl;
		exit(1);
	}
}

GLint uniformLocation(GLuint program,const char* aName){
	GLint attribID=glGetUniformLocation(program,aName);
	if(attribID==-1){
		std::cerr<<"Could not bind uniform "<<aName<<std::endl;
		exit(1);
	}
}
