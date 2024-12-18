#define GL_SILENCE_DEPRECATION
#include <cstdio>
#include <iostream>
#include <GLUT/glut.h>
#include <cmath>
#include <SFML/Audio.hpp>

sf::Music background;
sf::Music collision;
sf::Music animationMusic;



double eyeX = 2.3, eyeY = 1.3, eyeZ = 2.0;  // Initial camera position
double centerX = 0, centerY = 0, centerZ = 0;  // Look-at point
double cameraYaw = 0.0, cameraPitch = 0.0;    // Camera angles
double cameraDistance = 10.0;                  // Distance from the center point
double pMotionX = 0.0, pMotionZ = 0.0;
double pMotionSpeed = 0.1;        // Speed of player movement
bool isMovingLeft = false;
bool isMovingRight = false;
double playerRotation = 0.0;  // Player's current rotation angle
const double ROTATION_SPEED = 5.0;  // Degrees per rotation
bool isRotatingLeft = false;
bool isRotatingRight = false;
int lastMouseX, lastMouseY;                   // Store the last mouse position
bool preset;
const float FIELD_BOUNDARY = 14.9f;
const int COLOR_CHANGE_INTERVAL = 1000;
int javelinCount = 0;
float colorChangeR = 0.0, colorChangeG = 0.0, colorChangeB = 0.0;
// if you want continuous change of colors

int colorState = 0;     // Tracks which colors are transitioning
float colorSpeed = 0.005f; // Speed of color transition

struct GameState {
    bool hasJavelin;
    int score;
    bool needNewJavelin;
    float timeRemaining;  // Time in seconds
    bool isGameOver;
    bool isWin;
} gameState = {false, 0, true, 600.0f, false, false};

struct Points {
    float x, z;  // We use x and z since y is up in OpenGL
};

// Add these global variables at the top of the file
struct Javelin {
    float x;
    float z;
    bool active;  // Whether the javelin is currently visible
};

Javelin currentJavelin = {0, 0, false};

struct Podium {
    float x;
    float z;
};

Podium podium = {-5, -4};

// Constants for boundaries
const float JAVELIN_FIELD_MIN_X = -0.9f;  // Adjust these values based on your field size
const float JAVELIN_FIELD_MAX_X = -8.5f;
const float JAVELIN_FIELD_MIN_Z = -4.0f;
const float JAVELIN_FIELD_MAX_Z = 6.0f;
const float COLLISION_DISTANCE = 1.5f;  // Distance threshold for picking up javelin



// Define the curved lines parameters
const int POINTS_PER_CURVE = 50;  // Resolution of each curve
const float FIELD_WIDTH = 10.0f;   // Match your ground width
const float FIELD_LENGTH = 9.0f;   // Match your ground depth
const float CURVE_SPREAD = 2.0f;   // How wide the curves spread

struct AnimationState {
    bool isAnimating;
    float rotation;
    float scale;
    float translation;
    float colorChange;
} podiumAnim, scoreboardAnim, tripodAnim, lightPoleAnim, flagPoleAnim, javelinAnim;

// Animation parameters
const float ROTATION_SPEED_ANIM = 2.0f;
const float SCALE_SPEED = 0.01f;
const float TRANSLATION_SPEED = 0.05f;
const float COLOR_CHANGE_SPEED = 0.02f;

void Anim();

void loadSounds(){
    background.openFromFile("~/Assignment 2/Sounds/OlympicEnd.wav");
    collision.openFromFile("~/Assignment 2/Sounds/Collision.wav");
    animationMusic.openFromFile("~/Assignment 2/Sounds/Anim.wav");
}

void updateTimer(int value) {
    
    if (background.getStatus() != sf::Music::Playing)
        {
            background.setVolume(20);
            background.play();
        }
    
    if (!gameState.isGameOver) {
        gameState.timeRemaining -= 1.0f;  // Decrease by 1 second
        
        // Check for game end conditions
        if (gameState.timeRemaining <= 0) {
            gameState.timeRemaining = 0;
            gameState.isGameOver = true;
            gameState.isWin = (gameState.score >= 10);
        }
        
        // Schedule next timer update if game isn't over
        if (!gameState.isGameOver) {
            glutTimerFunc(1000, updateTimer, 0);
        }
    }
    glutPostRedisplay();
}

void spawnNewJavelin() {
    // Only spawn if we need a new javelin and player doesn't have one
    if (!gameState.needNewJavelin || gameState.hasJavelin) return;
    
    float randX = JAVELIN_FIELD_MIN_X + (rand() / (float)RAND_MAX) *
                  (JAVELIN_FIELD_MAX_X - JAVELIN_FIELD_MIN_X);
    float randZ = JAVELIN_FIELD_MIN_Z + (rand() / (float)RAND_MAX) *
                  (JAVELIN_FIELD_MAX_Z - JAVELIN_FIELD_MIN_Z);
    
    currentJavelin.x = randX;
    currentJavelin.z = randZ;
    currentJavelin.active = true;
    gameState.needNewJavelin = false;
}

bool checkJavelinCollision() {
    if (!currentJavelin.active || gameState.hasJavelin) return false;
    
    float dx = (pMotionX - 5.0f) - currentJavelin.x;
    float dz = (pMotionZ - 7.0f) - currentJavelin.z;
    float distance = sqrt(dx * dx + dz * dz);
    
    if(distance<COLLISION_DISTANCE){
        if (collision.getStatus() != sf::Music::Playing)
            {
                collision.play();
            }
        
    }

    return distance < COLLISION_DISTANCE;
}

bool checkPodiumCollision() {
    
    if (!gameState.hasJavelin) return false;  // Only check if player has javelin
    
    float dx = (pMotionX - 5.0f) - podium.x;
    float dz = (pMotionZ - 7.0f) - podium.z;
    float distance = sqrt(dx * dx + dz * dz);
    
    if(distance<COLLISION_DISTANCE){
        if (collision.getStatus() != sf::Music::Playing)
            {
                collision.play();
            }
        
    }

    return distance < COLLISION_DISTANCE;
}

void drawPodium(){
    
    //Podium
    
    glPushMatrix();
    
    // Apply animations
    if (podiumAnim.isAnimating) {
        float scale = 1.0f + sin(podiumAnim.scale) * 0.2f;
        glScalef(scale, scale, scale);
    }
    
    glColor3f(0.0f, 0.0f, 0.0f);  // White color
    glPushMatrix();
    glTranslatef(0.0f, 0.5f, 0.5f);
    glutSolidCube(1);
    glColor3f(1, 1.0f, 1.0f);
    glutWireCube(1.001);
    glPopMatrix();
    
    glColor3f(0.0f, 0.0f, 0.0f);  // White color
    glPushMatrix();
    glTranslatef(0.7f, 0.0f, 0.5f);
    glutSolidCube(0.85);
    glColor3f(1, 1.0f, 1.0f);
    glutWireCube(0.851);
    glPopMatrix();
    
    glColor3f(0.0f, 0.0f, 0.0f);  // White color
    glPushMatrix();
    glTranslatef(-0.7f, 0.0f, 0.5f);
    glutSolidCube(0.75);
    glColor3f(1, 1.0f, 1.0f);
    glutWireCube(0.751);
    glPopMatrix();
    glPopMatrix();
    
}

// Function to generate points for a curved line
void generateCurveLine(Points* points, float offset) {
    for (int i = 0; i < POINTS_PER_CURVE; i++) {
        float t = (float)i / (POINTS_PER_CURVE - 1);
        points[i].x = offset + (CURVE_SPREAD * t * t);  // Quadratic curve
        points[i].z = t * FIELD_LENGTH;
    }
}

// Function to draw distance markers
void drawDistanceMarker(float distance) {
    glPushMatrix();
    glColor3f(0, 1.0f, 0);  // White color
    
    // Draw the arc
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.01f, 0.0f);  // Center point
    for (float angle = -30.0f; angle <= 30.0f; angle += 1.0f) {
        float radian = angle * M_PI / 180.0f;
        float x = distance * sin(radian);
        float z = distance * cos(radian);
        glVertex3f(x, 0.01f, z);  // Slightly above ground to avoid z-fighting
//        if(angle==-30.0f || angle==30.0f){
//            std::cout << "(" << x << ", " << z << ")";
//        }
    }
    glEnd();
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_STRIP);
    for (float angle = -30.0f; angle <= 30.0f; angle += 1.0f) {
        float radian = angle * M_PI / 180.0f;
        float x = distance * sin(radian);
        float z = distance * cos(radian);
        glVertex3f(x, 0.01f, z);  // Slightly above ground to avoid z-fighting
    }
    glEnd();
    glPopMatrix();
}

// Main function to draw the javelin field (Goal)
void drawJavelinField() {
    glPushMatrix();
    
    // Move to ground level, slightly above to avoid z-fighting
    glTranslatef(-FIELD_WIDTH/2, -2.9, -FIELD_LENGTH/2);
    
    // Enable line smoothing for better looking curves
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(2.0f);
    
    // Generate and draw the curved boundary lines
    Points leftCurve[POINTS_PER_CURVE];
    Points rightCurve[POINTS_PER_CURVE];
    generateCurveLine(leftCurve, -CURVE_SPREAD);
    generateCurveLine(rightCurve, CURVE_SPREAD);
    
    // Draw the curves
    glColor3f(1.0f, 1.0f, 1.0f);  // White color
    
    // Draw distance markers (every 10 meters)
    for (float dist = 1.0f; dist <= FIELD_LENGTH; dist += 1.0f) {
        drawDistanceMarker(dist);
    }
    
    //for connecting the sector
    glBegin(GL_LINES);
    glVertex3f(-0.5, 0, 0.866025);
    glVertex3f(-4.5, 0, 7.79423);
    glEnd();
    
    glBegin(GL_LINES);
    glVertex3f(0.5, 0, 0.866025);
    glVertex3f(4.5, 0, 7.79423);
    glEnd();
    
    
    // Draw the runway
    glColor3f(0.5f, 0.7f, 1.0f);  // Light blue color
    glBegin(GL_QUADS);
    glVertex3f(-0.5f, 0.005f, -3.0f);  // Runway start
    glVertex3f(0.5f, 0.005f, -3.0f);
    glVertex3f(0.5f, 0.005f, 0.0f);    // Runway end
    glVertex3f(-0.5f, 0.005f, 0.0f);
    glEnd();
    
    drawPodium();

    
    glColor3f(1.0f, 1.0f, 1.0f);  // White color
    glPopMatrix();
    

}

void interpolateColor(float t, float* r, float* g, float* b) {
    // Yellow (255, 215, 0) to Dark Orange (255, 140, 0)
    *r = 1.0f;                          // Red stays at 1.0
    *g = 0.843f - t * 0.293f;          // Green goes from 0.843 to 0.55
    *b = 0.0f;                          // Blue stays at 0.0
}

void drawPole(int offsetX, int offsetZ) {
    glPushMatrix();
    
    // Draw the black pole (unchanged)
    glColor3f(0, 0, 0);
    glPushMatrix();
    glTranslated(-8 + offsetX, -0.6, -5+offsetZ);
    glRotated(-90, 0, 0, 1);
    glScaled(5, 0.2, 0.2);
    glutSolidCube(1);
    glPopMatrix();
    
    // Draw the animated light fixture
    glPushMatrix();
    if (lightPoleAnim.isAnimating) {
        float r, g, b;
        // Use sine function to create smooth oscillation between 0 and 1
        float t = (sin(lightPoleAnim.colorChange) + 1.0f) * 0.5f;
        interpolateColor(t, &r, &g, &b);
        glColor3f(r, g, b);
    } else {
        glColor3ub(255, 215, 0);  // Default yellow color
    }
    
    glTranslated(-8 + offsetX, 1.73, -5+offsetZ);
    glRotated(-90, 1, 0, 0);
    glutSolidCone(0.3, 1, 20, 20);
    glutSolidSphere(0.5f, 20, 20);
    glPopMatrix();
    
    glColor3f(1, 1, 1);  // Reset color
    glPopMatrix();
}

void drawFlagPole() {
    
    
    glPushMatrix();
    
    if (flagPoleAnim.isAnimating) {
        float waveEffect = sin(flagPoleAnim.translation * 2.0f) * 0.5f;
        glTranslatef(0, waveEffect, 0);
    }
    
    
    // Draw the flag pole
    glPushMatrix();
    

    glTranslatef(6.0f, -3.0f, 4.0f);
    glRotated(-90, 0, 1, 0);
    glPushMatrix();
    
    glColor3ub(0, 0, 0);
    glBegin(GL_LINES);
    glVertex3f(0, 3, 0);
    glVertex3f(0, 0, 0);
    glEnd();
    glPopMatrix();

    // Draw the Egyptian flag
    glPushMatrix();
    // Draw the white part of the flag
    glColor3ub(255, 0, 0);
    glBegin(GL_QUADS);
    glVertex3f(0.0f, 5.0f, 0.0f);
    glVertex3f(0.0f, 6.0f, 0.0f);
    glVertex3f(-2.0f,6.0f, 0.0f);
    glVertex3f(-2.0f, 5.0f, 0.0f);
    glEnd();
    
    
    glColor3ub(255, 255, 255);
    glBegin(GL_QUADS);
    glVertex3f(0.0f, 4.0f, 0.0f);
    glVertex3f(0.0f, 5.0f, 0.0f);
    glVertex3f(-2.0f, 5.0f, 0.0f);
    glVertex3f(-2.0f, 4.0f, 0.0f);
    glEnd();
    
    glColor3ub(0, 0, 0);
    glBegin(GL_LINE_LOOP);
    glVertex3f(0.0f, 4.0f, 0.0f);
    glVertex3f(0.0f, 5.0f, 0.0f);
    glVertex3f(-2.0f, 5.0f, 0.0f);
    glVertex3f(-2.0f, 4.0f, 0.0f);
    glEnd();
    
    
    glColor3ub(255, 255, 0);
    glPushMatrix();
    

    glScalef(0.2, 0.5, 0.5);
    glTranslatef(-3.5, 4.5, 0.1);
    glBegin(GL_QUADS);
    glVertex3f(0.0f, 4.0f, 0.0f);
    glVertex3f(0.0f, 5.0f, 0.0f);
    glVertex3f(-2.0f, 5.0f, 0.0f);
    glVertex3f(-2.0f, 4.0f, 0.0f);
    glEnd();
    glPopMatrix();
    

    // Draw the black part of the flag
    glColor3ub(0, 0, 0);
    glBegin(GL_QUADS);
    glVertex3f(0.0f, 3.0f, 0.0f);
    glVertex3f(0.0f, 4.0f, 0.0f);
    glVertex3f(-2.0f, 4.0f, 0.0f);
    glVertex3f(-2.0f, 3.0f, 0.0f);
    glEnd();
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
}

void drawWall(int offsetX, float offsetZ){
    glPushMatrix();
    glColor3f(0.8 + colorChangeR, 0.8 + colorChangeG, 0.8 + colorChangeB);
    glTranslated(-7.5 + offsetX, -2.5 , -5 + offsetZ);
    glutSolidCube(1);
    glColor3f(0, 0, 0);
    glutWireCube(1.001);
    glPopMatrix();
}

void drawJavelin() {
    // Draw the shaft of the javelin
    glPushMatrix();
    glColor3f(0.6, 0.6, 0.6);  // Metallic gray color
    glTranslatef(0.0f, 0.0f, 0.0f);
    glScalef(0.05f, 0.05f, 1.5f);
    glutSolidCone(1.0f, 1.0f, 20, 20);
    glPopMatrix();

    // Draw the tip of the javelin
    glPushMatrix();
    glColor3f(0.8, 0.6, 0.2);  // Bronze color
    glTranslatef(0.0f, 0.0f, 1.5f);
    glScalef(0.1f, 0.1f, 0.5f);
    glutSolidCone(1.0f, 1.0f, 20, 20);
    glPopMatrix();

    // Draw the fins of the javelin
    glPushMatrix();
    glColor3f(0.6, 0.6, 0.6);  // Metallic gray color
    glTranslatef(0.0f, 0.0f, 1.3f);
    glScalef(0.1f, 0.1f, 0.3f);

    // Fin 1
    glPushMatrix();
    glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Fin 2
    glPushMatrix();
    glRotatef(-45.0f, 0.0f, 1.0f, 0.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Fin 3
    glPushMatrix();
    glRotatef(135.0f, 0.0f, 1.0f, 0.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Fin 4
    glPushMatrix();
    glRotatef(-135.0f, 0.0f, 1.0f, 0.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPopMatrix();
}

void drawGroundJavelin() {
    // Only draw if javelin is active and player doesn't have it
    if (!currentJavelin.active || gameState.hasJavelin) return;
    
    glPushMatrix();
    glTranslatef(currentJavelin.x, -2.7f, currentJavelin.z);
    
    if (!gameState.hasJavelin) {
        float hoverHeight = sin(javelinAnim.translation) * 0.2f;
        glTranslatef(0.0f, hoverHeight, 0.0f);
        glRotatef(javelinAnim.rotation, 0, 1, 0);
    }
    
    
    glScalef(0.5, 0.5, 0.5);
    drawJavelin();
    glPopMatrix();
}

void drawTripodAndCamera() {
    
    glPushMatrix();
     
     if (tripodAnim.isAnimating) {
         glTranslatef(sin(tripodAnim.translation) * 5.3f, 0, 0);
     }
    
    
    // Draw the tripod legs
    glPushMatrix();
    glRotated(25, 0, 1, 0);
    glColor3ub(80, 80, 80);  // Dark, metallic shade
    glTranslatef(-6.0f, -3.0f, 0.0f);

    // Left leg
    glPushMatrix();
    glBegin(GL_LINES);
    glVertex3f(-1, 0, 0);
    glVertex3f(0, 1, 0);
    glPopMatrix();
    glEnd();
    glPopMatrix();

    // Right leg
    glPushMatrix();
    glBegin(GL_LINES);
    glVertex3f(1, 0, 0);
    glVertex3f(0, 1, 0);
    glPopMatrix();
    glEnd();
    glPopMatrix();

    // Back leg
    glPushMatrix();
    glBegin(GL_LINES);
    glVertex3f(0, 0, -1);
    glVertex3f(0, 1, 0);
    glPopMatrix();
    glEnd();
    glPopMatrix();

    

    // Draw the camera body
    glPushMatrix();
    glTranslated(0, 1, 0);
    glColor3ub(40, 40, 40);  // Dark, sleek shade
    glScalef(0.3f, 0.2f, 0.5f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Draw the camera lens
    glPushMatrix();
    glTranslated(0.2, 1, 0);
    glColor3ub(150, 150, 150);  // Metallic shade
    glScalef(0.2f, 0.2f, 0.2f);
    glutSolidSphere(0.5f, 20, 20);
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
}

void drawScoreboard() {
    
    glPushMatrix();
    
    if (scoreboardAnim.isAnimating) {
        glRotatef(scoreboardAnim.rotation, 1, 0, 0);
    }
    
    
    glPushMatrix();
    glTranslated(0, 10, -10);
    glRotated(90, 1, 0, 0);
    glScaled(5, 5, 5);

    // Draw the scoreboard base
    glPushMatrix();
    glColor3f(0.6, 1, 0.6);  // Green (Back Color)
    glTranslatef(0.0f, 0.0f, 0.0f);
    glScalef(2.0f, 0.2f, 1.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Draw the scoreboard frame
    glPushMatrix();
    glColor3f(0.8, 0.8, 0.8);  // Light gray color
    glTranslatef(0.0f, 0.1f, 0.0f);
    glScalef(2.1f, 0.3f, 1.1f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Draw the scoreboard display
    glPushMatrix();
    glColor3f(0.2, 0.2, 0.2);  // Dark gray color
    glTranslatef(0.0f, 0.2f, 0.05f);
    glScalef(1.9f, 0.2f, 0.9f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Draw the scoreboard lights
    glPushMatrix();
    glColor3f(1.0, 0.0, 0.0);  // Red color
    glTranslatef(-0.9f, 0.3f, 0.1f);
    glutSolidSphere(0.05f, 10, 10);
    glTranslatef(1.8f, 0.0f, 0.0f);
    glutSolidSphere(0.05f, 10, 10);
    glPopMatrix();

    glPushMatrix();
    glColor3f(1, 1, 1);  // White color
    glRotated(-90, 1, 0, 0);
    glTranslatef(-0.78f, -0.3f, 0.51f);  // Position
    glScalef(0.001f, 0.001f, 1.0f);   // Smaller size
    char text[50];
    sprintf(text, "SCORE %d  TIME %.0f", gameState.score, gameState.timeRemaining);
    for (char *c = text; *c != '\0'; c++) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
    }
    glPopMatrix();

    glPopMatrix();
    
    glPopMatrix();
}

void drawGameEndScreen() {
    if (!gameState.isGameOver) return;

    // Clear and set up basic 2D view
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    // Set up orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Draw background
    glBegin(GL_QUADS);
    if (gameState.isWin) {
        glColor3f(0.0, 0.5, 0.0);  // Dark green for win
    } else {
        glColor3f(0.5, 0.0, 0.0);  // Dark red for lose
    }
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();

    // Draw text
    glColor3f(1.0f, 1.0f, 1.0f);  // White color
    
    glPushMatrix();
    // Position for main message
    glTranslatef(-0.4f, 0.1f, 0.0f);
    glScalef(0.0003f, 0.0003f, 0.0003f);
    
    const char* message = gameState.isWin ? "GAME WIN!" : "GAME OVER";
    for (const char* c = message; *c != '\0'; c++) {
        glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, *c);
    }
    glPopMatrix();

    // Draw score
    glPushMatrix();
    glTranslatef(-0.4f, -0.1f, 0.0f);
    glScalef(0.0003f, 0.0003f, 0.0003f);
    
    char scoreText[50];
    sprintf(scoreText, "FINAL SCORE: %d/10", gameState.score);
    for (const char* c = scoreText; *c != '\0'; c++) {
        glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, *c);
    }
    glPopMatrix();
}

void DrawFence(){
    for (int i = 0; i<16; i++) {
        drawWall(i,-0.5);
    }
    for (int i = 0; i<11; i++) {
        drawWall(-1,i);
    }
    for (int i = 0; i<11; i++) {
        drawWall(16,i);
    }
    for (int i = 0; i<18; i++) {
        if(i%2 == 0){
            drawPole(i,0);
        }
    }
    for (int i = 0; i<12; i++) {
        if(i%2 == 0){
            drawPole(0,i);
        }
    }
    for (int i = 0; i<12; i++) {
        if(i%2 == 0){
            drawPole(16,i);
        }
    }
}

void drawGround(double thickness) {
    glColor3ub(178, 104, 85);
    glPushMatrix();
    glTranslated(0, -3, 0);
    glScaled(16.0, thickness, 10.0);
    glutSolidCube(1);
    glPopMatrix();
    glColor3f(1, 1, 1);
}

void drawPlayerModel() {
    
    if (gameState.hasJavelin) {
        glPushMatrix();
        glTranslatef(-5.0f + pMotionX, -2.7f, -7.0f + pMotionZ);
        glRotatef(playerRotation, 0, 1, 0);
        glTranslatef(0.0f, 0.8f, 0.0f);
        glTranslatef(-0.22f, 0.0f, 0.0f);
        glRotated(-45, 1, 0, 0);
        glScalef(0.5, 0.5, 0.5);
        drawJavelin();
        glPopMatrix();
    }


    // Player model (rest remains the same)
    glColor3f(0, 0, 0);
    glPushMatrix();
    glTranslatef(-5.0f + pMotionX, -2.7f, -7.0f + pMotionZ);
    glRotatef(playerRotation, 0, 1, 0);
    glScaled(0.5, 0.5, 0.5);

    // Draw the head
    glColor3ub(230, 200, 188);
    glPushMatrix();
    glTranslatef(0.0f, 1.5f, 0.0f);
    glScalef(0.5f, 0.5f, 0.5f);
    glutSolidSphere(0.5f, 20, 20);
    glPopMatrix();
    // Draw the head
    glColor3ub(230, 200, 188);
    glPushMatrix();
    glTranslatef(0.0f, 1.5f, 0.0f);
    glScalef(0.5f, 0.5f, 0.5f);
    glutSolidSphere(0.5f, 20, 20);
    glPopMatrix();

    // Draw the body
    glColor3f(1, 0, 0);
    glPushMatrix();
    glTranslatef(0.0f, 0.75f, 0.0f);
    glScalef(0.6f, 1.0f, 0.4f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Draw the left arm
    glColor3ub(230, 200, 188);
    glPushMatrix();
    glTranslatef(-0.4f, 1.5f, 0.0f);
    glScalef(0.2f, 0.6f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Draw the right arm
    glColor3ub(230, 200, 188);
    glPushMatrix();
    glTranslatef(0.4f, 0.75f, 0.0f);
    glScalef(0.2f, 0.6f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Draw the left leg
    glColor3ub(230, 200, 188);
    glPushMatrix();
    glTranslatef(-0.2f, 0.0f, 0.0f);
    glScalef(0.3f, 0.8f, 0.3f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Draw the right leg
    glColor3ub(230, 200, 188);
    glPushMatrix();
    glTranslatef(0.2f, 0.0f, 0.0f);
    glScalef(0.3f, 0.8f, 0.3f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPopMatrix();
    glColor3f(1, 1, 1);
}

void setupCamera() {
    if(!preset){
        eyeX = centerX + cameraDistance * cos(cameraPitch) * sin(cameraYaw);
        eyeY = centerY + cameraDistance * sin(cameraPitch);
        eyeZ = centerZ + cameraDistance * cos(cameraPitch) * cos(cameraYaw);
    }
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(100, 640 / 480, 0.001, 100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, 0, 1, 0);
}

void mouseMotion(int x, int y) {
    int deltaX = x - lastMouseX;
    int deltaY = y - lastMouseY;

    // Adjust yaw and pitch based on mouse movement
    cameraYaw += deltaX * -0.005;   // Adjust sensitivity as needed
    cameraPitch += deltaY * 0.005;

    // Limit pitch to avoid flipping the camera
    if (cameraPitch > 1.5) cameraPitch = 1.5;
    if (cameraPitch < -1.5) cameraPitch = -1.5;

    lastMouseX = x;
    lastMouseY = y;
    glutPostRedisplay();  // Redraw the scene with the new camera position
}

void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        preset = false;
        lastMouseX = x;
        lastMouseY = y;
        glutMotionFunc(mouseMotion);  // Register motion function
    } else {
        glutMotionFunc(NULL);          // Unregister when button is released
    }
}

void setView(int view) {
    switch(view) {
        case 1: // Top View
            cameraYaw = 0.0;
            cameraPitch = M_PI_2;  // Looking straight down
            cameraDistance = 10.0;
            centerX = 0;
            centerY = 0;
            centerZ = 0;
            break;
        case 2: // Side View
            cameraYaw = 0.0;     // 90 degrees to the side
            cameraPitch = 0.0;
            cameraDistance = 10.0;
            centerX = 0;
            centerY = 0;
            centerZ = 0;
            break;
        case 3: // Front View
            cameraYaw = -M_PI_2;
            cameraPitch = 0.0;
            cameraDistance = 10;
            centerX = 0;
            centerY = 0;
            centerZ = 0;
            break;
        case 4:
            centerZ -= 0.2;
            break;
        case 5:
            centerZ+=0.2;
            break;
        case 6:
            centerX-=0.2;
            break;
        case 7:
            centerX+=0.2;
            break;
            
        case 8:
            centerY+=0.2;
            break;
        case 9:
            centerY-=0.2;
            break;
            
            
    }
    // Recalculate eye position based on new camera angles
    eyeX = centerX + cameraDistance * cos(cameraPitch) * sin(cameraYaw);
    eyeY = centerY + cameraDistance * sin(cameraPitch);
    eyeZ = centerZ + cameraDistance * cos(cameraPitch) * cos(cameraYaw);
    
    glutPostRedisplay();  // Update display
}

void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case '1': setView(1); break;
        case '2': setView(2); break;
        case '3': setView(3); break;
        //camera motion in x&z
        case 'w': setView(4); break;
        case 's': setView(5); break;
        case 'a': setView(6); break;
        case 'd': setView(7); break;
        
        //y
        case 'q': setView(8); break;
        case 'z': setView(9); break;
            
        case 'p':
            podiumAnim.isAnimating = !podiumAnim.isAnimating;
            if (animationMusic.getStatus() != sf::Music::Playing && podiumAnim.isAnimating)
                {
                    animationMusic.play();
                }
            break;
        case 'b':
            scoreboardAnim.isAnimating = !scoreboardAnim.isAnimating;
            if (animationMusic.getStatus() != sf::Music::Playing && scoreboardAnim.isAnimating)
                {
                    animationMusic.play();
                }
            break;
        case 't':
            tripodAnim.isAnimating = !tripodAnim.isAnimating;
            if (animationMusic.getStatus() != sf::Music::Playing && tripodAnim.isAnimating)
                {
                    animationMusic.play();
                }
            break;
        case 'l':
            lightPoleAnim.isAnimating = !lightPoleAnim.isAnimating;
            if (animationMusic.getStatus() != sf::Music::Playing && lightPoleAnim.isAnimating)
                {
                    animationMusic.play();
                }
            break;
        case 'f':
            flagPoleAnim.isAnimating = !flagPoleAnim.isAnimating;
            if (animationMusic.getStatus() != sf::Music::Playing && flagPoleAnim.isAnimating)
                {
                    animationMusic.play();
                }
            break;
    }
    glutPostRedisplay();
}

void Display() {
    
    
    
    if (!gameState.isGameOver) {
        setupCamera();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawScoreboard();
        DrawFence();
        glPushMatrix();
        glTranslated(5, 0, 2);
        drawTripodAndCamera();
        glPopMatrix();
        drawFlagPole();
        glPushMatrix();
        glTranslated(0.4, 0.4, 0.6);
        glRotated(45, 0, 0, 1);
        glScaled(0.08, 0.08, 0.08);
        glPopMatrix();
        
        glPushMatrix();
        glTranslated(0.6, 0.38, 0.5);
        glRotated(30, 0, 1, 0);
        //    glutSolidTeapot(0.08);
        glPopMatrix();
        
        glPushMatrix();
        glTranslated(0.25, 0.42, 0.35);
        //    glutSolidSphere(0.1, 15, 15);
        glPopMatrix();
        
        glPushMatrix();
        glTranslated(0.4, 0.0, 0.4);
        //    drawTable(0.6, 0.02, 0.02, 0.3);
        glPopMatrix();
        
        drawGround(0.02);
        
        glPushMatrix();
        glRotated(90, 0, 1, 0);
        glTranslated(5, 0, 0);
        drawJavelinField();
        drawGroundJavelin();
        drawPlayerModel();
        glPopMatrix();
        glPushMatrix();
        glRotated(90, 0, 0, 1.0);
        //    drawWall(0.02);
        glPopMatrix();
        
        glPushMatrix();
        glRotated(-90, 1.0, 0.0, 0.0);
        //    drawWall(0.02);
        glPopMatrix();
        glutIdleFunc(Anim);
        
    }
    else{
        drawGameEndScreen();
    }
    glFlush();
}

void spe(int key, int x, int y) {
    switch(key) {
        case GLUT_KEY_UP:
            isMovingRight = true;
            break;
        case GLUT_KEY_DOWN:
            isMovingLeft = true;
            break;
        case GLUT_KEY_LEFT:
            isRotatingLeft = true;
            break;
        case GLUT_KEY_RIGHT:
            isRotatingRight = true;
            break;
    }
    glutPostRedisplay();
}

void speUp(int key, int x, int y) {
    switch(key) {
        case GLUT_KEY_UP:
            isMovingRight = false;
            break;
        case GLUT_KEY_DOWN:
            isMovingLeft = false;
            break;
        case GLUT_KEY_LEFT:
            isRotatingLeft = false;
            break;
        case GLUT_KEY_RIGHT:
            isRotatingRight = false;
            break;
    }
    glutPostRedisplay();
}

void generateNewColors(){
    colorChangeR = (rand() % 100) /100.0f;
    colorChangeG = (rand() % 100) /100.0f;
    colorChangeB = (rand() % 100) /100.0f;
}

void updateColors(int value){
    generateNewColors();
    glutTimerFunc(COLOR_CHANGE_INTERVAL, updateColors, 0);
    glutPostRedisplay();
}

void Anim(){
    
    // Check if we need to spawn initial javelin
    if (gameState.needNewJavelin && !currentJavelin.active && !gameState.hasJavelin) {
        spawnNewJavelin();
        
    }
    
    // Check for javelin pickup
    if (currentJavelin.active && checkJavelinCollision()) {
        currentJavelin.active = false;
        gameState.hasJavelin = true;
    }
    
    // Check for podium scoring
    if (gameState.hasJavelin && checkPodiumCollision()) {
        gameState.hasJavelin = false;
        gameState.score++;
        gameState.needNewJavelin = true;
        javelinCount = gameState.score;  // Update display counter
    }
    
    
    if (colorChangeR >= 1){
        colorChangeR = 0;
    }
    if (colorChangeG >= 1){
        colorChangeG = 0;
    }
    if (colorChangeB >= 1){
        colorChangeB = 0;
    }
    
//    if you want continuous change of colors
    
//    switch(colorState) {
//            case 0: // Red to Yellow (increase Green)
//                colorChangeG += colorSpeed;
//                if(colorChangeG >= 0.2f) {
//                    colorChangeG = 0.2f;
//                    colorState = 1;
//                }
//                break;
//
//            case 1: // Yellow to Green (decrease Red)
//                colorChangeR -= colorSpeed;
//                if(colorChangeR <= 0.0f) {
//                    colorChangeR = 0.0f;
//                    colorState = 2;
//                }
//                break;
//
//            case 2: // Green to Cyan (increase Blue)
//                colorChangeB += colorSpeed;
//                if(colorChangeB >= 0.2f) {
//                    colorChangeB = 0.2f;
//                    colorState = 3;
//                }
//                break;
//
//            case 3: // Cyan to Blue (decrease Green)
//                colorChangeG -= colorSpeed;
//                if(colorChangeG <= 0.0f) {
//                    colorChangeG = 0.0f;
//                    colorState = 4;
//                }
//                break;
//
//            case 4: // Blue to Magenta (increase Red)
//                colorChangeR += colorSpeed;
//                if(colorChangeR >= 0.2f) {
//                    colorChangeR = 0.2f;
//                    colorState = 5;
//                }
//                break;
//
//            case 5: // Magenta to Red (decrease Blue)
//                colorChangeB -= colorSpeed;
//                if(colorChangeB <= 0.0f) {
//                    colorChangeB = 0.0f;
//                    colorState = 0;
//                }
//                break;
//        }
    
    
    
    // Update animation parameters
    if (podiumAnim.isAnimating) {
        podiumAnim.scale += SCALE_SPEED;
    }
    
    if (scoreboardAnim.isAnimating) {
        scoreboardAnim.rotation += ROTATION_SPEED_ANIM;
    }
    
    if (tripodAnim.isAnimating) {
        tripodAnim.translation += TRANSLATION_SPEED;
    }
    
    if (lightPoleAnim.isAnimating) {
        lightPoleAnim.colorChange += COLOR_CHANGE_SPEED;
    }
    
    if (flagPoleAnim.isAnimating) {
        flagPoleAnim.translation += TRANSLATION_SPEED;
    }
    
    // Always animate the javelin when it's on the ground
    if (!gameState.hasJavelin && currentJavelin.active) {
        javelinAnim.rotation += ROTATION_SPEED_ANIM;
        javelinAnim.translation += TRANSLATION_SPEED;
    }
    
    
    if (isRotatingLeft) {
        playerRotation += ROTATION_SPEED;
        if (playerRotation >= 360.0) playerRotation -= 360.0;
    }
    if (isRotatingRight) {
        playerRotation -= ROTATION_SPEED;
        if (playerRotation < 0.0) playerRotation += 360.0;
    }
    
    // Modified movement to take rotation into account
    if (isMovingRight) {
        double angleRad = playerRotation * M_PI / 180.0;
        pMotionX += pMotionSpeed * sin(angleRad);
        pMotionZ += pMotionSpeed * cos(angleRad);
    }
    if (isMovingLeft) {
        double angleRad = playerRotation * M_PI / 180.0;
        pMotionX -= pMotionSpeed * sin(angleRad);
        pMotionZ -= pMotionSpeed * cos(angleRad);
    }
    

    
    if(pMotionZ > FIELD_BOUNDARY){
        if (collision.getStatus() != sf::Music::Playing)
            {
                collision.play();
            }
        pMotionZ = FIELD_BOUNDARY;
    }
    if(pMotionZ < -0.9){
        if (collision.getStatus() != sf::Music::Playing)
            {
                collision.play();
            }
        pMotionZ = -0.9;
    }
    
    if(pMotionX < -4.7){
        if (collision.getStatus() != sf::Music::Playing)
            {
                collision.play();
            }
        pMotionX = -4.7;
    }
    if(pMotionX > 4.7){
        if (collision.getStatus() != sf::Music::Playing)
            {
                collision.play();
            }
        pMotionX = 4.7;
    }
    
    glutPostRedisplay();
}

int main(int argc, char** argv) {

    glutInit(&argc, argv);

    glutInitWindowSize(1080, 720);
    glutInitWindowPosition(0, 0);

    glutCreateWindow("Lab 5");
    glutFullScreen();
    glutDisplayFunc(Display);

    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    
    loadSounds();

    glShadeModel(GL_SMOOTH);
    glutMouseFunc(mouseButton);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(spe);
    glutSpecialUpFunc(speUp);
    glutTimerFunc(COLOR_CHANGE_INTERVAL, updateColors, 0);
    glutTimerFunc(1000, updateTimer, 0);
    glutMainLoop();
    return 0;
}
