/*
Dono note: Dont worry about mouse vs world coords, your not interacting with anything for this assignment, so the math doesnt deal
at all with transformations, so dont worry.
I tested this version on my mac
I compile with:
cc -o opengl_mock openglglut.cpp -I/opt/local/include -lglfw -framework Cocoa -framework OpenGL -framework GLUT -framework Cocoa
*/

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

#define ZFAR 1.0 //4.0
#define PI 3.14159265f
#define ARADIS 10
#define TIME_STEP 0.5 // was 0.5
#define NUMOFAGENTS 16 //100 //Was 300
//bool circle = false;
//bool quad = false;
//bool mouse = false;
//time_t timer;
int loopCount = 1;
int compteCount = 0; // dw add

//FPS
float curTime = 0;
double lasttime = 0;
float totaltime = 0;
float frames = 0;
float curFps = 0;
float fps_avg = 0;
float updateTime = 0;
float updateFrames=0;
//ARGS
bool anim = false;
bool crowd = false;
bool shash = false;
bool favoid = false;
bool fps = false;

void draw_circle(float x, float  y, float z);
void step(int i);
void hashStep(int i);

typedef pair<int, int> coord; //For hashmap
typedef struct Circle {
    int id; //Used for spatial hash
    coord cell = {-555, -555 }; //Used for Spatial Hash //was oringaly string
    coord adjcells[4] = { {-555, -555} ,{-555, -555} ,{-555, -555} ,{-555, -555} }; //Used for Spatial Hash //Was oringally string[4]
    GLfloat x;
    GLfloat y;
    GLfloat z = 0;
    GLfloat dirX;
    GLfloat dirY;
    GLfloat dir_goal_x;
    GLfloat dir_goal_y;
    bool circleDrag = false; 
    bool isEmpty = true;
    void (*draw)(float x, float y, float z) { draw_circle };
    void (*stepF)(int i) { step };
};
Circle manager[NUMOFAGENTS] = { 0 };
// SPATIAL HASH 

struct HASH { //Needs to fix
    size_t operator()(const coord x)const {
        return hash<long long>()(((long long)x.first) ^ (((long long)x.second) << 32));
    }
};
unordered_map<coord, std::vector<Circle *>,HASH> spatialHash; //int[] was orginally a string
unordered_map<int, string> intToStr;
#define CELL_SIZE 50
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
            manager[i].x = (float)(rand()%3) - 1.5;
            manager[i].y = (float)(rand()%3) - 1.5;
            manager[i].dirX = (float)(rand()%3) - 1.5;
            manager[i].dirY = (float)(rand()%3) - 1.5;
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
    if (favoid) {
        for (int i = 0; i < NUMOFAGENTS; i++) {
            manager[i].x = (float)(rand() % 1000) - 500;
            manager[i].y = (float)(rand() % 1000) - 500;
            manager[i].dirX = (float)(rand() % 10) - 5;
            manager[i].dirY = (float)(rand() % 10) - 5;
            manager[i].dir_goal_x = (float)(rand() % 10) - 5;
            manager[i].dir_goal_y = (float)(rand() % 10) - 5;
            manager[i].isEmpty = false;
            manager[i].id = i;
        }
    }
    if (fps) {
        lasttime = time(NULL);
    }
    if (shash) {
        
        for(int x = -500; x <= 500; x+=CELL_SIZE) {
            for (int y = -500; y <= 500; y += CELL_SIZE) {
                spatialHash[{x, y}].reserve(10);
            }
        }
        
        for (int x = -500; x <= 500; x += CELL_SIZE) {
            intToStr[x] = to_string(x);
        }
        if (!favoid) {
            for (int i = 0; i < NUMOFAGENTS; i++) {
                manager[i].x =  (float)(rand() % 1000) - 500; //i % 2 == 0 ? -500 + i * 20 : -500 + i * 20;
                manager[i].y = (float)(rand() % 1000) - 500; //i % 2 == 0 ? -400:400; 
                manager[i].dirX = (float)(rand() % 10) - 5; //i % 2 == 0 ? 1 : -1;
                manager[i].dirY = (float)(rand() % 10) - 5; //i % 2 == 0 ? 1 : -1;
                manager[i].dir_goal_x = (float)(rand() % 10) - 5; //i % 2 == 0 ? 1 : -1;
                manager[i].dir_goal_y = (float)(rand() % 10) - 5; //i % 2 == 0 ? 1 : -1;
                if (abs(manager[i].dir_goal_x) < 1) (float)(rand() % 2) - 1 <= 0 ? manager[i].dir_goal_x = -1 : manager[i].dir_goal_x = 1;
                if (abs(manager[i].dir_goal_y) < 1) (float)(rand() % 2) - 1 <= 0 ? manager[i].dir_goal_y = -1 : manager[i].dir_goal_y = 1;
                manager[i].stepF = hashStep;
                manager[i].id = i;
               // manager[i].cell = {NULL,NULL};
                //manager[i].adjcells[0] = { NULL,NULL };
               // manager[i].adjcells[1] = { NULL,NULL };
               // manager[i].adjcells[2] = { NULL,NULL };
                //manager[i].adjcells[3] = { NULL,NULL };
                manager[i].isEmpty = false;
            }
        }
        else if(favoid) {
            for (int i = 0; i < NUMOFAGENTS; i++) {
                manager[i].stepF = hashStep;
                manager[i].id = i;
               // manager[i].cell = { NULL,NULL };
               // manager[i].adjcells[0] = { NULL,NULL };
               // manager[i].adjcells[1] = { NULL,NULL };
                //manager[i].adjcells[2] = { NULL,NULL };
               // manager[i].adjcells[3] = { NULL,NULL };
            }
        }
        for (int i = 0; i < NUMOFAGENTS; i++) {
            if (manager[i].isEmpty) break;
            int X = (int)(manager[i].x / CELL_SIZE) * CELL_SIZE;
            int Y = (int)(manager[i].y / CELL_SIZE) * CELL_SIZE;

            //int minX = (int)((manager[i].x - ARADIS) / CELL_SIZE) * CELL_SIZE;
            //int minY = (int)((manager[i].y - ARADIS) / CELL_SIZE) * CELL_SIZE;
            //int maxX = (int)((manager[i].x + ARADIS) / CELL_SIZE) * CELL_SIZE;
            //int maxY = (int)((manager[i].y + ARADIS) / CELL_SIZE) * CELL_SIZE;
            //string key = "";
            //key += to_string(X);
            //key += ",";
            //key += to_string(Y);
            manager[i].cell.first = X; //key;
            manager[i].cell.second = Y;
            //spatialHash[key].push_back(manager[i]);
            int adjCount = 0;
            //string prev[4];
            int prev[4][2];
            bool used = false;
            for (int j = -1; j <= 1; j++) {
                for (int k = -1; k <= 1; k++) {
                    if (j == 0 || k == 0) continue;
                    used = false;
                    int adjX = (int)((manager[i].x + j*ARADIS) / CELL_SIZE) * CELL_SIZE;
                    int adjY = (int)((manager[i].y + k*ARADIS) / CELL_SIZE) * CELL_SIZE;
                    if (adjX == X && adjY == Y) {
                        adjCount += 1;
                        continue;
                    }
                    //key = "";
                    //key += to_string(adjX);
                    //key += ",";
                    //key += to_string(adjY);
                    /*
                    for (string s : prev) {
                        if(s.compare(key) == 0) used = true;
                    }
                    */
                    for (int s = 0; s < 4;s++){
                        if (prev[s][0] == -555 && prev[s][1] == -555) continue;
                        if (prev[s][0] == adjX && prev[s][1] == adjY) used = true;
                    }
                    if (used) {
                        adjCount += 1;
                        continue;
                    }
                    manager[i].adjcells[adjCount].first = adjX;
                    manager[i].adjcells[adjCount].second = adjY;
                    prev[adjCount][0] = adjX;
                    prev[adjCount][1] = adjY;
                    //spatialHash[key].push_back(manager[i]);
                    adjCount += 1;
                }
            }
            spatialHash[manager[i].cell].push_back(&manager[i]);
            if(manager[i].adjcells[0].first != -555 || manager[i].adjcells[0].second != -555) spatialHash[manager[i].adjcells[0]].push_back(&manager[i]);
            if (manager[i].adjcells[1].first != -555 || manager[i].adjcells[1].second != -555) spatialHash[manager[i].adjcells[1]].push_back(&manager[i]);
            if (manager[i].adjcells[2].first != -555 || manager[i].adjcells[2].second != -555) spatialHash[manager[i].adjcells[2]].push_back(&manager[i]);
            if (manager[i].adjcells[3].first != -555 || manager[i].adjcells[3].second != -555) spatialHash[manager[i].adjcells[3]].push_back(&manager[i]);
            //if(manager[i].adjcells[0].compare("") !=0) spatialHash[manager[i].adjcells[0]].push_back(&manager[i]);
            //if (manager[i].adjcells[1].compare("") != 0) spatialHash[manager[i].adjcells[1]].push_back(&manager[i]);
            //if (manager[i].adjcells[2].compare("") != 0) spatialHash[manager[i].adjcells[2]].push_back(&manager[i]);
            //if (manager[i].adjcells[3].compare("") != 0) spatialHash[manager[i].adjcells[3]].push_back(&manager[i]);
            /*
            if (minX != X && minY != Y) {
                key = "";
                key += to_string(minX);
                key += ",";
                key += to_string(minY);
                manager[i].adjcells[3] = key;
                spatialHash[key].push_back(manager[i]);
            }
            if (maxX != X && maxY != Y) {
                key = "";
                key += to_string(maxX);
                key += ",";
                key += to_string(maxY);
                manager[i].adjcells[1] = key;
                spatialHash[key].push_back(manager[i]);
            }
            if (maxX != X && minY != Y) {
                key = "";
                key += to_string(maxX);
                key += ",";
                key += to_string(minY);
                manager[i].adjcells[2] = key;
                spatialHash[key].push_back(manager[i]);
            }
            if (minX != X && maxY != Y) {
                key = "";
                key += to_string(maxX);
                key += ",";
                key += to_string(maxY);
                manager[i].adjcells[0] = key;
                spatialHash[key].push_back(manager[i]);
            }
            */
    }
        
        /*
        for (int i = 0; i < (1000 * 1000) / CELL_SIZE; i++) {
            //spatialHash[i].push_back(manager[i]);
            
        }
        */
    }

}
void mouseCordConvert(float x, float y, float* outX, float* outY) {
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY;
    GLdouble posX, posY,posZ;

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
void mouseFunc(int button, int buttonState, int x, int y) {
    //TODO
        for (int i = 0; i < NUMOFAGENTS; i++) {
            if (manager[i].isEmpty) break;
            if (button == GLUT_LEFT_BUTTON && buttonState == GLUT_DOWN) {
                float outX, outY;
                mouseCordConvert(x, y,&outX,&outY);
				printf("Mouse coords: %f,%f\n", outX, outY);
                if (((outX - manager[i].x) * (outX - manager[i].x) + (-outY - manager[i].y) * (-outY - manager[i].y)) <= 10 * 10) {
                    manager[i].circleDrag = true;
                }
                else{
                    manager[i].circleDrag = false;
                }
            }
            else{
                manager[i].circleDrag = false;
                }
            }
        
}

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


void draw_circle(float x, float  y, float z) {
    GLfloat points[30];
    float angleStep = (PI/180) * (360.0f / 10.0f);
    for (int i = 0; i < 10; i++) {
        points[3*i] = x+ARADIS*cos((float)i*angleStep);
        points[3*i+1] =y+ARADIS*sin((float)i*angleStep);
        points[3*i+2] = 0;
    }
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, points);
    glColor3f(0.807, 0.0, 0.0);
    glDrawArrays(GL_POLYGON, 0,10);
    glColor3f(0.0, 0.0, 0.0);
    glDrawArrays(GL_LINE_LOOP, 0, 10);
    glDisableClientState(GL_VERTEX_ARRAY);
}
void draw_quad(float x, float y, float z)
{
   
    GLfloat points[] = {
        x+10.0f,y +5.0f,0,
        x +5.0f,y + 5.0f,0,
        x + 5.0f,y + 10.0f,0,
        x +10.0f,y + 10.0f,0
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
void draw_grid() {
    for (int x = -500; x < 500; x += CELL_SIZE) {
        glBegin(GL_LINES);
        glColor3f(0.0, 0.807, 0.2);
        glVertex2f(x, -500);
        glVertex2f(x, 500);
        glEnd();
    }
    for (int y = -500; y < 500; y += CELL_SIZE) {
        glBegin(GL_LINES);
        glColor3f(0.2, 0.807, 0.2);
        glVertex2f(-500,y);
        glVertex2f(500, y);
        glEnd();
    }
    glColor3f(0, 0, 0);
}
// check https://www.khronos.org/opengl/wiki/Viewing_and_Transformations#How_do_I_implement_a_zoom_operation.3F
void camera(float diam, float c_x, float c_y) {
    float zNear = 0.0;
    float zFar = zNear + diam;
    float left = c_x - diam;
    float right = c_x + diam;
    float bottom = c_y - diam;
    float top = c_y + diam;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glOrtho(left, right, bottom, top, zNear, zFar);
	glOrtho(-c_x/2,c_x/2 ,-c_y/2, c_y/2, zNear, zFar);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(1, -1, 1);

}

/* Handler for window-repaint event. Call back when the window first appears and
   whenever the window needs to be re-painted. */
void display() {
    camera(ZFAR, 1000, 1000);
    glViewport(0, 0, (GLsizei)1000, (GLsizei)1000); // Set our viewport to the size of our window
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
    if (anim||crowd||favoid||shash) {
        for (int i = 0; i < NUMOFAGENTS; i++) {
            if (manager[i].isEmpty) break;
            manager[i].draw(manager[i].x, manager[i].y, manager[i].z);
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
        sprintf_s(num, "CUR FPS: %f AVG FPS: %f", curFps,fps_avg);
        glRasterPos2d(0.0, 0.0);
        glColor3f(1, 0, 0);
        int len = (int)strlen(num);
        for (int i = 0; i < len; i++) {
            glutBitmapCharacter(GLUT_BITMAP_9_BY_15, (int)num[i]);
        }


    }
   
    glFlush();  // Render now
}
float distance(float x1,float y1,float  x2,float y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}
bool willCollide(Circle *a, Circle *b) {
    float radi = 2 * ARADIS;
    float wX = b->x - a->x;
    float wY = b->y - a->y;
    float c = (wX * wX + wY * wY) - radi*radi;
    if (c < 0) return true;
    float vX = a->dirX - b->dirX;
    float vY = a->dirY - b->dirY;
    float dotV = vX * vX + vY * vY;
    float dotWV = wX * vX + wY * vY;
    float discr = dotWV * dotWV - dotV * c;
    if (discr <= 0) {
        return false;
    }
    float tau = (dotWV - sqrt(discr)) / dotV;
    if (tau < 0)return false;
    else return true;
}
void step(int i) {
    const float d_h = 10 * 2 * ARADIS;
    float v_x = manager[i].dirX;
    float v_y = manager[i].dirY;
    const float zeta = 1.0023;
    const float max_force = 0.1;
    const float max_speed = 1.5;
    const float timeStep = TIME_STEP;
    float f_goal_x = (manager[i].dir_goal_x - v_x) / zeta;
    float f_goal_y = (manager[i].dir_goal_y - v_y) / zeta;
   // if (manager[i].dir_goal_x > v_x) printf("ALERT!\n");
    float fAvoid_x = 0;
    float fAvoid_y = 0;
    float fAvoidCtr = 0;
    float interacting_agents = 0;
    for (int j = 0; j < NUMOFAGENTS; j++) {
        compteCount += 1;
        if (i == j) continue;
        float dist = distance(manager[i].x, manager[i].y, manager[j].x, manager[j].y);
        if (dist > 0 && dist < d_h) {
            //if (!willCollide(&manager[i], &manager[j])) continue;
            float d_ab = dist - 2 * ARADIS > 0.001 ? dist - 2 * ARADIS : 0.001;
            float k = d_h - d_ab > 0 ? d_h - d_ab : 0;
            float x_ab = (manager[i].x - manager[j].x) / dist;
            float y_ab = (manager[i].y - manager[j].y) / dist;
            interacting_agents += 1;
            fAvoid_x += k * x_ab / d_ab;
            fAvoid_y += k * y_ab / d_ab;
            fAvoidCtr += 1;
            }
        }
        if (fAvoidCtr > 0) {
            fAvoid_x = fAvoid_x / fAvoidCtr;
            fAvoid_y = fAvoid_y / fAvoidCtr;
        }
        float forceSum_x = f_goal_x + fAvoid_x;
        float forceSum_y = f_goal_y + fAvoid_y;
        float fAvoid_Mag = sqrtf(forceSum_x * forceSum_x + forceSum_y * forceSum_y);

        if (fAvoid_Mag > max_force) {
            forceSum_x = max_force * forceSum_x / fAvoid_Mag;
            forceSum_y = max_force * forceSum_y / fAvoid_Mag;
        }

        v_x += timeStep * forceSum_x;
        v_y += timeStep * forceSum_y;
        float speed = sqrtf(v_x* v_x+ v_y*v_y);
        if (speed > max_speed) {
            v_x = max_speed * v_x / speed;
            v_y = max_speed * v_y / speed;
        }
        manager[i].dirX = v_x;
        manager[i].dirY = v_y;
        manager[i].x += timeStep* v_x;
        manager[i].y += timeStep* v_y;
    
}

void hashStep(int i) {
    const float d_h = 7 * 2 * ARADIS;
    float v_x = manager[i].dirX;
    float v_y = manager[i].dirY;
    const float zeta = 1.0023;
    const float max_force = 1.55; //0.15
    const float max_speed = 1.5;
    const float timeStep = TIME_STEP;
    float f_goal_x = (manager[i].dir_goal_x - v_x) / zeta;
    float f_goal_y = (manager[i].dir_goal_y - v_y) / zeta;
    // if (manager[i].dir_goal_x > v_x) printf("ALERT!\n");
    float fAvoid_x = 0;
    float fAvoid_y = 0;
    float fAvoidCtr = 0;
    float interacting_agents = 0;
    //Getting lists of current and neighboring cells from hash
    int X = (int)(manager[i].x / CELL_SIZE) * CELL_SIZE;
    int Y = (int)(manager[i].y / CELL_SIZE) * CELL_SIZE;
    //string key = "          ";
   
   //string const& reset = "";
   // key.reserve(20);
    //key += to_string(X);
   // key += ",";
    //key += to_string(Y);
    vector<Circle*>* list;
    //vector<Circle *> list;
    //list.reserve(sizeof(Circle*) * 300);
    //vector<Circle *>* list = &spatialHash[key];   //This was the oringal list code
    
    /*
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            if (x == 0 && y == 0)continue;
            int adjx = X + x * CELL_SIZE;
            int adjy = Y + y * CELL_SIZE;
            if (adjx >= 500 || adjx <= -500) continue;
            if (adjy >= 500 || adjy <= -500) continue;
            key = "";
            key += to_string(adjx);
            key += ",";
            key += to_string(adjy);
            if (spatialHash.count(key) < 1) continue;
            for (int i = 0; i < spatialHash[key].size(); i++) {
                list.push_back(spatialHash[key][i]);
            }
        }
    }
    */
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            //if (x == 0 && y == 0)continue;
            int adjx = X + x * CELL_SIZE;
            int adjy = Y + y * CELL_SIZE;
            if (adjx >= 500 || adjx <= -500) continue;
            if (adjy >= 500 || adjy <= -500) continue;
            // key.clear();
             //key += to_string(adjx);
            // key += intToStr.at(adjx);///[adjx];
            // key += ",";
             //key += intToStr.at(adjy);//[adjy];
             //key += to_string(adjy);
            // if (spatialHash.count(key) < 1) continue;
            try {
                list = &spatialHash.at({ adjx,adjy });//&spatialHash[key];
            }
            catch (...) {
                continue;
            }
            //Running through circles in the current hash
            for (int j = 0; j < list->size(); j++) {
                compteCount += 1;
                Circle* c = (*list)[j];//(*list)[j];
                if (manager[i].id == c->id) continue; //manager[i].x == c->x && manager[i].y == c->y
                float dist = distance(manager[i].x, manager[i].y, c->x, c->y);
                if (dist > 0 && dist < d_h) {
                    //if (!willCollide(&manager[i], c)) continue;
                    float d_ab = dist - 2 * ARADIS > 0.001 ? dist - 2 * ARADIS : 0.001;
                    float k = d_h - d_ab > 0 ? d_h - d_ab : 0;
                    float x_ab = (manager[i].x - c->x) / dist;
                    float y_ab = (manager[i].y - c->y) / dist;
                    interacting_agents += 1;
                    fAvoid_x += k * x_ab / d_ab;
                    fAvoid_y += k * y_ab / d_ab;
                    fAvoidCtr += 1;
                }
            }
        }
    }
            if (fAvoidCtr > 0) {
                fAvoid_x = fAvoid_x / fAvoidCtr;
                fAvoid_y = fAvoid_y / fAvoidCtr;
            }
            float forceSum_x = f_goal_x + fAvoid_x;
            float forceSum_y = f_goal_y + fAvoid_y;
            float fAvoid_Mag = sqrtf(forceSum_x * forceSum_x + forceSum_y * forceSum_y);

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
        //}
    //}
    float oldX = manager[i].x;
    float oldY = manager[i].y;

    manager[i].dirX = v_x;
    manager[i].dirY = v_y;
    manager[i].x += timeStep * v_x;
    manager[i].y += timeStep * v_y;
    //Updates circles location in the hash [Double check that its working properly!]
    X = (int)(manager[i].x / CELL_SIZE) * CELL_SIZE;
    Y = (int)(manager[i].y / CELL_SIZE) * CELL_SIZE;
    
    /*
    key = "";
    key += to_string(X);
    key += ",";
    key += to_string(Y);
    if (key.compare(manager[i].cell) != 0) {
        //spatialHash[manager[i].cell].remove(manager[i]);   use for loop to look for value to delete!
        vector<Circle>* search = &spatialHash[manager[i].cell];
        for (int j = 0; j < search->size();j++) {
            //compteCount += 1;
            if ((*search)[j].id == manager[i].id) {
                spatialHash[manager[i].cell].erase(search->begin() + j);
                break;
            }
        }
        manager[i].cell = key;
        //spatialHash[manager[i].cell].push_back(manager[i]);
    }
    */
    //PROBLEM: appears the sim is falling to properly delete old adjcent cells (APPEARS AT LOOPCOUNT 96)
    //key.clear();
    //key += intToStr[X];
    //key += ",";
    //key += intToStr[Y];
    bool movedCell = false;
    int size = spatialHash.size();
    if (manager[i].cell.first != X || manager[i].cell.second != Y) { //key.compare(manager[i].cell) != 0
        movedCell = true;
        //Delete agent from old cell
        vector<Circle*>* search = &spatialHash[manager[i].cell];
        for (int j = 0; j < search->size(); j++) {
            if ((*(*search)[j]).id == manager[i].id) {
                spatialHash[manager[i].cell].erase(search->begin() + j);
                break;
            } 
        }
        //Remove new key from adj cells if it is in there
        for (int n = 0; n < 4; n++) {
            if (manager[i].adjcells[n].first == X && manager[i].adjcells[n].second == Y) { //key.compare(manager[i].adjcells[n]
                manager[i].adjcells[n].first = -555;
                manager[i].adjcells[n].second = -555;
            }
        }

        manager[i].cell.first = X;
        manager[i].cell.second = Y;
        //check to see if agent is already in cell
        search = &spatialHash[manager[i].cell];
        bool inCell = false;
        for (int j = 0; j < search->size(); j++) {
            if ((*(*search)[j]).id == manager[i].id) {
                inCell = true;
            }
        }
        if(!inCell) spatialHash[manager[i].cell].push_back(&manager[i]);
    }
    size = spatialHash.size();

    //Compare Old Min and Max with new max and min, and then adjust hash positions if overlapping with new cells
    int adjCount = 0;
    vector<Circle*>* check;
    bool reset = false;
    //This looks though adject cells to see if they are different than the current ones. if they are, add the agent to thouse regions
    //1. Get key to adjcent region 2. Check if key is already in adjcells, if not, replace, if so ignore and continue 3.
    for (int j = -1; j <= 1; j++) {
        for(int z = -1;z<=1;z++){
            if (j == 0) continue;
            if (z == 0) continue;
            size = spatialHash.size();
            int adjX = (int)((manager[i].x + j * ARADIS) / CELL_SIZE) * CELL_SIZE;
            int adjY = (int)((manager[i].y + z * ARADIS) / CELL_SIZE) * CELL_SIZE;
           
            //key.clear();
            //key += intToStr[adjX];
            //key += ",";
            //key += intToStr[adjY];
            //Serach to see if the key is already in a cell, if it is then ignore it and look at the next cell
            //also need to make sure it is acutally bordering a cell
            if ((manager[i].adjcells[adjCount].first == adjX && manager[i].adjcells[adjCount].second == adjY)|| (manager[i].cell.first == adjX && manager[i].cell.second == adjY)) {
                //Makes sure that the agent is in the adjacent cell (THIS SECTION IS PROBABLY COUNTER PRODUCTIVE! THINK OF DELETEING THIS!)
                /*
                if(key.compare(manager[i].adjcells[adjCount]) == 0){
                    bool notIn = true;
                    check = &spatialHash[manager[i].adjcells[adjCount]];
                    for (int k = 0; k < check->size(); k++) {
                        if ((*(*check)[k]).id == manager[i].id) {
                            notIn = false;
                            break;
                        }
                    }
                    if (notIn) {
                        spatialHash[manager[i].adjcells[adjCount]].push_back(&manager[i]);
                    }
                }
                */
                //If the agent is no longer adjcent in the tested direction, then remove the agent from the cell and null the key
                if ((manager[i].cell.first == adjX && manager[i].cell.second == adjY) && (manager[i].adjcells[adjCount].first != -555 && manager[i].adjcells[adjCount].second != -555)) {
                    coord stor = { manager[i].adjcells[adjCount].first,manager[i].adjcells[adjCount].second}; //Copying the old cell into key 
                    manager[i].adjcells[adjCount].first = -555;
                    manager[i].adjcells[adjCount].second = -555;
                    //Check to see if the agent is completly unadjcent to the old cell
                    //if it isnt, ignore it. If it is, remove itself from the old cell
                    bool stillIn = false;
                    for (int n = 0; n < 4; n++) {
                        if (manager[i].adjcells[n].first == stor.first && manager[i].adjcells[n].second == stor.second) {
                            stillIn = true;
                            break;
                        }
                    }
                    if (!stillIn) {
                        //If it is not in the adjcell list, remove it from the old cell
                        check = &spatialHash[stor];
                        for (int k = 0; k < check->size(); k++) {
                            if ((*(*check)[k]).id == manager[i].id) {
                                spatialHash[stor].erase(check->begin() + k);
                                break;
                            }
                        }
                    }
                }
                adjCount++;
                continue;
            }
            
            //Since we already know the key must be different, erase the agent from the previous cell 
            //We still need to make sure that there is actually anything to delete in the hash
            if(manager[i].adjcells[adjCount].first != adjX && manager[i].adjcells[adjCount].second != adjY){
                //Check to see if the current cell being looked at is already in the list
                bool escape = false;
                for (int n = 0; n < 4; n++){
                    if (n == adjCount) continue;
                    if (manager[i].adjcells[adjCount].first == manager[i].adjcells[n].first && manager[i].adjcells[adjCount].second == manager[i].adjcells[n].second) {
                        escape = true;
                        break;
                    }
                }
                //If it is not still in the list, then delete it from the previous cell
                if(!escape){
                    check = &spatialHash[manager[i].adjcells[adjCount]];
                    for (int k = 0; k < check->size(); k++) {
                        if ((*(*check)[k]).id == manager[i].id) {
                            spatialHash[manager[i].adjcells[adjCount]].erase(check->begin() + k);
                            break;
                        }
                    }
                }
            }
            //Need to check to see if the key exists but it moved
            //If it has just merely moved to a different side of the agent, then move the key and move to another cell
            for (int n = 0; n < 4; n++) {
                if (adjX == manager[i].adjcells[n].first && adjY == manager[i].adjcells[n].second) {
                    manager[i].adjcells[adjCount].first = adjX;
                    manager[i].adjcells[adjCount].second = adjY;
                    adjCount++;
                    reset = true;
                    break;
                }
            }
            if (reset) {
                reset = false;
                continue;
            }
            //assign key to adj cells and add itself to hash. Then move on to next cell
                manager[i].adjcells[adjCount].first = adjX;
                manager[i].adjcells[adjCount].second = adjY;
                spatialHash[manager[i].adjcells[adjCount]].push_back(&manager[i]);
            
                adjCount += 1;
            
        }
        
    }
    size = spatialHash.size();
    loopCount++;
    if (movedCell) {
        printf("MOVED CELL\n");
    }
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
                if (manager[i].x > 500) manager[i].x = -499;
                if (manager[i].x < -500) manager[i].x = 499;
                if (manager[i].y > 500) manager[i].y = -499;
                if (manager[i].y < -500) manager[i].y = 499;
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
    }

        //hash system here
        //setup loop based on cell amount and cell size, go through each cell A) see what agents are there and B) check what collsions exist 
        //TODO: Create add agent function
        if (shash) {
          
            //Make sure agents are in bounds
            for (int i = 0; i < NUMOFAGENTS; i++) {
                manager[i].stepF(i);
                if (manager[i].x > 500) manager[i].x = -499;
                if (manager[i].x < -500) manager[i].x = 499;
                if (manager[i].y > 500) manager[i].y = -499;
                if (manager[i].y < -500) manager[i].y = 499;
            }
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
            favoid = true;
        }
        if (strcmp("-shash", argv[i]) == 0) {
            shash = true;
            fps = true;
        }
    }
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
