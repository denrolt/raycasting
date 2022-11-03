#include <glfw3.h>
#include <time.h>
#include <stdio.h>
#include <windows.h>
#include <math.h>
#include <stdbool.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define rad(a) (a)/180.0f * PI
#define mapxy(a) int(a) / tile * tile                        // нахожение горизонталей и вертикалей матрицы 
#define mapwh(a) int(a) / tile                               // нахождение индекса массива карты по координатам
#define mas(a,b) 0<=(mapwh(a))&&(mapwh(a))<=maph && 0<=(mapwh(b))&&(mapwh(b))<=mapw // проверка границ массива
const float PI = 3.14159265359f;
const int tile = 32;
const int width = 1024;
const int height = 768;
const int mapw = 13;
const int maph = 13;
const float ratwh = (float)height / width;
const int mapscale = 15;
int map[maph][mapw] =
{ {1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,1,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,1,0,0,0,0,0,1},
  {1,1,1,0,1,1,1,1,1,1,0,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,1,0,0,1,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,1,0,0,0,0,0,0,0,1,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1} }; // карта 


struct cam
{
    const int size = 25;
    const int speed = 100;
    const int povspeed = 100;
    bool KeyW = false;
    bool KeyS = false;
    bool KeyA = false;
    bool KeyD = false;
    float x = 0, y = 0;
    float pov = 0, fov = 60;
    float dirx = 0, diry = 0;
} cam;


int vertexWall[width * 4]; // массив линий
GLbyte color[width * 2 * 3];


void key_call (GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_W && action == 1) cam.KeyW = true;
    else if (key == GLFW_KEY_W && action == 0) cam.KeyW = false;

    if (key == GLFW_KEY_S && action == 1) cam.KeyS = true;
    else if (key == GLFW_KEY_S && action == 0) cam.KeyS = false;

    if (key == GLFW_KEY_A && action == 1) cam.KeyA = true;
    else if (key == GLFW_KEY_A && action == 0) cam.KeyA = false;

    if (key == GLFW_KEY_D && action == 1) cam.KeyD = true;
    else if (key == GLFW_KEY_D && action == 0) cam.KeyD = false;
}

void line(int x1, int y1, int x2, int y2)
{
    glBegin(GL_LINES);
    glVertex2i(x1,y1);
    glVertex2i(x2,y2);
    glEnd();
}

void rectangle(int x1, int y1, int x2, int y2)
{
    
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2i(x1+1, y1+1);
    glVertex2i(x1+1, y2-1);
    glVertex2i(x2-1, y1+1);
    glVertex2i(x2-1, y2-1);
    glEnd();
}

GLfloat wallxy[width * 4];

unsigned int tex;
unsigned int sky;

void loadtex (unsigned int *tex, char *str )
{
    {
        int wid, hei, cnt;
        unsigned char* data = stbi_load(str, &wid, &hei, &cnt, 0);

        glGenTextures(1, tex);
        glBindTexture(GL_TEXTURE_2D, *tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, cnt == 4 ?GL_RGBA:GL_RGB, wid, hei, 0, cnt == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);
    }

}

void mapshow()
{
    for (int i = 0; i < maph; i++)
        for (int j = 0; j < mapw; j++)
        {
            if (map[i][j] == 0)
                glColor3ub(150,150,150);
            else if (map[i][j] == 1)
                glColor3ub(0, 255, 0);
            else if(map[i][j] == 2)
                glColor3ub(150, 40, 40);

            rectangle(j * mapscale, i * mapscale, j * mapscale + mapscale, i * mapscale + mapscale);

        }
}


void camshow()
{
    glColor3f(1,1,0);
    glPointSize(10);
    glBegin(GL_POINTS);
    glVertex2i(int(cam.x / tile * mapscale), int(cam.y / tile * mapscale));
    glEnd();
    glLineWidth(2);
    glColor3b(200,22,0);
    line(cam.x / tile * mapscale, cam.y / tile * mapscale,(cam.x+30*cos(rad(cam.pov)))/ tile * mapscale,(cam.y+30*sin(rad(cam.pov))) / tile * mapscale);
    glLineWidth(1);
}


void camMove(GLFWwindow* window,GLdouble tick)
{
    float ax = cam.x;
    float ay = cam.y;

    if (cam.KeyW)
    {
        ax += cam.speed * cos(rad(cam.pov))*tick;
        ay += cam.speed * sin(rad(cam.pov))*tick;
    }
    else if (cam.KeyS)
    {
        ax -= cam.speed * cos(rad(cam.pov))*tick;
        ay -= cam.speed * sin(rad(cam.pov))*tick;
    }

    if (cam.KeyA)
        cam.pov -= cam.povspeed*tick;
    else if (cam.KeyD)
        cam.pov += cam.povspeed * tick;

    if (cam.pov > 359) cam.pov = 0;
    else if (cam.pov < 0) cam.pov = 359;

    if (map[mapwh(ay)][mapwh(ax)]==0)
    {
        cam.x = ax;
        cam.y = ay;
    };
}

void setcolor(int i, unsigned char r, unsigned char g, unsigned char b)
{
    color[i * 6] = r;
    color[i * 6 + 1] = g;
    color[i * 6 + 2] = b;

    color[i * 6 + 3] = r;
    color[i * 6 + 4] = g;
    color[i * 6 + 5] = b;
}



void dir()
{
    float x0 = cam.x;
    float y0 = cam.y;

    for (int i = 0; i < width; i++)
    {
        float angle = cam.pov - cam.fov / 2 + cam.fov / width * i;
        float depth_V = 0;
        float depth_H = 0;

        float cos_ang = cos(rad(angle));
        float sin_ang = sin(rad(angle));

        float xmap = cos_ang >= 0 ? mapxy(cam.x) : mapxy(cam.x + tile);
        float ymap = sin_ang >= 0 ? mapxy(cam.y) : mapxy(cam.y + tile);

        int tellmove = cos_ang >= 0 ? tile : -tile;
        float dely = 0;

        // Персечение ближних вертикалей
        while (angle != 270 && angle != 90 && mas(dely, xmap) && map[mapwh(dely)][mapwh(xmap - (cos_ang < 0))] == 0 || dely == 0)
        {
            xmap += tellmove;
            depth_V = (xmap - x0) / cos_ang;
            dely = y0 + depth_V * sin_ang;
        }
        tellmove = sin_ang >= 0 ? tile : -tile;
        float delx = 0;

        // Персечение ближних горозонталей
        while ( angle != 0 && angle != 180 && mas(ymap, delx) && map[mapwh(ymap - (sin_ang < 0))][mapwh(delx)] == 0 || delx == 0)
        {
            ymap += tellmove;
            depth_H = (ymap - y0) / sin_ang;
            delx = x0 + depth_H * cos_ang;
        }

        float mapcoord = (depth_V <= depth_H) ? map[mapwh(dely)][mapwh(xmap - (cos_ang < 0))]:map[mapwh(ymap - (sin_ang < 0))][mapwh(delx)]; // находим кубик

        int quality = 4;

        float ofset = depth_V <= depth_H?int(dely * quality) % (tile*quality) : int(delx * quality) % (tile * quality);
        wallxy[i * 4] = 1.0f / (tile * quality) * ofset;
        wallxy[i * 4+1] = 1;
        wallxy[i * 4+2] = 1.0f / (tile * quality) * ofset;
        wallxy[i * 4+3] = 0;



        float depth = depth_V < depth_H ? depth_V : depth_H;
        depth *= cos(rad(cam.pov - angle));
        depth = (depth < 1) ? 1 : depth;
        float topwall = (height / 2) * (1 - 1 / depth*tile);
        float botwall = (height / 2) * (1 + 1 / depth*tile);
        vertexWall[i * 4] = i;           //x1
        vertexWall[i * 4 + 1] = topwall; //y1

        vertexWall[i * 4 + 2] = i;       //x1
        vertexWall[i * 4 + 3] = botwall; //y1
        
        (depth_H <= depth_V)?setcolor(i,70,70,70): setcolor(i, 120, 120, 120);


        //line(i, topwall , i , botwall);
        angle = cam.pov - cam.fov / 2+cam.fov / width * i;
    }
}
void wallShow()
{
    glBegin(GL_QUADS);
    glColor3ub(30, 30, 30);
    glVertex2i(0,height/2);
    glVertex2i(width, height/2);
    glVertex2i(width, height);
    glVertex2i(0, height);
    glEnd();
    int skycoord[] = {0,0,   width,0,   width,height/2, 0,height/2};
    int r = 4;
    GLfloat skytexcoord[8] = { cam.pov / 360*r,0,  (cam.pov + 60) / 360*r,0,  (cam.pov + 60) / 360*r,1, cam.pov / 360*r,1 };

    glColor3ub(255, 255, 255);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, sky);

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    glTexCoordPointer(2, GL_FLOAT, 0, skytexcoord);
    glVertexPointer(2, GL_INT, 0, skycoord);

    glDrawArrays(GL_QUADS, 0, 4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
    



    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,tex);

    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    glColorPointer(3,GL_BYTE,0,color);
    glTexCoordPointer(2,GL_FLOAT,0,wallxy);
    glVertexPointer(2,GL_INT,0,vertexWall);
    glDrawArrays(GL_LINES,0,width*2);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
}

int main(void)
{
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(width, height, "Raycasting", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    memset(vertexWall, 0, width * 4);
    memset(vertexWall, 0, width * 2*3);
    memset(wallxy, 0, width * 4);
    glfwSetKeyCallback(window, key_call);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    cam.x = 250;
    cam.y = 250;
    glOrtho(0, width, height, 0, 1, -1);
   
    char namewall [] = "C:\\Users\\Дмитрий\\Desktop\\стена2.png";
    char namesky[] = "C:\\Users\\Дмитрий\\Desktop\\sky.png";

    GLdouble time1 = 0;
    GLdouble time2 = 0;
    GLdouble suma = 0;

    loadtex(&tex, namewall);
    loadtex(&sky, namesky);
    while (!glfwWindowShouldClose(window))
    {
        time1 = time2;
        time2 = glfwGetTime();
        suma += (time2 - time1);
        if (suma > 1)
        {
            printf("%.2f\n", 1 / (time2 - time1));
            suma = 0;
        }

        camMove(window,time2-time1);
        dir();

        glClear(GL_COLOR_BUFFER_BIT);

        wallShow();
        mapshow();
        camshow();

        glfwSwapBuffers(window);

        int iwid, ihei;

        glfwGetWindowSize(window, &iwid, &ihei);
        if (iwid != width || ihei!=height) glfwSetWindowSize(window,width,height);

        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
