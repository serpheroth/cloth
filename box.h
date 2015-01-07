#ifndef _BOX_H_
#define _BOX_H_

#include"object.h"

class Box:public Object{
public:
	Box();
	~Box();

	void build();

	void checkCollision(glm::vec3& p,glm::vec3& pp)override;

	inline void setVertices(float a, float b, float c){ xlen = a; ylen = b; zlen = c; }
private:
	float xlen, ylen, zlen;
};

#endif