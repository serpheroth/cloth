#define _USE_MATH_DEFINES 
#include<cmath>

#include <glm/gtc/type_ptr.hpp> 

#include<iostream>
using namespace std;

#include"constants.h"
#include"object.h"
#include"objloader.h"

Object::Object() :stretchStiffness(0.5f), bendingStiffness(0.5f), volume0(0.0f), scale(1.0f),
force(glm::vec3(0.0f)),cpos(glm::vec3(0.0f)), color(glm::vec3(1.0f)),
xdir(glm::vec3(1.0f, 0.0f, 0.0f)), ydir(glm::vec3(0.0f, 1.0f, 0.0f)), zdir(glm::vec3(0.0f, 0.0f, 1.0f)),thickness(0.5f)
{}

Object::~Object()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1,&vertexbuffer);
	glDeleteBuffers(1,&normalbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1,&elementbuffer);
}

void Object::selfCollision(){
	for(std::size_t i=0;i<ppos.size();i++){
		glm::vec3 q=ppos[i];
		for(std::size_t j=0;j<faces.size();j++){
			Face f=faces[j];
			
			if(i==f.x||i==f.y||i==f.z)
				continue;
			
			glm::vec3 p1=ppos[f.x];
			glm::vec3 p2=ppos[f.y];
			glm::vec3 p3=ppos[f.z];
			
			q-=p1;
			p2-=p1;
			p3-=p1;
			
			glm::vec3 p2p3=glm::cross(p2,p3);
			float normp2p3=glm::length(p2p3);
			if(normp2p3==0.0f)
				continue;
			glm::vec3 n=p2p3/normp2p3;	
			
			if(glm::dot(q,n)<0.0f)
				n=-n;
			
			float c=glm::dot(q,n)-thickness;
			
			if(c<0.0f){
				
				glm::mat3 A(p1.x,p2.x,p3.x,
							p1.y,p2.y,p3.y,
							p1.z,p2.z,p3.z);
							
				if(glm::determinant(A)==0.0f)
					continue;
				
				glm::vec3 r=glm::inverse(A)*glm::vec3(q-glm::dot(n,q)*n);
				if(r.x>=0.0f&&r.y>=0.0f&&r.z>=0.0f&&fabs(r.x+r.y+r.z-1.0f)<1e-5f)
					continue;
				
				glm::vec3 q1=n;
				glm::vec3 q2=(glm::cross(p3,q)+glm::dot(n,glm::cross(n,p3))*q)/normp2p3;
				glm::vec3 q3=(glm::cross(p2,q)+glm::dot(n,glm::cross(n,p2))*q)/normp2p3;
				glm::vec3 q4=-q1-q2-q3;
			
				float nominator=glm::dot(q1,q1)+glm::dot(q2,q2)+glm::dot(q3,q3)+glm::dot(q4,q4);
				if(nominator==0.0f)
					continue;
			
				float s=c/nominator;
			
				ppos[f.x]-=s*q4;
				ppos[f.y]-=s*q2;
				ppos[f.z]-=s*q3;
				ppos[i]-=s*q1;
				
			}
		}
	}
}

bool Object::rayTriangleIntersect(glm::vec3 orig, glm::vec3 dir, glm::vec3 vert0, glm::vec3 vert1, glm::vec3 vert2,float& t,float& u,float& v){
	glm::vec3 edge1=vert1-vert0;
	glm::vec3 edge2=vert2-vert0;
	
	glm::vec3 pvec=glm::cross(dir,edge2);
	float det=glm::dot(edge1,pvec);
	
	if(det<EPSILON)
		return false;
	
	glm::vec3 tvec=orig-vert0;
	u=glm::dot(tvec,pvec);
	if(u<0.0f||u>det)
		return false;
	glm::vec3 qvec=glm::cross(tvec,edge1);
	v=glm::dot(dir,qvec);
	if(v<0.0f||u+v>det)
		return false;
	
	t=glm::dot(edge2,qvec);
	float inv_det=1.0/det;
	t*=inv_det;
	u*=inv_det;
	v*=inv_det;
	
	return true;
	
}

void Object::predictPos(float timeStep){
	for (std::size_t i=0;i<pos.size();i++){
		velocities[i]+=force*timeStep;
		ppos[i]=pos[i]+velocities[i]*timeStep;
	}
}

void Object::update(float timeStep){
	for (std::size_t i=0;i<pos.size();i++){
		velocities[i]=(ppos[i]-pos[i])/timeStep;
		pos[i]=ppos[i];
	}
}

void Object::pinVertex(int index){
	if(index<0||index>pos.size())
		return;
	ppos[index]=pos[index];
}

void Object::load(std::string fname){
	loadObj(fname,pos,faces,cpos,xdir,ydir,zdir,scale);
	for (std::size_t i=0;i<pos.size();i++){
		ppos.push_back(pos[i]);
		velocities.push_back(glm::vec3(0.0f));
	}
	for(std::size_t i=0;i<faces.size();i++){
		Face f=faces[i];
		float dist=glm::length(pos[f.x]-pos[f.y]);
		edgeLen.push_back(dist);
		dist=glm::length(pos[f.y]-pos[f.z]);
		edgeLen.push_back(dist);
		dist=glm::length(pos[f.x]-pos[f.z]);
		edgeLen.push_back(dist);
	}
	for(std::size_t i=0;i<faces.size();i++){
		for(std::size_t j=i+1;j<faces.size();j++){
			int p1idx,p2idx,p3idx,p4idx;
			if(common(faces[i],faces[j],p1idx,p2idx,p3idx,p4idx)){
				glm::vec3 p1=ppos[p1idx];
				glm::vec3 p2=ppos[p2idx];
				glm::vec3 p3=ppos[p3idx];
				glm::vec3 p4=ppos[p4idx];
				
				p2 -= p1;
				p3 -= p1;
				p4 -= p1;

				glm::vec3 p2p3 = glm::cross(p2,p3);
				glm::vec3 p2p4 = glm::cross(p2,p4);

				float normp2p3 = glm::length(p2p3);
				if (normp2p3 == 0.0f)continue;
		
		
				float normp2p4 = glm::length(p2p4);
				if (normp2p4 == 0.0f)continue;

				glm::vec3 n1 = p2p3 / normp2p3;
				glm::vec3 n2 = p2p4 / normp2p4;

				float d = glm::dot(n1,n2);
				float phi = acos(d);
		
				if (d < -1.0f)
					d = -1.0f;
				else if (d>1.0f)
					d = 1.0f;
		
				if (d == -1.0f)
					phi = M_PI;
				else if (d == 1.0f)
					phi = 0.0f;
				
				angles.push_back(phi);
			}
		}
	}
	for (std::size_t i = 0; i < faces.size(); i++){
		Face f=faces[i];
		glm::vec3 p1 = ppos[f.x];
		glm::vec3 p2 = ppos[f.y];
		glm::vec3 p3 = ppos[f.z];
		volume0 += glm::dot(glm::cross(p1, p2), p3);
	}
	
	std::vector<int> ncount;
	for(std::size_t i=0;i<pos.size();i++){
		ncount.push_back(0);
		normals.push_back(glm::vec3(0.0f));
	}
	for(std::size_t i=0;i<faces.size();i++){
		Face f=faces[i];
		glm::vec3 p1=pos[f.x];
		glm::vec3 p2=pos[f.y];
		glm::vec3 p3=pos[f.z];
		glm::vec3 n=glm::cross(p1-p2,p3-p2);
		normals[f.x]+=n;
		normals[f.y]+=n;
		normals[f.z]+=n;
		ncount[f.x]++;
		ncount[f.y]++;
		ncount[f.z]++;
	}
	for(std::size_t i=0;i<pos.size();i++){
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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size()*sizeof(Face), &faces[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Object::updateNormals(){
	std::vector<int> ncount;
	for(std::size_t i=0;i<pos.size();i++){
		ncount.push_back(0);
		normals.push_back(glm::vec3(0.0f));
	}
	for(std::size_t i=0;i<faces.size();i++){
		Face f=faces[i];
		glm::vec3 p1=pos[f.x];
		glm::vec3 p2=pos[f.y];
		glm::vec3 p3=pos[f.z];
		glm::vec3 n=glm::cross(p1-p2,p3-p2);
		normals[f.x]=n;
		normals[f.y]=n;
		normals[f.z]=n;
		ncount[f.x]++;
		ncount[f.y]++;
		ncount[f.z]++;
	}
	for(std::size_t i=0;i<pos.size();i++){
		if(ncount[i]!=0)
		//normals[i]/=ncount[i];
		normals[i]=glm::normalize(normals[i]);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER,normalbuffer);
	glBufferData(GL_ARRAY_BUFFER,normals.size()*sizeof(glm::vec3),&normals[0],GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER,0);
}

void Object::render(){
	extern GLubyte colorID,lightdirID;
	glUniform3fv(colorID, 1, &color[0]);
	glUniform3fv(lightdirID, 1, &lightdir[0]);
	
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, pos.size()*sizeof(glm::vec3), &pos[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER,normalbuffer);
	glBufferData(GL_ARRAY_BUFFER,normals.size()*sizeof(glm::vec3),&normals[0],GL_STATIC_DRAW);
	glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(float)*3,(void*)0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glDrawElements(GL_TRIANGLES, faces.size()*3, GL_UNSIGNED_BYTE, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(0);
}

void Object::checkCollision(Object& obj){
	for (std::size_t i=0;i<pos.size();i++){
		obj.checkCollision(pos[i],ppos[i]);
	}
}

void Object::checkCollision(glm::vec3& p,glm::vec3& pp){
	
}

void Object::strechConstraint(){
	for(std::size_t i=0;i<faces.size();i++){
		Face f=faces[i];
		glm::vec3 dist=ppos[f.x]-ppos[f.y];
		float d=glm::length(dist);
		if(d==0.0f)
			continue;
		float d0=edgeLen[3*i];
		float delta=d-d0;
		glm::vec3 dp=dist*((d-d0)/(2.0f*d));
		ppos[f.x] -= dp*stretchStiffness;
		ppos[f.y] += dp*stretchStiffness;
		
		dist=ppos[f.y]-ppos[f.z];
		d=glm::length(dist);
		if(d==0.0f)
			continue;
		d0=edgeLen[3*i+1];
		delta=d-d0;
		dp=dist*((d-d0)/(2.0f*d));
		ppos[f.y] -= dp*stretchStiffness;
		ppos[f.z] += dp*stretchStiffness;
		
		dist=ppos[f.x]-ppos[f.z];
		d=glm::length(dist);
		if(d==0.0f)
			continue;
		d0=edgeLen[3*i+2];
		delta=d-d0;
		dp=dist*((d-d0)/(2.0f*d));
		ppos[f.x] -= dp*stretchStiffness;
		ppos[f.z] += dp*stretchStiffness;
	}
}

void Object::bendingConstraint(){
	int count=0;
	for(std::size_t i=0;i<faces.size();i++){
		for(std::size_t j=i+1;j<faces.size();j++){
			int p1idx,p2idx,p3idx,p4idx;
			if(common(faces[i],faces[j],p1idx,p2idx,p3idx,p4idx)){
				float phi0=angles[count++];
				glm::vec3 p1=ppos[p1idx];
				glm::vec3 p2=ppos[p2idx];
				glm::vec3 p3=ppos[p3idx];
				glm::vec3 p4=ppos[p4idx];
				
				p2 -= p1;
				p3 -= p1;
				p4 -= p1;

				glm::vec3 p2p3 = glm::cross(p2,p3);
				glm::vec3 p2p4 = glm::cross(p2,p4);

				float normp2p3 = glm::length(p2p3);
				if (normp2p3 == 0.0f)continue;


				float normp2p4 = glm::length(p2p4);
				if (normp2p4 == 0.0f)continue;

				glm::vec3 n1 = p2p3 / normp2p3;
				glm::vec3 n2 = p2p4 / normp2p4;

				float d = glm::dot(n1, n2);
				float phi = acos(d);

				if (d < -1.0f)
					d = -1.0f;
				else if (d>1.0f)
					d = 1.0f;

				if (d == -1.0f){
					phi = M_PI;
				}else if (d == 1.0f){
					phi = 0.0f;
				}

				if (phi == phi0)
					continue;


				glm::vec3 p2n2 = glm::cross(p2,n2);
				glm::vec3 p2n1 = glm::cross(p2,n1);
				glm::vec3 q3 = (p2n2 + (-p2n1)*d) / normp2p3;
				glm::vec3 q4 = (p2n1 + (-p2n2)*d) / normp2p4;
				glm::vec3 q2 = -(glm::cross(p3,n2) + (glm::cross(n1,p3))*d) / normp2p3 - (glm::cross(p4,n1) + (glm::cross(n2,p4))*d) / normp2p4;
				glm::vec3 q1 = -q2 - q3 - q4;

				float nominator = glm::dot(q1, q1) + glm::dot(q2, q2) + glm::dot(q3, q3) + glm::dot(q4, q4);
				if (nominator == 0.0f)
					continue;
				float coeff = -sqrt(1 - d*d)*(phi - phi0) / nominator;
				ppos[p1idx] += coeff*q1*bendingStiffness;
				ppos[p2idx] += coeff*q2*bendingStiffness;
				ppos[p3idx] += coeff*q3*bendingStiffness;
				ppos[p4idx] += coeff*q4*bendingStiffness;
			}
		}
	}
}

void Object::ballonConstraint(){
	float volume = 0.0f;
	for (std::size_t i = 0; i < faces.size(); i++){
		Face f=faces[i];
		glm::vec3 p1 = ppos[f.x];
		glm::vec3 p2 = ppos[f.y];
		glm::vec3 p3 = ppos[f.z];
		volume += glm::dot(glm::cross(p1, p2), p3);
	}
	std::vector<glm::vec3> gradients;
	for(std::size_t i=0;i<ppos.size();i++){
		gradients.push_back(glm::vec3(0.0f));
	}
	for(std::size_t i=0;i<faces.size();i++){
		Face f=faces[i];
		glm::vec3 p1=ppos[f.x];
		glm::vec3 p2=ppos[f.y];
		glm::vec3 p3=ppos[f.z];
		
		gradients[f.x]+=glm::cross(p2,p3);
		gradients[f.y]+=glm::cross(p3,p1);
		gradients[f.z]+=glm::cross(p1,p2);
	}
	float sum = 0.0f;
	for(std::size_t i=0;i<ppos.size();i++){
		sum+=glm::dot(gradients[i],gradients[i]);
	}
	for(std::size_t i=0;i<ppos.size();i++){
		glm::vec3 deltap=-gradients[i]*(volume-kpressure*volume0)/sum;
		ppos[i]+=deltap;
	}
}

bool Object::common(const Face& i1,const Face& i2,int& p1,int& p2,int& p3,int& p4){
#define IN(a,b,i) (((a==i.x&&b==i.y)||(a==i.y&&b==i.x))?i.z:\
				   ((a==i.x&&b==i.z)||(a==i.z&&b==i.x))?i.y:\
				   ((a==i.y&&b==i.z)||(a==i.z&&b==i.y))?i.x:-1)
					   int a;
	if((a=IN(i1.x,i1.y,i2))>0){
		p1=i1.x;p2=i1.y;
		p3=i1.z;p4=a;
	}else if((a=IN(i1.x,i1.z,i2))>0){
		p1=i1.x;p2=i1.z;
		p3=i1.y;p4=a;
	}else if((a=IN(i1.y,i1.z,i2))>0){
		p1=i1.y;p2=i1.z;
		p3=i1.x;p4=a;
	}else
		return false;
	return true;
#undef IN
}

void Object::dampVelocities(){
	glm::vec3 xcm = glm::vec3(0.0f);
	glm::vec3 vcm = glm::vec3(0.0f);
	for(std::size_t i=0;i<pos.size();i++){
		xcm += ppos[i];
		vcm += velocities[i];
	}
	xcm /= pos.size();
	vcm /= pos.size();

	glm::vec3 L = glm::vec3(0.0f);
	for(std::size_t i=0;i<pos.size();i++){
		glm::vec3 ri = ppos[i] - xcm;
		L += glm::cross(ri,velocities[i]);
	}

}

void Object::rayIntersect(const glm::vec3& cameraPos,const glm::vec3& dir){
	extern int draggingIdx;
	float dSqrMin=1e10f;
	int total=pos.size()/threadNum;
	for(int i=0;i<threadNum;i++){
		threads.push_back(std::thread([&,i](){
			for(int j=i*total;j<(i+1)*total;j++){
				glm::vec3 p=ppos[j];
				float t=-glm::dot(dir,(cameraPos-p))/glm::dot(dir,dir);
				if(t<0.0f)
					continue;
				glm::vec3 d=cameraPos+dir*t-p;
				float dSqr=glm::dot(d,d);
				if(dSqr>rangeSqr)
					continue;
				if(dSqr<dSqrMin){
					dSqrMin=dSqr;
					draggingIdx=j;
				}
			}
		}));
	}
	for(int i=0;i<threadNum;i++)
		threads[i].join();
	
	threads.clear();
}

void Object::drag(const glm::vec3& cameraPos,const glm::vec3& dir){
	
	extern int draggingIdx;
	float t=-glm::dot(dir,(cameraPos-ppos[draggingIdx]))/glm::dot(dir,dir);
	pos[draggingIdx]=ppos[draggingIdx]=cameraPos+dir*t;
}





