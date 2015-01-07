#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <thread>

#include "util.h"

class Object{
public:
	Object();
	~Object();

	
	void checkCollision(Object& obj);
	virtual void checkCollision(glm::vec3& p,glm::vec3& pp);

	void pinVertex(int index);
	void predictPos(float timeStep);
	void update(float timeStep);
	void strechConstraint();
	void bendingConstraint();
	void ballonConstraint();
	void selfCollision();
	
	inline void setOrientation(glm::vec3 xdir, glm::vec3 ydir, glm::vec3 zdir){ this->xdir = xdir; this->ydir = ydir; this->zdir = zdir; }
	inline void setPosition(float x, float y, float z){ this->cpos = glm::vec3(x, y, z); }
	inline void applyForce(glm::vec3 force){ this->force += force; }
	inline void setColor(float r, float g, float b){ this->color = glm::vec3(r, g, b); }
	inline void setScale(float scale){ this->scale = scale; }
	inline void setKPressure(float k){this->kpressure=k;}
	inline void setStretchStiffness(float k, int iterNum){ stretchStiffness = 1.0f - pow(1.0f - k, 1.0f / iterNum); }
	inline void setBendingStiffness(float k, int iterNum){ bendingStiffness = 1.0f - pow(1.0f - k, 1.0f / iterNum); }
	inline const std::vector<glm::vec3>& getPos(){return pos;}
	inline const std::vector<Face>& getFaces(){return faces;}
	inline void setThickness(float t){this->thickness=t;}
	
	void render();
	void load(std::string fname);
	void dampVelocities();
	
	void updateNormals();
	
	void drag(const glm::vec3& cameraPos,const glm::vec3& dir);
	void rayIntersect(const glm::vec3& cameraPos,const glm::vec3& dir);

protected:
	inline bool common(const Face& i1,const Face& i2,int& p1,int& p2,int& p3,int& p4); 
	bool rayTriangleIntersect(glm::vec3 orig, glm::vec3 dir, glm::vec3 vert0, glm::vec3 vert1, glm::vec3 vert2,float& t,float& u,float& v);
	
	glm::vec3 force;
	float scale;
	glm::vec3 color;
	
	glm::vec3 cpos;

	glm::vec3 xdir, ydir, zdir;

	std::vector<glm::vec3> pos;
	std::vector<glm::vec3> ppos;
	std::vector<glm::vec3> velocities;
	std::vector<glm::vec3> normals;
	std::vector<Face> faces;
	std::vector<float> edgeLen;
	std::vector<float> angles;
	
	GLuint vertexbuffer,elementbuffer,normalbuffer;
	
	float stretchStiffness;
	float bendingStiffness;

	float volume0;
	float kpressure;
	float thickness;
	
	std::vector<std::thread> threads;
	
};

#endif