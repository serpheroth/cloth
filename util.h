#ifndef __UTIL_H_
#define __UTIL_H_

#include <GL/glew.h>

struct Face{
	Face();
	Face(const Face& f);
	Face(GLubyte x,GLubyte y,GLubyte z);
	~Face();
	GLubyte operator[](int i);
	Face& operator=(const Face& f);
	
	GLubyte x,y,z;
};

GLuint create_program(const char* vfname,const char* ffname);
GLint attributeLocation(GLuint program,const char* aName);
GLint uniformLocation(GLuint program,const char* aName);

#endif
