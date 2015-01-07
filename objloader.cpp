#include<fstream>
#include<cstdlib>

#include<boost/algorithm/string.hpp>
#include<boost/lexical_cast.hpp>

#include"objloader.h"

#include "util.h"

void loadObj(std::string fname,std::vector<glm::vec3>& vertices,std::vector<Face>& faces,glm::vec3 &cpos, const glm::vec3& xdir,const glm::vec3& ydir,const glm::vec3 zdir, float scale){
	std::ifstream fin(fname);
	if(!fin){
		std::cerr<<"Cannot open the file!"<<std::endl;
		exit(1);
	}
	std::string line;
	std::vector<std::string> parts;
	while(std::getline(fin,line)){
		parts.clear();
		boost::trim(line);
		
		if(boost::starts_with(line,"v")){
			boost::split(parts,line,boost::is_any_of(" "));
			if(parts.size()!=4){
				std::cerr<<"The file has wrong format!"<<std::endl;
				exit(1);
			}
			glm::vec3 vertex=cpos+(boost::lexical_cast<float>(parts[1])*xdir+
			boost::lexical_cast<float>(parts[2])*ydir+
			boost::lexical_cast<float>(parts[3])*zdir)*scale;
			vertices.push_back(vertex);
		}else if(boost::starts_with(line,"f")){
			boost::split(parts,line,boost::is_any_of(" "));
			if(parts.size()!=4){
				std::cerr<<"The file has wrong format!"<<std::endl;
				exit(1);
			}
			GLubyte x=boost::lexical_cast<int>(parts[1])-1;
			GLubyte y=boost::lexical_cast<int>(parts[2])-1;
			GLubyte z=boost::lexical_cast<int>(parts[3])-1;
			faces.push_back(Face(x,y,z));
		}
	}
	fin.close();
}