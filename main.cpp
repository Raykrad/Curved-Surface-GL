#include <iostream>
#include <cmath>
#include <cstdlib>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define MAP_WIDTH 25
#define MAP_HEIGHT 25
#define MOUNTAIN_COUNT 7
#define MOUNTAIN_RADIUS 5
#define MOUNTAIN_HEIGHT 2

typedef struct {
    float x, y, z;
} Cell;

typedef struct {
    float r, g, b;
} Color;

Cell map[MAP_WIDTH][MAP_HEIGHT];
Color mapColors[MAP_WIDTH][MAP_HEIGHT];

float rotateAngleX = 20;
float rotateAngleZ = 0;
float cameraPosition[2] = { 0,0 };
float lastX = 1280 / 2.0f;
float lastY = 720 / 2.0f;
bool firstMouse = true;

GLuint mapIndices[MAP_WIDTH - 1][MAP_HEIGHT - 1][6];
int mapIndicesCount = sizeof(mapIndices) / sizeof(GLuint);

bool rotateLeft = false;
bool rotateRight = false;
bool rotateUp = false;
bool rotateDown = false;

bool onMap(float x, float y) {
    return (x >= 0) && (x < MAP_WIDTH) && (y >= 0) && (y < MAP_HEIGHT);
}

void createMountains(int centerX, int centerY, int radius, int height) {
    for (int i = centerX - radius; i <= centerX + radius; i++) {
        for (int j = centerY - radius; j <= centerY + radius; j++) {
            if (onMap(i, j)) {
                float distance = sqrt(pow(centerX - i, 2) + pow(centerY - j, 2));
                if (distance < radius) {
                    float gradientX = (float)i / MAP_WIDTH;
                    float gradientY = (float)j / MAP_HEIGHT;
                    map[i][j].z += cos(distance / radius * (M_PI / 2)) * height;
                    mapColors[i][j].r = 1.0 - gradientX;
                    mapColors[i][j].g = 1.0 - gradientY;
                    mapColors[i][j].b = 1.0;
                }
            }
        }
    }
}

void initializeMap() {
    for (int i = 0; i < MAP_WIDTH; i++) {
        for (int j = 0; j < MAP_HEIGHT; j++) {
            float colorOffset = (rand() % 20) * 0.01;
            mapColors[i][j].r = 0.8 + colorOffset;
            mapColors[i][j].g = 0.8 + colorOffset;
            mapColors[i][j].b = 0.8 + colorOffset;

            map[i][j].x = i;
            map[i][j].y = j;
            map[i][j].z = (rand() % 10) * 0.05;
        }
    }

    for (int i = 0; i < MAP_WIDTH - 1; i++) {
        int index = i * MAP_HEIGHT;
        for (int j = 0; j < MAP_HEIGHT - 1; j++) {
            mapIndices[i][j][0] = index;
            mapIndices[i][j][1] = index + 1;
            mapIndices[i][j][2] = index + 1 + MAP_HEIGHT;

            mapIndices[i][j][3] = index + 1 + MAP_HEIGHT;
            mapIndices[i][j][4] = index + MAP_HEIGHT;
            mapIndices[i][j][5] = index;
            index++;
        }
    }

    for (int i = 0; i < MOUNTAIN_COUNT; i++) {
        createMountains(rand() % MAP_WIDTH, rand() % MAP_HEIGHT, MOUNTAIN_RADIUS, MOUNTAIN_HEIGHT);
    }
}

void renderWorld() {
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, map);
    glColorPointer(3, GL_FLOAT, 0, mapColors);
    glDrawElements(GL_TRIANGLES, mapIndicesCount, GL_UNSIGNED_INT, mapIndices);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

float getHeight(float x, float y) {
    if (!onMap(x, y)) return 0;
    int tmpX = (int)x;
    int tmpY = (int)y;

    float h1 = ((1 - (x - tmpX)) * map[tmpX][tmpY].z + (x - tmpX) * map[tmpX + 1][tmpY].z);
    float h2 = ((1 - (x - tmpX)) * map[tmpX][tmpY + 1].z + (x - tmpX) * map[tmpX + 1][tmpY + 1].z);
    return (1 - (y - tmpY)) * h1 + (y - tmpY) * h2;
}

float tmpZ = 0.0;

void moveCamera(GLFWwindow* window) {
    float sensitivity = 0.005f;

    if (rotateLeft) rotateAngleZ += 90.0f * sensitivity;
    if (rotateRight) rotateAngleZ -= 90.0f * sensitivity;
    if (rotateUp) rotateAngleX -= 45.0f * sensitivity;
    if (rotateDown) rotateAngleX += 45.0f * sensitivity;

    if (rotateAngleX > 90.0f)
        rotateAngleX = 90.0f;
    if (rotateAngleX < -90.0f)
        rotateAngleX = -90.0f;

    float tmpAngle = -rotateAngleZ / 180 * M_PI;

    float speedX = 0, speedY = 0;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        speedX += sin(tmpAngle) * 0.02;
        speedY += cos(tmpAngle) * 0.02;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        speedX -= sin(tmpAngle) * 0.02;
        speedY -= cos(tmpAngle) * 0.02;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        speedX += sin(tmpAngle - M_PI * 0.5) * 0.02;
        speedY += cos(tmpAngle - M_PI * 0.5) * 0.02;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        speedX += sin(tmpAngle + M_PI * 0.5) * 0.02;
        speedY += cos(tmpAngle + M_PI * 0.5) * 0.02;
    }

    float newX = cameraPosition[0] + speedX;
    float newY = cameraPosition[1] + speedY;
    if (onMap(newX, newY)) {
        cameraPosition[0] = newX;
        cameraPosition[1] = newY;
        tmpZ = getHeight(cameraPosition[0], cameraPosition[1]) + 1.8;
    }

    glRotatef(-rotateAngleX, 1, 0, 0);
    glRotatef(-rotateAngleZ, 0, 0, 1);
    glTranslatef(-cameraPosition[0], -cameraPosition[1], -tmpZ);
}


void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
        case GLFW_KEY_LEFT:
            rotateLeft = true;
            break;
        case GLFW_KEY_RIGHT:
            rotateRight = true;
            break;
        case GLFW_KEY_UP:
            rotateUp = true;
            break;
        case GLFW_KEY_DOWN:
            rotateDown = true;
            break;
        default:
            break;
        }
    }
    else if (action == GLFW_RELEASE) {
        switch (key) {
        case GLFW_KEY_LEFT:
            rotateLeft = false;
            break;
        case GLFW_KEY_RIGHT:
            rotateRight = false;
            break;
        case GLFW_KEY_UP:
            rotateUp = false;
            break;
        case GLFW_KEY_DOWN:
            rotateDown = false;
            break;
        default:
            break;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = lastX - xpos;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    rotateAngleZ += xoffset;
    rotateAngleX += yoffset;

    if (rotateAngleX > 90.0f)
        rotateAngleX = 90.0f;
    if (rotateAngleX < -90.0f)
        rotateAngleX = -90.0f;
}

void run() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(1);
    }

    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Curved Surface", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(1);
    }

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    glfwSetWindowMonitor(window, primaryMonitor, 0, 0, mode->width, mode->height, mode->refreshRate);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, keyCallback);

    // Отключение курсора на рабочем столе, чтобы не влиял на программу
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);

    initializeMap();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glFrustum(-1, 1, -1, 1, 1, 80);
        glEnable(GL_DEPTH_TEST);

        glClearColor(0.1f, 0.1f, 0.1f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPushMatrix();

        moveCamera(window);
        renderWorld();

        glPopMatrix();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}

int main() {
    run();
    return 0;
}
