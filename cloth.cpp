#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <thread>

#include "util.h"
#include "object.h"
#include "box.h"
#include "constants.h"

#include <iostream>

// for camera
float cangle=0.0f;
float cradius=20.0f;
glm::vec3 cameraPos=glm::vec3(0.0f,0.0f,-cradius);
glm::vec3 lookAt=glm::vec3(0.0f,0.0f,0.0f);
glm::vec3 cameraUp=glm::vec3(0.0f,1.0f,0.0f);
float cdeltaAngle=0;
float fovy=50.0f;
float near=0.1f,far=500.0f;
glm::vec3 view,h,v;

// for OpenGL
int width=2560,height=1600;
float ratio=(float)width/(float)height;
GLuint VertexArrayID, programID, MVPID, colorID, lightdirID;
glm::mat4 MVP, Projection, View;
GLFWwindow* window;
GLuint polygonMode = GL_LINE;

// for objects
Object cloth;
Box box;
Box ground;

float groundY = -5.0f;
int iterNum = 8;

// for frame rate
int timeStep = 1;
typedef std::chrono::duration<int, std::ratio<1, 60>> frame_duration;

// for control
bool paused=true;
int draggingIdx=-1;
bool pin=true;

void initializeObjects(){
	ground.setColor(0.6f,0.6f,0.6f);
	ground.setVertices(1600.0f,1.0f,1600.0f);
	ground.setPosition(0.0f,groundY,0.0f);
	ground.build();
	
	box.setColor(0.0f,0.7f,0.0f);
	box.setPosition(1.0f,groundY+1.5f,1.0f);
	box.build();
	
	//cloth.setOrientation(glm::vec3(0.1f,1.0f,0.0f),glm::vec3(1.0f,0.0f,0.1f),glm::vec3(0.0f,0.0f,1.0f));
	cloth.setPosition(0.0f,2.0f,0.0f);
	cloth.setKPressure(3.0f);
	
	cloth.setStretchStiffness(0.9f, iterNum);
	cloth.setBendingStiffness(0.9f, iterNum);
	cloth.applyForce(gravity);
	cloth.setColor(0.58f,0.0f,0.83f);
	cloth.setScale(5.f);
	cloth.load("models/flat-mesh.obj");
}

void initialize(){
	
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.9f, 0.9f, 0.9f, 0.0f);
	//glPolygonOffset(0.9f,0.9f);

	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	
	initializeObjects();	
	
	programID = create_program( "shaders/normal.vs","shaders/normal.fs" );
	MVPID = glGetUniformLocation(programID, "MVP");
	colorID = glGetUniformLocation(programID,"color");
	lightdirID = glGetUniformLocation(programID,"lightdir");
	
	
	Projection=glm::perspective(fovy,ratio,near,far);
	View=glm::lookAt(cameraPos, lookAt,cameraUp);
	MVP = Projection * View;
	
	view=glm::normalize(lookAt-cameraPos);
	h=glm::normalize(glm::cross(view,cameraUp));
	v=glm::normalize(glm::cross(h,view));
	
	float rad=fovy*M_PI/180.0f;
	float vLength=tan(rad/2)*near;
	float hLength=vLength*width/height;
	
	v*=vLength;
	h*=hLength;
}


void renderScene(void){
	
	glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
	
	glUseProgram(programID);
	glUniformMatrix4fv(MVPID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(lightdirID, 1, &lightdir[0]);
	
	
	box.render();
	ground.render();
	cloth.render();
}


void keyPress(GLFWwindow* window, int key, int scancode, int action, int mods){
	if(action!=	GLFW_PRESS&&action!=GLFW_REPEAT)
		return;
	switch (key){
		case GLFW_KEY_LEFT:
		case GLFW_KEY_A:
			cangle+=0.05f;
			cameraPos=glm::vec3(cradius*sin(cangle),cameraPos.y,-cradius*cos(cangle));
			break;
		case GLFW_KEY_RIGHT:
		case GLFW_KEY_D:
			cangle-=0.05f;
			cameraPos=glm::vec3(cradius*sin(cangle),cameraPos.y,-cradius*cos(cangle));
			break;
		case GLFW_KEY_L:
			polygonMode = GL_LINE;
			break;
		case GLFW_KEY_F:
			polygonMode = GL_FILL;
			break;
		case GLFW_KEY_UP:
		case GLFW_KEY_W:
			cameraPos+=glm::vec3(0.0f,1.0f,0.0f);
			break;
		case GLFW_KEY_DOWN:
		case GLFW_KEY_S:
			cameraPos+=glm::vec3(0.0f,-1.0f,0.0f);
			break;
		case GLFW_KEY_P:
			paused=true;
			break;
		case GLFW_KEY_R:
			paused=false;
			break;
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
		case GLFW_KEY_I:
			pin=true;
			break;
		case GLFW_KEY_U:
			pin=false;
			break;
	}
	Projection=glm::perspective(fovy,ratio,near,far);
	View=glm::lookAt(cameraPos, lookAt, cameraUp);
	MVP = Projection * View;
	
	view=glm::normalize(lookAt-cameraPos);
	h=glm::normalize(glm::cross(view,cameraUp));
	v=glm::normalize(glm::cross(h,view));
	
	float rad=fovy*M_PI/180.0f;
	float vLength=tan(rad/2)*near;
	float hLength=vLength*width/height;
	
	v*=vLength;
	h*=hLength;
}

void mousePress(GLFWwindow* window, int button,int action, int mods){
	
	if(button==GLFW_MOUSE_BUTTON_LEFT&&action==GLFW_RELEASE){
		draggingIdx=-1;
		return;
	}else if(button==GLFW_MOUSE_BUTTON_LEFT&&action==GLFW_PRESS){
		double xpos,ypos;
		glfwGetCursorPos(window,&xpos,&ypos);
		float x=xpos;float y=ypos;

		x-=width/2.0f;
		y-=height/2.0f;

		x/=(width/2.0f);
		y/=-(height/2.0f);


		glm::vec3 p=cameraPos+view*near+h*x+v*y;
		glm::vec3 dir=glm::normalize(p-cameraPos);
		
		cloth.rayIntersect(cameraPos,dir);
	}
}

void mouseMove(GLFWwindow *, double xpos, double ypos){
	if(draggingIdx<0)
		return;
	float x=xpos;float y=ypos;

	x-=width/2.0f;
	y-=height/2.0f;

	x/=(width/2.0f);
	y/=-(height/2.0f);


	glm::vec3 p=cameraPos+view*near+h*x+v*y;
	glm::vec3 dir=glm::normalize(p-cameraPos);
	cloth.drag(cameraPos,dir);
}

void update(){
	cloth.predictPos(timeStep/60.0f);
	for(int i=0;i<iterNum;i++){
		//cloth.ballonConstraint();
		//cloth.selfCollision();
		cloth.strechConstraint();
		cloth.bendingConstraint();
		cloth.checkCollision(box);
		cloth.checkCollision(ground);
		if(pin){
			cloth.pinVertex(0);
			cloth.pinVertex(10);
			cloth.pinVertex(20);
		}
	}
	cloth.update(timeStep/60.0f);
	cloth.updateNormals();
	
	cloth.dampVelocities();
}


void cleanup(){
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);
}

int main( void )
{
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}
	
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	window = glfwCreateWindow( width, height, "Cloth - Pu", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	
	glewExperimental=GL_TRUE;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GL_TRUE);
	
	glfwSetKeyCallback(window, keyPress);
	glfwSetMouseButtonCallback(window,mousePress);
	glfwSetCursorPosCallback(window,mouseMove);
	
	
	initialize();

	do{
		auto start_time = std::chrono::steady_clock::now();
		
		renderScene();
		glfwSwapBuffers(window);
		glfwPollEvents();
		if(!paused)
		update();
		
		auto end_time = start_time + frame_duration(timeStep);
		std::this_thread::sleep_until(end_time);
		
	}while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	cleanup();

	glfwTerminate();

	return 0;
}
