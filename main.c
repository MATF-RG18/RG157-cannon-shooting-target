#include<stdio.h>
#include<GL/glut.h>
#include<math.h>
#include"image.h"
#include <string.h>

//Macro for cannon rotations
#define INCREMENT_CANNON_ANGLE 5
#define MAX_TARGETS 6
#define TARGET_SIZE 0.5

//These are macros for the sea
#define U_FROM -100
#define V_FROM -8
#define U_TO 100
#define V_TO 8

//Texture names
#define FILENAME0 "box.bmp"
#define FILENAME1 "stone.bmp"

//This is a macro that represents z coordinate of the targets
#define PLATFORM_DISTANCE 4

//Target structure, flag 'is_visible' equals 0 if the target is hit, otherwise it's 1
typedef struct target {
    double x;
    double y;
    double z;
    int is_visible;
} TARGET;

TARGET targets[MAX_TARGETS];

static void on_keyboard(unsigned char key, int x, int y);
static void on_reshape(int width, int height);
static void on_Timer(int value);
static void on_display(void);
static void on_motion(int x, int y);
static void on_mouse(int button, int state, int x, int y);
static void draw_cannon(float radius, float height);
static void draw_targets();
static void on_timer2(int value);
static float function(float u, float v);
static void set_vertex_and_normal(float u, float v);
static void plot_function();

//Time for sea movement
float t;

int windowWidth;
int windowHeight;

//Flag for on_timer2 function
static int animation_ongoing2;

//here we will store how many balls the cannon fired and how much of them hit the target
int shot, hit;

//coordinates along which the cannon rotates when aiming
float cannon_movement_x;
float cannon_movement_y;

//coordinates of the cannon ball
float cannon_ball_x;
float cannon_ball_y;
float cannon_ball_z;

//initial velocity (the cannon ball moves along y,z axis)
float ivelY;
float ivelZ;
float ivelX; //added to make cannon ball movement smoother

//constant which regulates the speed of the cannon ball
float iVel;

//current cannon ball velocity
float velZ;
float velY;
float velX; //added to make cannon ball movement smoother

float Time; //time past from firing the cannon

//Flag for cannon ball timer
int animation_ongoing;

const float DEGTORAD = 3.1415769/180.0f; //angle in degrees*DEGTORAD = angle in radians

//IDs for the texture files
static GLuint names[2];

//This variable indicates whether we use the keyboard or the mouse for giving commands
//k == 1 -> keyboard
//k == 0 ->mouse
static int k;

//These two functions are copied from openGL library, but with added functionality to draw textured cubes
static void drawBox(GLfloat size, GLenum type, int texture);
void glutTexturedSolidCube(GLdouble size, int texture);

//This function is requied to print score on the screen
void printText(GLfloat x, GLfloat y, GLfloat z, char *string);

int main(int argc, char** argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

    glutInitWindowSize(600, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(argv[0]);

    glutKeyboardFunc(on_keyboard);
    glutReshapeFunc(on_reshape);
    glutDisplayFunc(on_display);
    glutPassiveMotionFunc(on_motion);
    glutMouseFunc(on_mouse);

    glClearColor(0, 0.74901960784, 1, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);

    //Time for sea movement
    t = 0;

    //angles for rotation of the cannon
    cannon_movement_x = 0.0;
    cannon_movement_y = 0.0;


    //cannon ball position at each given time
    cannon_ball_x = 0;
    cannon_ball_y = 0;
    cannon_ball_z = 0;

    //Ball velocity at the start of the program
    velZ = 0.0f;
    velY = 0.0f;
    velX = 0.0f;

    //Balls shot and balls that hit the target
    hit = 0;
    shot = 0;

    //Time passed since ball was fired
    Time = 0;

    //Flag for ball timer
    animation_ongoing = 0;

    //initially we use the mouse for giving commands
    k = 0;

    int i;
    for(i = 0; i < MAX_TARGETS; i++) {
        targets[i].is_visible = 1;
        targets[i].z = PLATFORM_DISTANCE;
    }

    targets[0].x = -1;
    targets[0].y = -0.5;

    targets[1].x = 0;
    targets[1].y = -0.5;

    targets[2].x = 1;
    targets[2].y = -0.5;

    targets[3].x = -0.5;
    targets[3].y = 0;

    targets[4].x = 0.5;
    targets[4].y = 0;

    targets[5].x = 0;
    targets[5].y = 0.5;

    Image *image;
    glEnable(GL_TEXTURE_2D);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    image = image_init(0, 0);

    image_read(image, FILENAME0);

    glGenTextures(2, names);

    //Binding texture for FILENAME0
    glBindTexture(GL_TEXTURE_2D, names[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
            image->width, image->height, 0,
            GL_RGB, GL_UNSIGNED_BYTE, image->pixels);


    image_read(image, FILENAME1);

    //Binding texture for FILENAME1
    glBindTexture(GL_TEXTURE_2D, names[1]);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 image->width, image->height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, image->pixels);


    //Disabling texturing
    glBindTexture(GL_TEXTURE_2D, 0);

    image_done(image);

    glutMainLoop();
    return 0;
}

static void on_mouse(int button, int state, int x, int y){
    if(k == 0){
      if(button == GLUT_LEFT_BUTTON && !animation_ongoing) {
          Time = 0;
          shot += 1;
          animation_ongoing = 1;
          iVel = 0.3;
          ivelZ = iVel * cos(cannon_movement_x * DEGTORAD) * cos(cannon_movement_y * DEGTORAD);
          ivelY = -iVel * sin(cannon_movement_x * DEGTORAD);
          ivelX = iVel * cos(cannon_movement_x * DEGTORAD) * sin(cannon_movement_y * DEGTORAD);
          glutTimerFunc(50, on_Timer, 0);
          glutPostRedisplay();
      }
      if(button == GLUT_RIGHT_BUTTON){
            //we need to reset all variables for cannon reloading
              animation_ongoing = 0;
              cannon_ball_z = 0;
              cannon_ball_y = 0;
              cannon_ball_x = 0;
              velZ = 0;
              velY = 0;
              velX = 0;
              glutPostRedisplay();
      }
    }
}

static void on_motion(int x, int y){
    if(k == 0){
      //Idea for this function is that we first find position X and Y where
      //posX represents pixel distance from the center of the screen in x coordinates
      //and posY represents pixel distance from the center of the screen in y coordinates
      //because of that our posX and posY are in interval [-height/2, height/2] for Y
      // since the biggest distance from the center of the screen is height/2 and smallest is -height/2
      // in the same respect posX is in interval [-width/2, width/2];
      //we want our cannon to rotate for -45 degrees around y axis when our mouse is on the left edge of the screen
      // and to rotate for 45 degrees when our mouse
      // is on the right edge of the screen
      //In the same manner we want our cannon to rotate around x axis for -30 degrees when our mouse is on the top
      //edge of the screen and for 30 degrees when our mouse is in the bottom of the screen.
      // Because of that we only need linear mapping between two intervals.

      double posY = y - windowHeight/2; // scaling [-height/2, height/2] -> [-45, 45]
      double posX = x - windowWidth/2; // scaling [-width/2, width/2] -> [30, -30]
      //using formula for scaling x from range [a, b] to range [c, d]
      // f(x) = (x-a) * ((d-c) / (b-a)) + c , where f(a) = c, f(b) = d;

      cannon_movement_x = (posY+(windowHeight*1.0)/2)*(90/(1.0*windowHeight)) - 45;
      if(cannon_movement_x > 5)
          cannon_movement_x = 5;

      cannon_movement_y = (posX+(windowWidth*1.0)/2)*(-60/(windowWidth*1.0)) + 30;

      glutPostRedisplay();
    }
}

static void on_keyboard(unsigned char key, int x, int y){
    switch (key) {
      case 27:
          exit(0);
          break;
      case 'k':
      case 'K':
      //keyboard -> mouse and vice versa
        k = !k;
        break;
    }
    if(k == 1){
      switch(key){
          case 'w':
          case 'W':
              if(cannon_movement_x > -30)
                  cannon_movement_x -= INCREMENT_CANNON_ANGLE;
              glutPostRedisplay();
              break;
          case 's':
          case 'S':
              if(cannon_movement_x < 5) //we can't have the cannon go too low because of the stand it's on
                  cannon_movement_x += INCREMENT_CANNON_ANGLE;
              glutPostRedisplay();
              break;
          case 'd':
          case 'D':
              if(cannon_movement_y > -45)
                  cannon_movement_y -= INCREMENT_CANNON_ANGLE;
              glutPostRedisplay();
              break;
          case 'a':
          case 'A':

              if(cannon_movement_y < 45)
                  cannon_movement_y += INCREMENT_CANNON_ANGLE;
              glutPostRedisplay();
              break;
          case 'q':
          case 'Q':
              if(!animation_ongoing) {
                  Time = 0;
                  shot += 1;
                  animation_ongoing = 1;
                  iVel = 0.3;
                  ivelZ = iVel * cos(cannon_movement_x * DEGTORAD) * cos(cannon_movement_y * DEGTORAD);
                  ivelY = -iVel * sin(cannon_movement_x * DEGTORAD);
                  ivelX = iVel * cos(cannon_movement_x * DEGTORAD) * sin(cannon_movement_y * DEGTORAD);
                  glutTimerFunc(50, on_Timer, 0);
              }
              break;
          case 'r':
          case 'R':
              //we need to reset all variables for cannon reloading
              animation_ongoing = 0;
              cannon_ball_z = 0;
              cannon_ball_y = 0;
              cannon_ball_x = 0;
              velZ = 0;
              velY = 0;
              velX = 0;
              glutPostRedisplay();
              break;
      }
    }
}

static void on_reshape(int width, int height){
    glViewport(0, 0, width, height);
    windowHeight = height;
    windowWidth = width;


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (float)width/height, 1, 1500);
}

static void on_Timer(int value){
    if(value != 0)
        return ;

    if(cannon_ball_z > -50){
        cannon_ball_z += velZ;
        cannon_ball_y += velY - 9.8 * Time;  //we must subtract gravity * Time
        cannon_ball_x += velX;

        //cannon ball velocity at current time
        velY = ivelY;
        velZ = ivelZ;
        velX = ivelX;

        Time += 0.001;

        int i;
        for(i = 0; i < MAX_TARGETS; i++){
            if(cannon_ball_z  <= targets[i].z + TARGET_SIZE / 2.0
                && cannon_ball_z >= targets[i].z - TARGET_SIZE / 2.0

                && cannon_ball_y <= targets[i].y + TARGET_SIZE / 2.0
                && cannon_ball_y >= targets[i].y - TARGET_SIZE / 2.0

                && cannon_ball_x >= targets[i].x - TARGET_SIZE / 2.0
                && cannon_ball_x <= targets[i].x + TARGET_SIZE / 2.0
                && targets[i].is_visible
              ){
                hit += 1;
                targets[i].is_visible = 0;
            }
        }
    }
    else{
        animation_ongoing = 0;
    }

    glutPostRedisplay();

    if(animation_ongoing) {
        glutTimerFunc(50, on_Timer, 0);
    }
}

static void on_display(void){
    //Clear color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //light position and light properties
    GLfloat light_position[] = { 1, 1, 1, 0 };
    GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1 };
    GLfloat light_diffuse[] = { 0.7, 0.7, 0.7, 1 };
    GLfloat light_specular[] = { 0.9, 0.9, 0.9, 1 };

    //setting camera position, what we are looking at and up vector
    gluLookAt(0,2, -1, 0, 1.1, 0, 0, 1, 0);

    //Enable lighting and set lighting properties
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    char score[50];
    sprintf(score, "shot: %d balls", shot);
    printText(3.42, 0.2, 6, score);


    char score2[50];
    sprintf(score2, "hit: %d targets", hit);
    printText(3.3, 0, 5.5, score2);


    //Here we start sea animation so it moves
    if(!animation_ongoing2) {
        glutTimerFunc(10, on_timer2, 0);
        animation_ongoing2 = 1;
    }
    //Set shading to be smooth
    glShadeModel(GL_SMOOTH);

    //blue rectangle that represents the sea
    glPushMatrix();
        glTranslatef(0, -1.5, 0);
        plot_function();
    glPopMatrix();

    glPushMatrix();
        glTranslatef(0, -2.3, PLATFORM_DISTANCE);
        glutTexturedSolidCube(3, 1); //surface on which boxes stand
    glPopMatrix();

    glPushMatrix();
        draw_targets();
    glPopMatrix();

    draw_cannon(0.2, 0.7);

    glutSwapBuffers();
}


/*
 *Function draw_cannon draws a cannon that consists of a sphere and a cylinder
 * and sets their lighting material coefficients.
 * The sphere was created by using an openGL function, and the cylinder
 * was created by drawing quad strips in a loop creating a tube (the bases are not necessary in this case).
*/
static void draw_cannon(float radius, float height){
     //setting the material lighting attributes
    GLfloat ambient_coeffs[] = { 0.05375, 0.05, 0.06625, 1 };
    GLfloat diffuse_coeffs[] = { 0.18275, 0.17, 0.22525, 1 };
    GLfloat specular_coeffs[] = {  0.332741, 0.328634, 0.346435, 1 };

    GLfloat shininess = 0.3*128;
    //Setting material lighting properties
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient_coeffs);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse_coeffs);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular_coeffs);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);


    float x = 0.0;  //x and y are polar coordinates
    float y = 0.0;
    float angle = 0.0; //initial angle which will be used for calculating polar coordinates
    float angle_step = 0.005; //value for increasing of the angle in the loop

    /*
     The tube of the cylinder is drawn by drawing 2 vertices in each iteration, and connecting
     them in a quad strip.
     When the loop is exited we need to draw two 2 more vertices to complete the tube, since
     the angle will never be exactly equal to 2pi, so the last strip needs to be drawn explicitly.
     */

    glPushMatrix();
        glTranslatef(0, -0.5, 0.2);
        glutTexturedSolidCube(0.9, 1);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(cannon_ball_x, cannon_ball_y, cannon_ball_z);
        glutSolidSphere(0.1, 100, 100);
    glPopMatrix();


    //Rotations for aiming
    glPushMatrix();
        glRotatef(cannon_movement_x, 1, 0, 0);
        glRotatef(cannon_movement_y, 0, 1, 0);
        //begin to draw cylinder from quad stripes
        glBegin(GL_QUAD_STRIP);
            while(angle < 2*M_PI){
                x = radius*cos(angle);//x polar coordinate
                y = radius*sin(angle);//y polar coordinate
                glNormal3f(x / radius, y / radius, 0.0); //normal for each vertex (important for lighting)
                glVertex3f(x, y, height);//draw upper vertex
                glVertex3f(x, y, 0.0); // draw lower vertex
                angle += angle_step; // increase angle
            }
            glVertex3f(radius, 0.0, height); // draw last two vertices
            glVertex3f(radius, 0.0, 0.0);
    glEnd();

    glutSolidSphere(radius, 100, 100);

    glPopMatrix();

}

static void draw_targets() {
    int i;
    for(i = 0; i < MAX_TARGETS; i++) {
        if(targets[i].is_visible) {
            glPushMatrix();
                glTranslatef(targets[i].x, targets[i].y, targets[i].z);
                glutTexturedSolidCube(TARGET_SIZE, 0); //draw cube and translate it to desired place
            glPopMatrix();
        }
    }
}

/* Next two functions are taken from openGL library and they represent glutSolidCube function
 *with added textures, int texture represents which texture we want*/
static void drawBox(GLfloat size, GLenum type, int texture){
  static GLfloat n[6][3] =
  {
    {-1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {1.0, 0.0, 0.0},
    {0.0, -1.0, 0.0},
    {0.0, 0.0, 1.0},
    {0.0, 0.0, -1.0}
  };
  static GLint faces[6][4] =
  {
    {0, 1, 2, 3},
    {3, 2, 6, 7},
    {7, 6, 5, 4},
    {4, 5, 1, 0},
    {5, 6, 2, 1},
    {7, 4, 0, 3}
  };
  GLfloat v[8][3];
  GLint i;

  v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 2;
  v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 2;
  v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 2;
  v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2;
  v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 2;
  v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 2;


  glBindTexture(GL_TEXTURE_2D, names[texture]); // Here we use 'texture' argument to specify which texture we want
  for (i = 5; i >= 0; i--) {
    glBegin(type);
    glNormal3fv(&n[i][0]); //The only thing that is different from glutSolid cube is that we add glTexCoord function call while drawing vertices
            glTexCoord2f(0, 0);
    glVertex3fv(&v[faces[i][0]][0]);
            glTexCoord2f(1, 0);
    glVertex3fv(&v[faces[i][1]][0]);
            glTexCoord2f(1, 1);
    glVertex3fv(&v[faces[i][2]][0]);
            glTexCoord2f(0, 1);
    glVertex3fv(&v[faces[i][3]][0]);
    glEnd();
  }
    glBindTexture(GL_TEXTURE_2D, 0); // Disable texturing
}

void glutTexturedSolidCube(GLdouble size, int texture){
  drawBox(size, GL_QUADS, texture);
}

static float function(float u, float v) {
    return sin((t+u*30+v*500)/60)/15; //This function is made with random numbers trying to achieve that function looks like waves
}

static void set_vertex_and_normal(float u, float v) {
    float diff_u, diff_v; //vertex differentiation, it represents normal vector

    diff_u = (function(u+1, v) - function(u-1, v)) / 2.0; //vertex differentiation approximation
    diff_v = (function(u, v+1) - function(u, v-1)) / 2.0;

    glNormal3f(-diff_u, 1, -diff_v); //setting normal for vertex for lighting

    glVertex3f(u, function(u, v), v); //drawing vertex
}

static void plot_function() {

    //'Sea' material lighting coefficients
    GLfloat ambient_coeffs[] = { 100.0/255, 149.0/255, 237/255.0, 1.0f };
    GLfloat diffuse_coeffs[] = { 100.0/255, 149.0/255, 237/255.0, 1.0f };
    GLfloat specular_coeffs[] = { 100.0/255, 149.0/255, 237/255.0, 1.0f };
    GLfloat shininess = 92;

    //Enable material properties
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient_coeffs);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse_coeffs);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular_coeffs);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

    glEnable(GL_LIGHTING);

    int u, v;

    /*Here we draw rectangle where lower left vertex has coordinates (U_FROM, V_FROM)
     * and upper right vertex has coordinates (U_TO, V_TO)*/
    glPushMatrix();
        for(u = U_FROM; u < U_TO; u++) {
            glBegin(GL_TRIANGLE_STRIP);
                for(v = V_FROM; v <= V_TO; v++) {
                    set_vertex_and_normal(u, v);
                    set_vertex_and_normal(u + 1, v);
                }
            glEnd();
        }
    glPopMatrix();
}

//In this timer we just want to increase time passed for the 'wave' animation on our 'sea'
static void on_timer2(int value) {
    if(value != 0)
        return;
    t += 10;

    glutPostRedisplay(); //Redisplay after incrementing time
    if(animation_ongoing2) {
        glutTimerFunc(10, on_timer2, 0);
    }
}

void printText(GLfloat x, GLfloat y, GLfloat z, char *string) {
  glColor3f(1, 1, 1);
  glRasterPos3f(x,y,z);
  int len, i;
  len = (int)strlen(string);
  for (i = 0; i < len; i++) {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, string[i]);
  }
}
