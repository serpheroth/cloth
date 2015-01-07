#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include "box.h"
#include "constants.h"
#include <iostream>

#include "util.h"
using namespace std;

Box::Box() :xlen(2.0f), ylen(2.0f), zlen(2.0f)
{}

Box::~Box(){}

void Box::build(){
	float a = xlen / 2, b = ylen / 2, c = zlen / 2;
	
	// Front face
	pos.push_back(cpos+scale*glm::vec3(-a, -b,  c));
	pos.push_back(cpos+scale*glm::vec3( a, -b,  c));
	pos.push_back(cpos+scale*glm::vec3( a,  b,  c));
	pos.push_back(cpos+scale*glm::vec3(-a,  b,  c));

	// Back face
	pos.push_back(cpos+scale*glm::vec3(-a, -b, -c));
	pos.push_back(cpos+scale*glm::vec3(-a,  b, -c));
	pos.push_back(cpos+scale*glm::vec3( a,  b, -c));
	pos.push_back(cpos+scale*glm::vec3( a, -b, -c));

	// Top face
	pos.push_back(cpos+scale*glm::vec3(-a,  b, -c));
	pos.push_back(cpos+scale*glm::vec3(-a,  b,  c));
	pos.push_back(cpos+scale*glm::vec3( a,  b,  c));
	pos.push_back(cpos+scale*glm::vec3( a,  b, -c));

	// Bottom face
	pos.push_back(cpos+scale*glm::vec3(-a, -b, -c));
	pos.push_back(cpos+scale*glm::vec3( a, -b, -c));
	pos.push_back(cpos+scale*glm::vec3( a, -b,  c));
	pos.push_back(cpos+scale*glm::vec3(-a, -b,  c));

	// Right face
	pos.push_back(cpos+scale*glm::vec3( a, -b, -c));
	pos.push_back(cpos+scale*glm::vec3( a,  b, -c));
	pos.push_back(cpos+scale*glm::vec3( a,  b,  c));
	pos.push_back(cpos+scale*glm::vec3( a, -b,  c));

	// Left face
	pos.push_back(cpos+scale*glm::vec3(-a, -b, -c));
	pos.push_back(cpos+scale*glm::vec3(-a, -b,  c));
	pos.push_back(cpos+scale*glm::vec3(-a,  b,  c));
	pos.push_back(cpos+scale*glm::vec3(-a,  b, -c));

	for(std::size_t i=0;i<pos.size();i++){
		ppos.push_back(pos[i]);
		velocities.push_back(glm::vec3(0.0f));
	}
	
	faces.push_back(Face(0,  1,  2));faces.push_back(Face(  0,  2,  3));    // front
	faces.push_back(Face(4,  5,  6));faces.push_back(Face(  4,  6,  7));	// back
	faces.push_back(Face(8,  9,  10));faces.push_back(Face( 8,  10, 11));    // top
	faces.push_back(Face(12, 13, 14));faces.push_back(Face( 12, 14, 15));    // bottom
	faces.push_back(Face(16, 17, 18));faces.push_back(Face( 16, 18, 19));    // right
	faces.push_back(Face(20, 21, 22));faces.push_back(Face( 20, 22, 23));     // left
	
	
	std::vector<int> ncount;
	for(std::size_t i=0;i<ppos.size();i++){
		ncount.push_back(0);
		normals.push_back(glm::vec3(0.0f));
	}
	for(std::size_t i=0;i<faces.size();i++){
		Face f=faces[i];
		glm::vec3 p1=ppos[f.x];
		glm::vec3 p2=ppos[f.y];
		glm::vec3 p3=ppos[f.z];
		glm::vec3 n=glm::cross(p1-p2,p3-p2);
		normals[f.x]+=n;
		normals[f.y]+=n;
		normals[f.z]+=n;
		ncount[f.x]++;
		ncount[f.y]++;
		ncount[f.z]++;
	}
	for(std::size_t i=0;i<ppos.size();i++){
		if(ncount[i]!=0)
		normals[i]/=ncount[i];
		normals[i]=glm::normalize(normals[i]);
	}
	
	
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, pos.size()*sizeof(glm::vec3), &pos[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glGenBuffers(1,&normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER,normalbuffer);
	glBufferData(GL_ARRAY_BUFFER,normals.size()*sizeof(glm::vec3),&normals[0],GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER,0);

	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size()*3, &faces[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void Box::checkCollision(glm::vec3& p, glm::vec3& pp){
	glm::vec3 dp=pp-p;
	if(glm::length(dp)==0.0f)
		return;
	glm::vec3 localp = p - cpos;
	
	float localpx = glm::dot(localp, xdir);
	float localpy = glm::dot(localp, ydir);
	float localpz = glm::dot(localp, zdir);
	
	glm::vec3 localpp = pp - cpos;
	float localppx=glm::dot(localpp,xdir);
	float localppy=glm::dot(localpp,ydir);
	float localppz=glm::dot(localpp,zdir);
	
	if((localppx>-xlen/2&&localppx<xlen/2&&localppy>-ylen/2&&localppy<ylen/2&&localppz>-zlen/2&&localppz<zlen/2)||
		
		(localppx>-xlen/2&&localppx<xlen/2&&localppy>-ylen/2&&localppy<ylen/2&&localppz<-zlen/2&&localpz>zlen/2)||
		(localppx>-xlen/2&&localppx<xlen/2&&localppy>-ylen/2&&localppy<ylen/2&&localppz>zlen/2&&localpz<-zlen/2)||
			
		(localppy>-ylen/2&&localppy<ylen/2&&localppz>-zlen/2&&localppz<zlen/2&&localppx<-xlen/2&&localpx>xlen/2)||
		(localppy>-ylen/2&&localppy<ylen/2&&localppz>-zlen/2&&localppz<zlen/2&&localppx>xlen/2&&localpx<-xlen/2)||
				
		(localppz>-zlen/2&&localppz<zlen/2&&localppx>-xlen/2&&localppx<xlen/2&&localppy<-ylen/2&&localpy>ylen/2)||
		(localppz>-zlen/2&&localppz<zlen/2&&localppx>-xlen/2&&localppx<xlen/2&&localppy>ylen/2&&localpy<-ylen/2)
			
	){
		// x
		if ((localpx<-xlen / 2 && localppx>-xlen / 2) || (localppx>xlen / 2 && localpx<-xlen / 2)){
			pp = cpos + (-xlen / 2 - epsilon)*xdir + localppy*ydir + localppz*zdir;
		}
		if ((localppx<xlen / 2 && localpx>xlen / 2) || (localppx<-xlen / 2 && localpx>xlen / 2)){
			pp = cpos + (xlen / 2 + epsilon)*xdir + localppy*ydir + localppz*zdir;
		}
		// y
		if ((localpy<-ylen / 2 && localppy>-ylen / 2) || (localppy>ylen / 2 && localpy<-ylen / 2)){
			pp = cpos + (-ylen / 2 - epsilon)*ydir + localppx*xdir + localppz*zdir;
		}
		if ((localppy<ylen / 2 && localpy>ylen / 2) || (localppy<-ylen / 2 && localpy>ylen / 2)){
			pp = cpos + (ylen / 2 + epsilon)*ydir + localppx*xdir + localppz*zdir;
		}
		// z
		if ((localpz<-zlen / 2 && localppz>-zlen / 2) || (localppz>zlen / 2 && localpz<-zlen / 2)){
			pp = cpos + (-zlen / 2 - epsilon)*zdir + localppy*ydir + localppx*xdir;
		}
		if ((localppz<zlen / 2 && localpz>zlen / 2) || (localppz<-zlen / 2 && localpz>zlen / 2)){
			pp = cpos + (zlen / 2 + epsilon)*zdir + localppy*ydir + localppx*xdir;
		}
	}
}

