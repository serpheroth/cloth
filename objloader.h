#ifndef _OBJLOADER_H_
#define _OBJLOADER_H_

#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "util.h"


void loadObj(std::string fname,std::vector<glm::vec3>& vertices,std::vector<Face>& faces,glm::vec3 &cpos, const glm::vec3& xdir,const glm::vec3& ydir,const glm::vec3 zdir, float scale);

#endif