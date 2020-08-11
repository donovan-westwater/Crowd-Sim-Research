
//#define _ITERATOR_DEBUG_LEVEL 0
//#define _DEBUG_LT_PRED(pred, x, y)    pred(x, y)

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <iterator> 
#include <unordered_map> 
#include <string>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <fstream>
//#include <Windows.h>

#ifdef __APPLE__
/* Defined before OpenGL and GLUT includes to avoid deprecation messages */
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
//include <GLFW/glfw3.h>
#else

//#include <GL/GL.h>
//#include <GL/GL.h>
#include <glut.h>
#endif
#include <vector>

using namespace std;

#define ZFAR 1.0 //Controls z-axis, No longer relevant
#define PI 3.14159265f //Estimation of PI
#define ARADIS 0.5 //Old Radius: 10 This controls the agent's radius since all agents are circles
#define TIME_STEP 0.25 // Old TIMESTEP 0.5 This controls how fast the agents move / distance agents move each update
#define NUMOFAGENTS 300 //How many agents does the sim manage/ create
#define NUMOFOBSTACLES 4 //How many obstacles does the sim manage/ create

int loopCount = 1; //Debug variable, used to count how many times a for/while loop has looped
int compteCount = 0; //Debug variable, used to count how many times has the sim looped on the whole

//FPS
//float curTime = 0; //Unused variable
double lasttime = 0;    //Last time returned by time(null)
float totaltime = 0;    //Time since sim has started
float frames = 0;       //Total num of frames since sim as started
float curFps = 0;       //the current fps of the frame
float fps_avg = 0;      //The average fps of the sim since it ran
float updateTime = 0;   //time between updates
float updateFrames = 0; //frames between updates
//ARGS
bool anim = false;      //Animates some circles moving if true
bool crowd = false;     //Animates a larger group of circles if true
bool shash = false;     //Runs a scenario using the spatial hash version of collision dectection 
bool favoid = false;    //Runs a scenario wthout using the spatial hash version of collision dectection
bool fps = false;       //If true, displays FPS
bool stats = false;     //If true, displays the ID, Direction of movment and current coords of the agents
//SCNEARIO ARGS
bool basic = true;         //Scenario where all agents are randomly placed with randomized goals to head to
bool agent_circle = false; //Scenario where all agents are placed in a circle and move to the opposite side
bool con_circle = false;   //Scenario where all agents are placed in etheir an inner or outer cirlce and the move to swap postions with inner and outer
bool agent_swap = false;   //Scenario where agents swap postions directly across from eachtother. WORKS ONLY WITH NUMOFAGENTS set to 2!
bool crossFlow = false;    //Scenario where two clusters of agents move past eachother to the other side of the sim
bool wallTest = false;     //Scenario meant to test collisions with walls 
//COLLISION DECTION
float k = 1.5;
float t0 = 3.0000000; //2 - 4
float m = 2;
double fAvoid_x = 0;
double fAvoid_y = 0;

typedef struct Square;
void draw_circle(float x, float  y, float z, bool atGoal);
void draw_quad(Square *self,float x, float y, float z);
void step(int i);
void hashStep(int i);

typedef pair<int, int> coord; //For hashmap
typedef struct Circle {
    int id; //Unique ID of the Agent
    //coord cell = { -555, -555 }; //Used for Spatial Hash //was oringaly string
    //coord adjcells[4] = { {-555, -555} ,{-555, -555} ,{-555, -555} ,{-555, -555} }; //Used for Spatial Hash //Was oringally string[4]
    GLfloat x; //X position of agent
    GLfloat y; //Y position of agent
    GLfloat z = 0;
    GLfloat dirX; //X velocity of agent
    GLfloat dirY; //Y velocity of agent
    GLfloat goal_x; //X goal position of agent
    GLfloat goal_y; //Y goal position of agent
    bool circleDrag = false;
    bool isEmpty = true; //Determines if the obstacle is being used
    bool atGoal = false; //Is the agent at their goal?
    void (*draw)(float x, float y, float z, bool atGoal) { draw_circle }; //Pointer to the function that draws the agent
    void (*stepF)(int i) { step }; //Pointer to the function that calculates collision avoidence
};
typedef struct Square {
    int id; //Used for spatial hash
    //coord cell = { -555, -555 }; //Used for Spatial Hash //was oringaly string
    //coord adjcells[4] = { {-555, -555} ,{-555, -555} ,{-555, -555} ,{-555, -555} }; //Used for Spatial Hash //Was oringally string[4]
    GLfloat x; //X position of obstacle
    GLfloat y; //Y position of obstacle
    GLfloat z = 0;
    GLfloat wid; //The Width of the obstacle
    GLfloat length; //The Length of the obstacle
    GLfloat goal_x; //Isn't used on obstacles
    GLfloat goal_y; //Isn't used on obstacles
    bool circleDrag = false;
    bool isEmpty = true; //Determines if the obstacle is being used
    bool atGoal = false; //Not used on obstacles
    void (*draw)(Square *self,float x, float y, float z) { draw_quad }; //Pointer to the function that draws the obstacle
    //void (*stepF)(int i) { step };
};
Circle manager[NUMOFAGENTS] = { 0 };      //Stores all the agents in the sim
Square obstacles[NUMOFOBSTACLES] = { 0 }; //Stores all the obstacles in the sim

#define ENVIRO_SIZE 50 //Width and length of the enviroment
#define CELL_SIZE 1//2//25 //Width and length of the cells used in spatial hash
#define BucketSize 20 //How many agents can be stored in a bucket used in spatial hash 
#define width (ENVIRO_SIZE)/CELL_SIZE //(ENVIRO_SIZE/2) - -(ENVIRO_SIZE/2) Controls how many spatial hash buckets there are
int spatialHash[width * width][BucketSize];
/*
Description: returns the index of the bucket that corresponds to the cell that contains the coords given
Params: the x and y coords 
Return Index to a bucket corresponding to the cell with the coord inside it
*/
int hashFun(float x, float y) {
    int xpart = (int)(x / CELL_SIZE) * CELL_SIZE;
    int ypart = (int)(y / CELL_SIZE) * CELL_SIZE;
    int out = (((xpart + (ENVIRO_SIZE / 2)) / CELL_SIZE) + ((ypart + (ENVIRO_SIZE / 2)) / CELL_SIZE) * width);
    return out;
}

//Note from Donovan: I hope to rewrite this to be less of a mess of ifs
/*
Description: Initializes the scenario, collsion dectection, and FPS variables. Does this based on the flags turned on by command line input
Params: None
Return None
*/
void initGL() {
    /*
    if (mouse) {
        manager[0].x = 0;
        manager[0].y = 0;
        manager[0].isEmpty = false;

        manager[1].x = 20;
        manager[1].y = -20;
        manager[1].isEmpty = false;
    }
    */
    if (anim) {
        for (int i = 0; i < 3; i++) {
            manager[i].x = (float)(rand() % 3) - 1.5;
            manager[i].y = (float)(rand() % 3) - 1.5;
            manager[i].dirX = (float)(rand() % 3) - 1.5;
            manager[i].dirY = (float)(rand() % 3) - 1.5;
            manager[i].isEmpty = false;
        }
    }
    if (crowd) {
        for (int i = 0; i < NUMOFAGENTS; i++) {
            manager[i].x = (float)(rand() % 7) - 3.5;
            manager[i].y = (float)(rand() % 7) - 3.5;
            manager[i].dirX = (float)(rand() % 7) - 3.5;
            manager[i].dirY = (float)(rand() % 7) - 3.5;
            manager[i].isEmpty = false;
        }
    }
    
    if (favoid) { //Normal step fucntion
        for (int i = 0; i < NUMOFAGENTS; i++) {
            manager[i].stepF = step;
            manager[i].id = i;
            manager[i].isEmpty = false;
        }
    }
    if (fps) {
        lasttime = time(NULL);
    }
    if (shash) { //Spatial hash step function
        for (int i = 0; i < NUMOFAGENTS; i++) {
            manager[i].stepF = hashStep;
            manager[i].id = i;
            manager[i].isEmpty = false;
            // manager[i].cell = { NULL,NULL };
            // manager[i].adjcells[0] = { NULL,NULL };
            // manager[i].adjcells[1] = { NULL,NULL };
             //manager[i].adjcells[2] = { NULL,NULL };
            // manager[i].adjcells[3] = { NULL,NULL };
        }
        memset(spatialHash, -1, sizeof(int) * width * width * BucketSize);

    }
    if (basic) { //Default Scenario (See basic variable to description of this scneario)
        for (int i = 0; i < NUMOFAGENTS; i++) {
            if (!favoid && !shash) manager[i].stepF = step;
            manager[i].x = (float)(rand() % ENVIRO_SIZE) - (ENVIRO_SIZE / 2); //(float)(rand() % 1000) - 500; //(i + 1) % 2 == 0 ? manager[i].x = 0 : manager[i].x = 5; 10*cos(ang*i);
            manager[i].y = (float)(rand() % ENVIRO_SIZE) - (ENVIRO_SIZE / 2); //(float)(rand() % 1000) - 500;//(i + 1) % 2 == 0 ? manager[i].y = 250 : manager[i].y = -150; 10*sin(ang * i);
            manager[i].dirX = 0;//(float)(rand() % 10) - 5;   //0;
            manager[i].dirY = 0;//(float)(rand() % 10) - 5;   //(i + 1) % 2 == 0 ? manager[i].dirY = -0.5 : manager[i].dirY = 0.5;
            manager[i].goal_x = (float)(rand() % ENVIRO_SIZE) - (ENVIRO_SIZE / 2); //(float)(rand() % 1000) - 500; //manager[i].dirX;
            manager[i].goal_y = (float)(rand() % ENVIRO_SIZE) - (ENVIRO_SIZE / 2); //(float)(rand() % 1000) - 500; //manager[i].dirY;
            manager[i].isEmpty = false;
            manager[i].id = i;

            float limit = (ENVIRO_SIZE / 2);
            if (manager[i].x > limit) manager[i].x = -limit + 1;
            if (manager[i].x < -limit) manager[i].x = limit - 1;
            if (manager[i].y > limit) manager[i].y = -limit + 1;
            if (manager[i].y < -limit) manager[i].y = limit - 1;
        }

    }
    else if (agent_circle) { //Circle Scenario (See agent_circle variable to description of this scneario)
        for (int i = 0; i < NUMOFAGENTS; i++) {
            float ang = 2 * PI / NUMOFAGENTS;
            if (!favoid && !shash) manager[i].stepF = step;
            manager[i].x = 10 * cos(ang * i);//(float)(rand() % ENVIRO_SIZE) - (ENVIRO_SIZE / 2); 
            manager[i].y = 10 * sin(ang * i);//(float)(rand() % ENVIRO_SIZE) - (ENVIRO_SIZE / 2); 
            manager[i].dirX = 0; //i % 2 == 0 ? 1 : -1;
            manager[i].dirY = 0; //i % 2 == 0 ? 1 : -1;
            manager[i].goal_x = -manager[i].x;//(float)(rand() % ENVIRO_SIZE) - (ENVIRO_SIZE / 2); //i % 2 == 0 ? 1 : -1;
            manager[i].goal_y = -manager[i].y;//(float)(rand() % ENVIRO_SIZE) - (ENVIRO_SIZE / 2); //i % 2 == 0 ? 1 : -1;

            float limit = (ENVIRO_SIZE / 2);
            if (manager[i].x > limit) manager[i].x = -limit + 1;
            if (manager[i].x < -limit) manager[i].x = limit - 1;
            if (manager[i].y > limit) manager[i].y = -limit + 1;
            if (manager[i].y < -limit) manager[i].y = limit - 1;
        }
    }
    else if (con_circle) { //Concentric Circle Scenario (See con_circle variable to description of this scneario)
        float dist = 1;
        int index = 1;
        //float rotateCheck = 2*PI;
        for (int i = 0; i < NUMOFAGENTS; i++) {
            float halfAg = (NUMOFAGENTS / 2);
            float ang = 2*PI/halfAg;
            if (i % 2 == 0) {
                dist = 2;
                manager[i].goal_x = -5 * cos(ang * index);
                manager[i].goal_y = -5 * sin(ang * index);
                index++;
            }
            else {
                dist = 1;
                manager[i].goal_x = -5 *2* cos(ang * index);
                manager[i].goal_y = -5 *2* sin(ang * index);

            }
            if (!favoid && !shash) manager[i].stepF = step;
            manager[i].x = 5 *dist* cos(ang * index);//(float)(rand() % ENVIRO_SIZE) - (ENVIRO_SIZE / 2); 
            manager[i].y = 5 *dist* sin(ang * index);//(float)(rand() % ENVIRO_SIZE) - (ENVIRO_SIZE / 2); 
            manager[i].dirX = 0; //i % 2 == 0 ? 1 : -1;
            manager[i].dirY = 0; //i % 2 == 0 ? 1 : -1;
            //manager[i].goal_x = -manager[i].x;//(float)(rand() % ENVIRO_SIZE) - (ENVIRO_SIZE / 2); //i % 2 == 0 ? 1 : -1;
            //manager[i].goal_y = -manager[i].y;//(float)(rand() % ENVIRO_SIZE) - (ENVIRO_SIZE / 2); //i % 2 == 0 ? 1 : -1;

            float limit = (ENVIRO_SIZE / 2);
            if (manager[i].x > limit) manager[i].x = -limit + 1;
            if (manager[i].x < -limit) manager[i].x = limit - 1;
            if (manager[i].y > limit) manager[i].y = -limit + 1;
            if (manager[i].y < -limit) manager[i].y = limit - 1;
        }
    }
    else if (agent_swap) { //Agents Swapping Positions Scenario (See agent_swap variable to description of this scneario)
        for (int i = 0; i < NUMOFAGENTS; i++) {
            if (!favoid && !shash) manager[i].stepF = step;
            float ang = PI;
            manager[i].x = i == 1 ? -10 : 10;//10 * cos(ang * i);
            manager[i].y = 0;//10 * sin(ang * i);
            manager[i].dirX = 0; //i % 2 == 0 ? 1 : -1;
            manager[i].dirY = 0; //i % 2 == 0 ? 1 : -1;
            manager[i].goal_x = -manager[i].x;//(float)(rand() % ENVIRO_SIZE) - (ENVIRO_SIZE / 2); //i % 2 == 0 ? 1 : -1;
            manager[i].goal_y = -manager[i].y;//(float)(rand() % ENVIRO_SIZE) - (ENVIRO_SIZE / 2); //i % 2 == 0 ? 1 : -1;

            float limit = (ENVIRO_SIZE / 2);
            if (manager[i].x > limit) manager[i].x = -limit + 1;
            if (manager[i].x < -limit) manager[i].x = limit - 1;
            if (manager[i].y > limit) manager[i].y = -limit + 1;
            if (manager[i].y < -limit) manager[i].y = limit - 1;
        }
    }
    //Placeholder
    else if (wallTest) { //Scearnio that tests obstace collsion (See wallTest variable to description of this scneario)
        for (int i = 0; i < NUMOFAGENTS; i++) {
            float ang = 2 * PI / NUMOFAGENTS;
            if (!favoid && !shash) manager[i].stepF = step;
            manager[i].x = 10 * cos(ang * i);
            manager[i].y = 10 * sin(ang * i);
            manager[i].dirX = 0; 
            manager[i].dirY = 0; 
            manager[i].goal_x = -manager[i].x;
            manager[i].goal_y = -manager[i].y;
        }
        for (int i = 0; i < NUMOFOBSTACLES; i++) {
            //if (obstacles[i].isEmpty) break;
            if(i == 0){
                obstacles[i].x = 0;
                obstacles[i].y = 10;
                obstacles[i].wid = 10;
                obstacles[i].length = 2;
                obstacles[i].isEmpty = false;
            }
            if (i == 1) {
                obstacles[i].x = 0;
                obstacles[i].y = -10;
                obstacles[i].wid = 10;
                obstacles[i].length = 2;
                obstacles[i].isEmpty = false;
            }
            if (i == 2) {
                obstacles[i].x = -5;
                obstacles[i].y = 5;
                obstacles[i].wid = 2;
                obstacles[i].length = 10;
                obstacles[i].isEmpty = false;
            }
            if (i == 3) {
                obstacles[i].x = -5;
                obstacles[i].y = -5;
                obstacles[i].wid = 2;
                obstacles[i].length = 10;
                obstacles[i].isEmpty = false;
            }
        }
    }
    //Agent Group Flow Scenario (See crossFlow variable to description of this scneario)
    else if (crossFlow) {
        int space = 1;
        int row = 0;
        for (int i = 0; i < NUMOFAGENTS; i++) {
            if (!favoid && !shash) manager[i].stepF = step;
            if (i % 2 == 0) {
                manager[i].x = space*1.5;
                manager[i].y = row*2+7;
                manager[i].goal_x = manager[i].x;
                manager[i].goal_y = -manager[i].y;;
                space++;

                float limit = (ENVIRO_SIZE / 2);
                if (manager[i].goal_x > limit) manager[i].goal_x = -limit + 1;
                if (manager[i].goal_x < -limit) manager[i].goal_x = limit - 1;
                if (manager[i].goal_y > limit) manager[i].goal_y = -limit + 1;
                if (manager[i].goal_y < -limit) manager[i].goal_y = limit - 1;
            }
            else {
                manager[i].y = space * 1.5;
                manager[i].x = row*2+7;
                manager[i].goal_y = manager[i].y;
                manager[i].goal_x = -manager[i].x;

                float limit = (ENVIRO_SIZE / 2);
                if (manager[i].goal_x > limit) manager[i].goal_x = -limit + 1;
                if (manager[i].goal_x < -limit) manager[i].goal_x = limit - 1;
                if (manager[i].goal_y > limit) manager[i].goal_y = -limit + 1;
                if (manager[i].goal_y < -limit) manager[i].goal_y = limit - 1;
            }
            if (space % 5 == 0) {
                row++;
                space = 0;
            }
            manager[i].dirX = 0; //i % 2 == 0 ? 1 : -1;
            manager[i].dirY = 0; //i % 2 == 0 ? 1 : -1;
            //manager[i].goal_x = -manager[i].x;//(float)(rand() % ENVIRO_SIZE) - (ENVIRO_SIZE / 2); //i % 2 == 0 ? 1 : -1;
            //manager[i].goal_y = -manager[i].y;//(float)(rand() % ENVIRO_SIZE) - (ENVIRO_SIZE / 2); //i % 2 == 0 ? 1 : -1;

            float limit = (ENVIRO_SIZE / 2);
            if (manager[i].x > limit) manager[i].x = -limit + 1;
            if (manager[i].x < -limit) manager[i].x = limit - 1;
            if (manager[i].y > limit) manager[i].y = -limit + 1;
            if (manager[i].y < -limit) manager[i].y = limit - 1;
        }
    }
}
//Ignore this: Not used in Sim
void mouseCordConvert(float x, float y, float* outX, float* outY) {
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY;
    GLdouble posX, posY, posZ;

    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    //POINT mouse;                        //Stores The X And Y Coords For The Current Mouse Position
   // GetCursorPos(&mouse);               // Gets The Current Cursor Coordinates (Mouse Coordinates)

    winX = (float)x;
    winY = (float)y; //(float)viewport[3] - (float)y;

    gluUnProject(winX, winY, 0, modelview, projection, viewport, &posX, &posY, &posZ);
    *outX = (float)posX;
    *outY = (float)posY;
    //emcpy(x, &posX, sizeof(float));
    // memcpy(y, &posY, sizeof(float));
}
//Ignore this: Not used in Sim
void mouseFunc(int button, int buttonState, int x, int y) {
    //TODO
    for (int i = 0; i < NUMOFAGENTS; i++) {
        if (manager[i].isEmpty) break;
        if (button == GLUT_LEFT_BUTTON && buttonState == GLUT_DOWN) {
            float outX, outY;
            mouseCordConvert(x, y, &outX, &outY);
            printf("Mouse coords: %f,%f\n", outX, outY);
            if (((outX - manager[i].x) * (outX - manager[i].x) + (-outY - manager[i].y) * (-outY - manager[i].y)) <= 10 * 10) {
                manager[i].circleDrag = true;
            }
            else {
                manager[i].circleDrag = false;
            }
        }
        else {
            manager[i].circleDrag = false;
        }
    }

}
//Ignore this: Not used in Sim
void mouseDragFunc(int x, int y) {
    //TODO Sets the current postion of the circle to the mouse's current poistion in object space

    for (int i = 0; i < NUMOFAGENTS; i++) {
        if (manager[i].isEmpty) break;
        if (manager[i].circleDrag) {
            float outX, outY;
            mouseCordConvert(x, y, &outX, &outY);
            manager[i].x = outX;
            manager[i].y = -outY;
        }
    }
}

//Draws the agent based on their postion, changes their color when at Goal
void draw_circle(float x, float  y, float z, bool atGoal) {
    GLfloat points[30];
    float angleStep = (PI / 180) * (360.0f / 10.0f);
    for (int i = 0; i < 10; i++) {
        points[3 * i] = x + ARADIS * cos((float)i * angleStep);
        points[3 * i + 1] = y + ARADIS * sin((float)i * angleStep);
        points[3 * i + 2] = 0;
    }
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, points);
    if (atGoal) glColor3f(0.0, 0.807, 0.0);
    else glColor3f(0.807, 0.0, 0.0);
    glDrawArrays(GL_POLYGON, 0, 10);
    glColor3f(0.0, 0.0, 0.0);
    glDrawArrays(GL_LINE_LOOP, 0, 10);
    glDisableClientState(GL_VERTEX_ARRAY);
}
//Displays the ID, Velocity, and position of the agent
void display_stats(int id) {
    char num[70];
    sprintf_s(num, "ID: %d Dir of X: %f Dir of Y: %f", id, manager[id].dirX, manager[id].dirY); //Var nameare
    glRasterPos2d(manager[id].x - (ARADIS + 0.5), manager[id].y + (ARADIS + 0.5));
    glColor3f(0, 1, 0);
    int len = (int)strlen(num);
    for (int i = 0; i < len; i++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, (int)num[i]);
    }
}
//Draws the obstacle at its postion once give its x and y coords
void draw_quad(Square *self,float x, float y, float z)
{
        /* 
        GLfloat points[] = {
        x + 5.0f,y + 5.0f,0,
        x + 5.0f,y - 5.0f,0,
        x - 5.0f,y - 5.0f,0,
        x - 5.0f,y + 5.0f,0
        };
        */

    GLfloat points[] = {
        x + self->wid/2,y + self->length / 2,0,
        x + self->wid / 2,y - self->length / 2,0,
        x - self->wid / 2,y - self->length / 2,0,
        x - self->wid / 2,y + self->length / 2,0
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, points);
    glColor3f(0.807, 0.0, 0.0);
    glDrawArrays(GL_QUADS, 0, 4);
    glColor3f(0.0, 0.0, 0.0);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);

    /*
    glBegin(GL_QUADS);              // Each set of 4 vertices form a quad
    glColor3f(1.0f, 0.0f, 0.0f); // Red
    glVertex2f(-0.8f, 0.1f);     // Define vertices in counter-clockwise (CCW) order
    glVertex2f(-0.2f, 0.1f);     //  so that the normal (front-face) is facing you
    glVertex2f(-0.2f, 0.7f);
    glVertex2f(-0.8f, 0.7f);
    glEnd();
    */
}
//Draws a grid to display all the cells in the sim. Called in FPS
void draw_grid() {
    for (int x = -ENVIRO_SIZE / 2; x < ENVIRO_SIZE / 2; x += CELL_SIZE) {
        glBegin(GL_LINES);
        glColor3f(0.0, 0.807, 0.2);
        glVertex2f(x, -500);
        glVertex2f(x, 500);
        glEnd();
    }
    for (int y = -ENVIRO_SIZE / 2; y < ENVIRO_SIZE / 2; y += CELL_SIZE) {
        glBegin(GL_LINES);
        glColor3f(0.2, 0.807, 0.2);
        glVertex2f(-500, y);
        glVertex2f(500, y);
        glEnd();
    }
    glColor3f(0, 0, 0);
}
// Camera for the sim. Automatically adjusted for window size
void camera(float diam, float c_x, float c_y) {
    float zNear = 0.0;
    float zFar = zNear + diam;
    float left = c_x - diam;
    float right = c_x + diam;
    float bottom = c_y - diam;
    float top = c_y + diam;
    //Aspect Ratio Adjustment
    GLsizei height = glutGet(GLUT_WINDOW_HEIGHT);
    GLsizei wid = glutGet(GLUT_WINDOW_WIDTH);
    if (height == 0) height = 1;                // To prevent divide by 0
    GLfloat aspect = (GLfloat)wid / (GLfloat)height;
    glViewport(0, 0, (GLsizei)wid, (GLsizei)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glOrtho(left, right, bottom, top, zNear, zFar);
    //glOrtho(-c_x / 2, c_x / 2, -c_y / 2, c_y / 2, zNear, zFar);
    if (wid >= height) {
        // aspect >= 1, set the height from -1 to 1, with larger width
        glOrtho(aspect * (-c_x / 2), aspect * (c_x / 2), -c_y / 2, c_y / 2, zNear, zFar);
    }
    else {
        // aspect < 1, set the width to -1 to 1, with larger height
        glOrtho(-c_x / 2, c_x / 2, (-c_y / 2) / aspect, (c_y / 2) / aspect, zNear, zFar);
    }
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(1, -1, 1);

}

/* Handler for window-repaint event. Call back when the window first appears and
   whenever the window needs to be re-painted. */
void display() {
    camera(ZFAR, ENVIRO_SIZE, ENVIRO_SIZE);
    //glViewport(0, 0, (GLsizei)1000, (GLsizei)1000); // Set our viewport to the size of our window
    glMatrixMode(GL_MODELVIEW);
    glClearColor(0.870f, 0.905f, 0.937f, 1.0f); // Set background color to back and opaque
    glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer (background)
    /*
    if(quad)draw_quad(-10.0f, -10.0f, 0); //Should be determined by an global bool intialized in the options check
    if (circle) {
        draw_circle(2, 0, 0); //Should be determined by an global bool intialized in the options check
        draw_circle(0, 0, 0); //Should be determined by an global bool intialized in the options check
    }
    if (mouse) {
        for (int i = 0; i < 50; i++) {
            if (manager[i].isEmpty) break;
            manager[i].draw(manager[i].x, manager[i].y, manager[i].z);
            //printf("Current Cords: %f,%f\n", manager[i].x, manager[i].y);
        }
    }
    */
    if (anim || crowd || favoid || shash) {
        for (int i = 0; i < NUMOFAGENTS; i++) {
            if (manager[i].isEmpty) break;
            manager[i].draw(manager[i].x, manager[i].y, manager[i].z, manager[i].atGoal);
            if (stats) display_stats(i);
        }
        for (int i = 0; i < NUMOFOBSTACLES; i++) {
            if (obstacles[i].isEmpty) break;
            obstacles[i].draw(&obstacles[i],obstacles[i].x, obstacles[i].y, obstacles[i].z);
        }
        draw_grid();
    }
    if (fps) {
        double now = time(NULL);
        double delta = now - lasttime;
        lasttime = now;
        totaltime += delta;
        //FPS calculation
        frames++;
        updateTime += delta;
        updateFrames++;
        if (updateTime > 2) {
            curFps = 1 * frames / totaltime;
            fps_avg = 1 * updateFrames / updateTime;
            updateTime = 0;
            updateFrames = 0;
        }
        char num[50];
        sprintf_s(num, "AVG FPS: %f CUR FPS: %f", curFps, fps_avg); //Var nameare
        glRasterPos2d((double)(ENVIRO_SIZE / 4), -(double)(ENVIRO_SIZE / 4));
        glColor3f(1, 0, 0);
        int len = (int)strlen(num);
        for (int i = 0; i < len; i++) {
            glutBitmapCharacter(GLUT_BITMAP_9_BY_15, (int)num[i]);
        }


    }

    glFlush();  // Render now
}
//Calculates the distance between two agents
float distance(float x1, float y1, float  x2, float y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}
//Unused fucntion: determines wether or not to agents wound collide
bool willCollide(Circle* a, Circle* b) {
    float radi = 2 * ARADIS;
    float wX = b->x - a->x;
    float wY = b->y - a->y;
    float c = (wX * wX + wY * wY) - radi * radi;
    if (c < 0) return true;
    float vX = a->dirX - b->dirX;
    float vY = a->dirY - b->dirY;
    float dotV = vX * vX + vY * vY;
    float dotWV = wX * vX + wY * vY;
    float discr = dotWV * dotWV - dotV * c;
    if (discr <= 0) {
        return false;
    }
    float tau = (dotWV - sqrtf(discr)) / dotV;
    if (tau < 0)return false;
    else return true;
}

//Takes two agents and checks collsion againest them and stores output into favoid_x and favoid_y
void collisionCheck(int i,int j,float rad) {
    //Collsion code goes here [Main issue: The time to collision (tt) is inaccurate and predicts a time that is too far!]
    float dist = distance(manager[i].x, manager[i].y, manager[j].x, manager[j].y);
    //float rad = 2 * ARADIS;
    //rad = rad * 1.05;
    //if (dist < 2 * ARADIS) {
    //    rad = 2 * ARADIS - dist;
    //}
    //bool coll = willCollide(&manager[i], &manager[j]);
    float wx = manager[j].x - manager[i].x;
    float wy = manager[j].y - manager[i].y;
    float vx = manager[i].dirX - manager[j].dirX;
    float vy = manager[i].dirY - manager[j].dirY;
    //if(vx == 0 && wx == 0) vx += 5.1;
    //if(vy == 0 && vy == 0) vy += 5.1;
    float a = vx * vx + vy * vy;
    float b = wx * vx + wy * vy;
    float c = (wx * wx + wy * wy) - rad * rad;
    float discr = b * b - a * c;
    if (discr > 0 && (a < -0.00001f || a > 0.00001f)) {
        discr = sqrtf(discr);
        float tt = (b - discr) / a;
        //tt /= 90000.0f;
        if (tt > 0) {
            float part1 = -k * exp(-tt / t0);
            float part2 = (a * powf(tt, m));
            float part3 = (m / tt + 1 / t0);
            float part4 = (vx - (b * vx - a * wx) / discr);
            fAvoid_x += (-k * exp(-tt / t0) / (a * powf(tt, m))) * (m / tt + 1 / t0) * (vx - (b * vx - a * wx) / discr);
            fAvoid_y += (-k * exp(-tt / t0) / (a * powf(tt, m))) * (m / tt + 1 / t0) * (vy - (b * vy - a * wy) / discr);
        }
    }
}
//GOAL: Implement social avoidence force model
//This implmentation will ingore the friction force and the wall repulsion force
//The big equation you see in the powerpoint (V(t)e(t)-V(t)/t) is just the force goal, so all we are 
//focusing on are the inter-agent forces (ignore kg(r-d)v't section of the equation, deals with friction)
//Equation becomes: f = {Aexp(r-d/B)+Kg(r-d)}n
//Checks if agent will collide with other agents and obstacles
void step(int i) {
    float goalDist = distance(manager[i].x, manager[i].y, manager[i].goal_x, manager[i].goal_y);
    if (goalDist < 0.5) {
        if (!manager[i].atGoal) manager[i].atGoal = true;
        //return;
        manager[i].dirX = 0;
        manager[i].dirY = 0;
        manager[i].goal_x = manager[i].x;
        manager[i].goal_y = manager[i].y;
    }
    const float d_h = 10;// 10 * 2 * ARADIS;
    double v_x = manager[i].dirX;
    double v_y = manager[i].dirY;
    const float zeta = 0.54;//1.0023;
    const float max_force = 20; //1.5
    const float max_speed = 2;
    float prefSpeed = 1.5;
    const float timeStep = TIME_STEP / 10;
    //Making sure nothing is zero
    int caster = manager[i].goal_x;
    float rng = (((float)rand() / (RAND_MAX)) - 0.5) / 1000000;
    if (caster == 0) manager[i].goal_x += rng;
    caster = manager[i].goal_y;
    rng = (((float)rand() / (RAND_MAX)) - 0.5) / 1000000;
    if (caster == 0) manager[i].goal_y += rng;
    //if (abs(manager[i].goal_x - 0.0) <= 0.0005) manager[i].goal_x += (rand() % 2 - 1) / 10000;
    //if (abs(manager[i].goal_y - 0.0) <= 0.0005) manager[i].goal_y += (rand() % 2 - 1) / 10000;
    float prefVeloX = manager[i].goal_x - manager[i].x;
    float prefVeloY = manager[i].goal_y - manager[i].y;
    if (prefVeloX != 0 || prefVeloY != 0) {
        if (prefSpeed > max_speed) prefSpeed = max_speed;
        prefVeloX = prefSpeed * (prefVeloX / (sqrtf(prefVeloX * prefVeloX + prefVeloY * prefVeloY)));
        prefVeloY = prefSpeed * (prefVeloY / (sqrtf(prefVeloX * prefVeloX + prefVeloY * prefVeloY)));
    }
    
    float f_goal_x = (prefVeloX - v_x) / zeta;//0.9 * prefVeloX+ 0.1 * v_x; //(prefVeloX - v_x) / zeta; 0.2 * prefVeloX + 0.8 * v_x;
    float f_goal_y = (prefVeloY - v_y) / zeta;//0.9* prefVeloY + 0.1 * v_y;//(prefVeloY - v_y) / zeta; 0.2 * prefVeloY + 0.8 * v_y;

    float A = 2000;
    float B = 0.08;//0.08
    //float k = 1.5;
    //float k_frict = 0;//100;
    //float t0 = 3.0000000; //2 - 4
    //float m = 2;
    //double fAvoid_x = 0;
    //double fAvoid_y = 0;
    fAvoid_x = 0;
    fAvoid_y = 0;
    float fAvoidCtr = 0;

    for (int j = 0; j < NUMOFAGENTS; j++) {
        compteCount += 1;
        if (i == j) continue;
        float dist = distance(manager[i].x, manager[i].y, manager[j].x, manager[j].y);
        if (dist > 0 && dist < d_h) {
            
            //Collsion code goes here [Main issue: The time to collision (tt) is inaccurate and predicts a time that is too far!]
            
            float rad = 2 * ARADIS;
            //rad = rad * 1.05;
            if (dist < 2 * ARADIS) {
                rad = 2 * ARADIS - dist;
            }
            collisionCheck(i, j,rad);
            /*
            //bool coll = willCollide(&manager[i], &manager[j]);
            float wx = manager[j].x - manager[i].x;
            float wy = manager[j].y - manager[i].y;
            float vx = manager[i].dirX - manager[j].dirX;
            float vy = manager[i].dirY - manager[j].dirY;
            //if(vx == 0 && wx == 0) vx += 5.1;
            //if(vy == 0 && vy == 0) vy += 5.1;
            float a = vx * vx + vy * vy;
            float b = wx * vx + wy * vy;
            float c = (wx * wx + wy * wy) - rad * rad;
            float discr = b * b - a * c;
            if (discr > 0 && (a < -0.00001f || a > 0.00001f)) {
                discr = sqrtf(discr);
                float tt = (b - discr) / a;
                //tt /= 90000.0f;
                if (tt > 0) {
                    float part1 = -k * exp(-tt / t0);
                    float part2 = (a * powf(tt, m));
                    float part3 = (m / tt + 1 / t0);
                    float part4 = (vx - (b * vx - a * wx) / discr);
                    fAvoid_x += (-k * exp(-tt / t0) / (a * powf(tt, m))) * (m / tt + 1 / t0) * (vx - (b * vx - a * wx) / discr);
                    fAvoid_y += (-k * exp(-tt / t0) / (a * powf(tt, m))) * (m / tt + 1 / t0) * (vy - (b * vy - a * wy) / discr);
                }
            }
            */
            //Collsion code stops here
            fAvoidCtr += 1;
        }
    }
    //Controls collision dection with osbstacles
    for (int j = 0; j < NUMOFOBSTACLES; j++) {
        if (obstacles[i].isEmpty) break;
        Square* curSquare = &obstacles[i];
        float dist = distance(curSquare->x, curSquare->y, manager[i].x, manager[i].y);
        float rad = 2*ARADIS;
        
        float diffx = manager[i].x - curSquare->x;
        float diffy = manager[i].y - curSquare->y;
        float wid = curSquare->wid/ 2;
        float len = curSquare->length/ 2;
        //THIS AREA THAT ISNT WORKING
        //Clamps the diff values to the bounds of the rectangle
        if (diffx < -wid || diffx > wid){
            diffx = max(-wid, min(diffx,  wid));
        }
        if (diffy < -len || diffy > len) {
            diffy = max(-len, min(diffy, len));
        }
        //Stores the closest point to the circle
        float closex = curSquare->x + diffx;
        float closey = curSquare->y + diffy;
        //rad += sqrtf(diffx * diffx + diffy * diffy);
        //Gets the difference between the agent's center and the point
        diffx = closex - manager[i].x;
        diffy = closey - manager[i].y;
        float closeDist = sqrtf(diffx * diffx + diffy * diffy);
        if(closeDist > 0 && closeDist < d_h) {
            if (sqrtf(diffx * diffx + diffy * diffy) < 2*ARADIS) {
                rad = 2*ARADIS - sqrtf(diffx * diffx + diffy * diffy);
                
            }
            collisionCheck(i, j,rad);
            /*
            float ovx = v_x;//v_x + v_x;
            float ovy = v_y;//v_y + v_y;
            const float a = ovx*ovx+ovy*ovy;
            float owx = closex - manager[i].x;
            float owy = closey - manager[i].y;
            float b = owx*ovx+owy*ovy;
            float c = (owx * owx + owy * owy) - (rad * rad);
            float discr = b*b-a*c;
            if (discr > 0 && (a < -0.00001f || a > 0.00001f)) {
                discr = sqrtf(discr);
                float tt = (b - discr) / a;
                //tt /= 90000.0f;
                if (tt > 0) {
                    float part1 = -k * exp(-tt / t0);
                    float part2 = (a * powf(tt, m));
                    float part3 = (m / tt + 1 / t0);
                    float part4 = (ovx - (b * ovx - a * owx) / discr);
                    fAvoid_x += (-k * exp(-tt / t0) *(ovx-(b*ovx-a*owx)/discr)/(a*powf(tt,m))*(m/tt + 1/t0));
                    fAvoid_y += (-k * exp(-tt / t0) *(ovy-(b*ovy-a*owy)/discr)/(a*powf(tt,m))*(m/tt + 1/t0));
                }
            }
            */
            //Collsion code stops here
            fAvoidCtr += 1;
        }
    }
    if (fAvoidCtr > 0) {
        fAvoid_x = fAvoid_x / fAvoidCtr;
        fAvoid_y = fAvoid_y / fAvoidCtr;
    }
    double forceSum_x = f_goal_x + fAvoid_x;
    double forceSum_y = f_goal_y + fAvoid_y;

    double fAvoid_Mag = sqrt(forceSum_x * forceSum_x + forceSum_y * forceSum_y);


    if (fAvoid_Mag > max_force) {
        forceSum_x = max_force * forceSum_x / fAvoid_Mag;
        forceSum_y = max_force * forceSum_y / fAvoid_Mag;
    }

    v_x += timeStep * forceSum_x;
    v_y += timeStep * forceSum_y;
    float speed = sqrtf(v_x * v_x + v_y * v_y);
    if (speed > max_speed) {
        v_x = max_speed * v_x / speed;
        v_y = max_speed * v_y / speed;
    }
    manager[i].dirX = v_x;
    manager[i].dirY = v_y;
    manager[i].x += timeStep * v_x;
    manager[i].y += timeStep * v_y;

}
//Chnage d_h to 3*2*ARADIS
//B to 0.88 (B determines how far the agents will push apart from eachother)
void hashStep(int i) {
    float goalDist = distance(manager[i].x, manager[i].y, manager[i].goal_x, manager[i].goal_y);
    if (goalDist < 0.5) {
        if (!manager[i].atGoal) manager[i].atGoal = true;
        //return;
        manager[i].dirX = 0;
        manager[i].dirY = 0;
        //manager[i].goal_x = manager[i].x;
        //manager[i].goal_y = manager[i].y;
    }
    const float d_h = 10;// *2 * ARADIS;
    double v_x = manager[i].dirX;
    double v_y = manager[i].dirY;
    const float zeta = 0.54;// 1.0023;
    const float max_force = 20; // 1.55 //0.15
    const float max_speed = 2;
    const float prefSpeed = 1.5;
    const float timeStep = TIME_STEP/10;
    float prefVeloX = manager[i].goal_x - manager[i].x;
    float prefVeloY = manager[i].goal_y - manager[i].y;
    if (prefVeloX != 0 && prefVeloY != 0) {
        prefVeloX = prefSpeed * (prefVeloX / (sqrtf(prefVeloX * prefVeloX + prefVeloY * prefVeloY)));
        prefVeloY = prefSpeed * (prefVeloY / (sqrtf(prefVeloX * prefVeloX + prefVeloY * prefVeloY)));
    }
    //Add coords for goal, and replace dir_goal with a vector to the goal
    //Add a weight on how much goal and current veclotiy combine (10% goal 90% current for example)
    float f_goal_x = (prefVeloX - v_x) / zeta;
    float f_goal_y = (prefVeloY - v_y) / zeta;
    // if (manager[i].dir_goal_x > v_x) printf("ALERT!\n");
    //float k = 1.5;
    //float k_frict = 0;//100;
    //float t0 = 3.0000000; //2 - 4
    //float m = 2;
    //double fAvoid_x = 0;
    //double fAvoid_y = 0;
    fAvoid_x = 0;
    fAvoid_y = 0;
    float fAvoidCtr = 0;
    float interacting_agents = 0;
    //Getting lists of current and neighboring cells from hash
    //int X = (int)(manager[i].x / CELL_SIZE) * CELL_SIZE;
    //int Y = (int)(manager[i].y / CELL_SIZE) * CELL_SIZE;
    //int test = hashFun(manager[i].x, manager[i].x);
    //int test2 = hashFun(X, Y);
    int* list;
    int hash;
    int lookSize = 2;//d_h / CELL_SIZE;
    for (int x = -lookSize; x <= lookSize; x++) { //2
        for (int y = -lookSize; y <= lookSize; y++) { //2
            //if (x == 0 && y == 0)continue;
            int adjx = manager[i].x + x * CELL_SIZE;
            int adjy = manager[i].y + y * CELL_SIZE;
            if (adjx >= ENVIRO_SIZE/2 || adjx <= -ENVIRO_SIZE/2) continue;
            if (adjy >= ENVIRO_SIZE/2 || adjy <= -ENVIRO_SIZE/2) continue;
            // key.clear();
             //key += to_string(adjx);
            // key += intToStr.at(adjx);///[adjx];
            // key += ",";
             //key += intToStr.at(adjy);//[adjy];
             //key += to_string(adjy);
            // if (spatialHash.count(key) < 1) continue;
            try {
                hash = hashFun(adjx, adjy);
                list = spatialHash[hashFun(adjx, adjy)];//&spatialHash[key];
            }
            catch (...) {
                continue;
            }
            //Running through circles in the current hash
            for (int j = 0; j < BucketSize; j++) {
                compteCount += 1;
                if (list[j] == -1) break;
                Circle* agent = &manager[list[j]];//(*list)[j];
                if (manager[i].id == agent->id) continue; //manager[i].x == c->x && manager[i].y == c->y
                float dist = distance(manager[i].x, manager[i].y, agent->x, agent->y);
                if (dist > 0 && dist < d_h) {
                    //Collsion code goes here [Main issue: The time to collision (tt) is inaccurate and predicts a time that is too far!]
                    float rad = 2 * ARADIS;
                    //rad = rad * 1.05;
                    if (dist < 2 * ARADIS) {
                        rad = rad - dist;
                    }
                    collisionCheck(i, agent->id, rad);
                    /*
                    //bool coll = willCollide(&manager[i], &manager[j]);
                    float wx = agent->x - manager[i].x;
                    float wy = agent->y - manager[i].y;
                    float vx = manager[i].dirX - agent->dirX;
                    float vy = manager[i].dirY - agent->dirY;
                    //if(vx == 0 && wx == 0) vx += 5.1;
                    //if(vy == 0 && vy == 0) vy += 5.1;
                    float a = vx * vx + vy * vy;
                    float b = wx * vx + wy * vy;
                    float c = (wx * wx + wy * wy) - rad * rad;
                    float discr = b * b - a * c;
                    if (discr > 0 && (a < -0.00001f || a > 0.00001f)) {
                        discr = sqrtf(discr);
                        float tt = (b - discr) / a;
                        //tt /= 90000.0f;
                        if (tt > 0) {
                            float part1 = -k * exp(-tt / t0);
                            float part2 = (a * powf(tt, m));
                            float part3 = (m / tt + 1 / t0);
                            float part4 = (vx - (b * vx - a * wx) / discr);
                            fAvoid_x += (-k * exp(-tt / t0) / (a * powf(tt, m))) * (m / tt + 1 / t0) * (vx - (b * vx - a * wx) / discr);
                            fAvoid_y += (-k * exp(-tt / t0) / (a * powf(tt, m))) * (m / tt + 1 / t0) * (vy - (b * vy - a * wy) / discr);
                        }
                    }
                    */
                    fAvoidCtr += 1;
                }
            }
        }
    }/*
    if (fAvoidCtr > 0) {
        fAvoid_x = fAvoid_x / fAvoidCtr;
        fAvoid_y = fAvoid_y / fAvoidCtr;
    }
    */
    double forceSum_x = f_goal_x + fAvoid_x;
    double forceSum_y = f_goal_y + fAvoid_y;
    //if (forceSum_x >= INFINITY) forceSum_x = max_force;
    //if (forceSum_x <= -INFINITY) forceSum_x = -max_force;
    //if (forceSum_y >= INFINITY) forceSum_y = max_force;
    //if (forceSum_y <= -INFINITY) forceSum_y = -max_force;
    double fAvoid_Mag = sqrt(forceSum_x * forceSum_x + forceSum_y * forceSum_y);
    //if (fAvoid_Mag == INFINITY) fAvoid_Mag = FLT_MAX;
    //if (fAvoid_Mag == -INFINITY) fAvoid_Mag = -FLT_MAX;

    if (fAvoid_Mag > max_force) {
        forceSum_x = max_force * forceSum_x / fAvoid_Mag;
        forceSum_y = max_force * forceSum_y / fAvoid_Mag;
    }

    v_x += timeStep * forceSum_x;
    v_y += timeStep * forceSum_y;

    //if (v_x >= INFINITY) v_x = max_speed;
    //if (v_x <= -INFINITY) v_x = -max_speed;
    //if (v_y >= INFINITY) v_y = max_speed;
    //if (v_y <= -INFINITY) v_y = -max_speed;

    float speed = sqrtf(v_x * v_x + v_y * v_y);
    if (speed > max_speed) {
        v_x = max_speed * v_x / speed;
        v_y = max_speed * v_y / speed;
    }


    //float oldX = manager[i].x;
    //float oldY = manager[i].y;

    manager[i].dirX = v_x;
    manager[i].dirY = v_y;
    manager[i].x += timeStep * v_x;
    manager[i].y += timeStep * v_y;
    //Updates circles location in the hash [Double check that its working properly!]
    //X = (int)(manager[i].x / CELL_SIZE) * CELL_SIZE;
   // Y = (int)(manager[i].y / CELL_SIZE) * CELL_SIZE;


}
//Writes down the positions of all the agents onto a text file for use in the unity side of the sim
void storLocs() {
    ofstream frame;
    string strFrames = to_string((int)frames);
    frame.open("../frames/frame" + strFrames + ".txt");
    for (int i = 0; i < NUMOFAGENTS; i++) {
        string strX = to_string(manager[i].x);
        string strY = to_string(manager[i].y);
        frame << strX << "|" << strY << "\n";
    }
    frame.close();

}

void update(int value) {
    //TODO animation
    if (anim || crowd || favoid) {
        for (int i = 0; i < NUMOFAGENTS; i++) {
            if (manager[i].isEmpty) break;
            //float mag = sqrt(manager[i].dirX* manager[i].dirX+ manager[i].dirY * manager[i].dirY);
            //float stepX = (manager[i].dirX / mag) * 0.05;
            //float stepY = (manager[i].dirY / mag) * 0.05;
            //manager[i].x += stepX;
            //manager[i].y += stepY;
            if (favoid) {
                manager[i].stepF(i);
                float limit = (ENVIRO_SIZE / 2);
                if (manager[i].x > limit) manager[i].x = -limit + 1;
                if (manager[i].x < -limit) manager[i].x = limit - 1;
                if (manager[i].y > limit) manager[i].y = -limit + 1;
                if (manager[i].y < -limit) manager[i].y = limit - 1;
            }

            if (anim) {
                if (manager[i].x > 20.5) manager[i].x = -20.4;
                if (manager[i].x < -20.5) manager[i].x = 20.4;
                if (manager[i].y > 20.5) manager[i].y = -20.4;
                if (manager[i].y < -20.5) manager[i].y = 20.4;
            }
            if (crowd) {
                if (manager[i].x > 30.5) manager[i].x = -30.4;
                if (manager[i].x < -30.5) manager[i].x = 30.4;
                if (manager[i].y > 30.5) manager[i].y = -30.4;
                if (manager[i].y < -30.5) manager[i].y = 30.4;
            }
        }
        //storLocs();
    }

    //hash system here
    //setup loop based on cell amount and cell size, go through each cell A) see what agents are there and B) check what collsions exist 
    //TODO: Create add agent function
    if (shash) {
        //Create hashes for every agent in the sim
        for (int i = 0; i < NUMOFAGENTS; i++) {
            if (manager[i].isEmpty) break;
            int Index = hashFun(manager[i].x, manager[i].y);
            for (int n = 0; n < BucketSize; n++) {
                if (spatialHash[Index][n] == -1) {
                    spatialHash[Index][n] = manager[i].id;
                    break;
                }
            }
            int adjCount = 0;
            int prev[4] = { -1,-1,-1,-1 };
            bool used = false;
            for (int j = -1; j <= 1; j++) {
                for (int k = -1; k <= 1; k++) {
                    if (j == 0 || k == 0) continue;
                    used = false;
                    Index = hashFun(manager[i].x + j * ARADIS, manager[i].y + k * ARADIS);
                    for (int n = 0; n < 4; n++)
                    {
                        if (prev[n] == -1) break;
                        if (prev[n] == Index) {
                            adjCount += 1;
                            used = true;
                            break;
                        }
                    }
                    if (used) continue;
                    for (int n = 0; n < BucketSize; n++) {
                        //NEW
                        if (spatialHash[Index][n] == manager[i].id) {
                            break;
                        }
                        //NEW
                        if (spatialHash[Index][n] == -1) {
                            spatialHash[Index][n] = manager[i].id;
                            prev[adjCount] = Index;
                            break;
                        }
                    }
                    adjCount += 1;

                }
            }
        }
        //Make sure agents are in bounds and update agents
        for (int i = 0; i < NUMOFAGENTS; i++) {
            manager[i].stepF(i);
            float limit = (ENVIRO_SIZE / 2);
            if (manager[i].x > limit) manager[i].x = -limit + 1;
            if (manager[i].x < -limit) manager[i].x = limit - 1;
            if (manager[i].y > limit) manager[i].y = -limit + 1;
            if (manager[i].y < -limit) manager[i].y = limit - 1;
        }
        //storLocs();
        //ClEAR ALL VALUES IN THE HASH!
        memset(spatialHash, -1, sizeof(int) * width * width * BucketSize);
    }
    glutPostRedisplay(); // Inform GLUT that the display has changed
    glutTimerFunc(25, update, 0);//Call update after each 25 millisecond
}

/* Main function: GLUT runs as a console application starting at main()  */
int main(int argc, char** argv) {
    //Add check for options here
    for (int i = 0; i < argc; i++) {
        /*
        if (strcmp("-quad", argv[i]) == 0) {
            quad = true;
        }
        if (strcmp("-circle", argv[i]) == 0) {
            circle = true;
        }
        if (strcmp("-mouse", argv[i]) == 0) {
            mouse = true;
        }
        */
        if (strcmp("-anim", argv[i]) == 0) {
            anim = true;
        }
        if (strcmp("-crowd", argv[i]) == 0) {
            crowd = true;
        }
        if (strcmp("-favoid", argv[i]) == 0) {
            favoid = true;
        }
        if (strcmp("-fps", argv[i]) == 0) {
            fps = true;
        }
        if (strcmp("-shash", argv[i]) == 0) {
            shash = true;
            fps = true;
            //stats = true;
        }
        if (strcmp("-stats", argv[i]) == 0) {
            stats = true;
        }
        
        if (strcmp("-agent_circle", argv[i]) == 0) {
            basic = false;
            agent_circle = true;
        }
        else if (strcmp("-con_circle", argv[i]) == 0) {
            basic = false;
            con_circle = true;
        }
        else if (strcmp("-agent_swap", argv[i]) == 0) {
            basic = false;
            agent_swap = true;
        }
        else if (strcmp("-wallTest", argv[i]) == 0) {
            basic = false;
            wallTest = true;
        }
        else if (strcmp("-crossFlow", argv[i]) == 0) {
            basic = false;
            crossFlow = true;
        }
    }
    if (!shash && !fps && !favoid && !anim && !crowd) printf("No args found!\n");
    //Option check ends
    glutInit(&argc, argv);                 // Initialize GLUT
    glutCreateWindow("OpenGL Setup Test"); // Create a window with the given title
    initGL(); //initializes circles for mouse, animation, and crowd
    glutMouseFunc(mouseFunc);       // called for mousedown and mouseup events
    glutMotionFunc(mouseDragFunc);  // called when mouse is dragged
    glutInitWindowSize(100, 100);   // Set the window's initial width & height
    //glutReshapeWindow(720, 1024);
    glutInitWindowPosition(0, 0); // Position the window's initial top-left corner
    glutDisplayFunc(display); // Register display callback handler for window re-paint
    glutTimerFunc(25, update, 0);
    glutMainLoop();           // Enter the event-processing loop
    return 0;
}