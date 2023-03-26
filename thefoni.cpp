#include "/usr/include/glad/glad.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include "stb_image.cpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "texture.h"
#include "texture.cpp"
#include "Shaders.cpp"
#include "Shaders.h"
#include "VAO.cpp"
#include "VAO.h"
#include "VBO.cpp"
#include "VBO.h"
#include "EBO.cpp"
#include "EBO.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window, Shader shaderProgram);
//void mouseInput(GLFWwindow* window, double xPos, double yPos);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// basic camera system :
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// camera rotation
float yaw = -90.0f;
bool firstMouse = true;
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;



int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Cirno Simulator", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    // VBO/VAO

    glEnable(GL_DEPTH_TEST);
    Shader shaderProgram("vertexShader.vert", "fragmentShader.frag");

    float vertices[] = {
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Left
        -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // Right
        0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // Up
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f};

    unsigned int indices[]{
        0, 3, 2,
        0, 1, 2};

    VAO VAO1;
    VAO1.Bind();

    VBO VBO1(vertices, sizeof(vertices));

    EBO EBO1(indices, sizeof(indices));

    VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 8 * sizeof(float), (void *)0);
    VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    VAO1.LinkAttrib(VBO1, 2, 2, GL_FLOAT, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    VAO1.Unbind();
    VBO1.Unbind();
    EBO1.Unbind();

    GLuint uniID = glGetUniformLocation(shaderProgram.ID, "scale");
    GLuint tex0Uni = glGetUniformLocation(shaderProgram.ID, "tex0");

    int widthImg, heightImg, numColCh;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *bytes = stbi_load("cirno.png", &widthImg, &heightImg, &numColCh, 0);
    if (bytes == NULL)
    {
        std::cout << "Error loading tex" << std::endl;
        std::cout << stbi_failure_reason() << std::endl;
    }

    Texture cirno(GL_TEXTURE0);
    cirno.BindTex();
    cirno.ParameterTex(widthImg, heightImg, bytes);
    stbi_image_free(bytes);
    cirno.UnbindTex();

    shaderProgram.Activate();
    // glDisable(GL_DEPTH_TEST);
    // initiating the camView matrix:

    // glm stuff :
    glm::mat4 model = glm::mat4(1.0f);
    
    // code responsible for prespective rotation (0deg makes it 2d, anyting higher or lower than 0deg is 3d prespective)
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    /*glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));*/
    
    unsigned int modLoc = glGetUniformLocation(shaderProgram.ID, "model");
    glUniformMatrix4fv(modLoc, 1, GL_FALSE, glm::value_ptr(model));

    
    float velocity;
    float earthGravity = 9.81f;
    bool enableGravity = false;
    unsigned int transformLoc = glGetUniformLocation(shaderProgram.ID, "model");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window, shaderProgram);

        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view;
        view = glm::lookAt(cameraPos, (cameraPos + cameraFront), cameraUp);
        unsigned int viewLoc = glGetUniformLocation(shaderProgram.ID, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 proj;
        proj = glm::perspective(glm::radians(45.f), 800.f / 600.f, 0.1f, 100.f);
        unsigned int projLoc = glGetUniformLocation(shaderProgram.ID, "proj");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

        VAO1.Bind();
        // EVEN MORE GLM STUFF WTF???? :
        float time = 0;
        float position;
        for (unsigned int i = 0; i < 1; i++)
        {
            // gravity code (not the best)
            if (enableGravity == true) {
                std::cout << "im being enabled" << std::endl;
                velocity += earthGravity * deltaTime;
                position += velocity * deltaTime;
                model = glm::translate(model, glm::vec3(0.f, (-position * 0.1f), 0.f));
            } else {
                // time = 0;
                std::cout << "im being disabled" << std::endl;
                velocity = 0;
                position = 0;
                model = glm::translate(model, glm::vec3(0.f, 0.f, 0.f));
            }
            
            unsigned int transformLoc = glGetUniformLocation(shaderProgram.ID, "model");
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            std::cout << -velocity * 0.1f << -position * 0.1f << std::endl;
            
            // beginning of the key handling
            if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
                std::cout << "i have been accessed" << std::endl;
                enableGravity = true;
                std::cout << "i have been enabled" << std::endl;
            }
            if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
                enableGravity = false;
                std::cout << "i have been disabled" << std::endl;
            }

            // character movement
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            {
                std::cout << "I am constanly being pressed to the right!" << std::endl;
                //model = glm::rotate(model, glm::radians(9.f), glm::vec3(0.0f, 0.1f, 0.0f));
                model = glm::translate(model, glm::vec3(0.1f, 0.0f, 0.0f));
                unsigned int transformLoc = glGetUniformLocation(shaderProgram.ID, "model");
                glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model));
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                //model = glm::translate(model, glm::vec3(-0.1f, 0.0f, 0.0f));
            }
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            {

                velocity = (deltaTime) * 1.5f;
                position += (velocity * deltaTime);
                // earthGravity = 0;
                std::cout << velocity << "      " << position << std::endl;
                std::cout << "I am constanly being pressed upwards!" << std::endl;
                model = glm::translate(model, glm::vec3(0.0f, (position * 0.1f) + 0.2f, 0.0f));
                unsigned int transformLoc = glGetUniformLocation(shaderProgram.ID, "model");
                glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model));
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            {
                std::cout << "I am constanly being pressed to the left!" << std::endl;
                model = glm::translate(model, glm::vec3(-0.1f, 0.0f, 0.0f));
                unsigned int transformLoc = glGetUniformLocation(shaderProgram.ID, "model");
                glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model));
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            {
                std::cout << "I am constanly being pressed downwards!" << std::endl;
                model = glm::translate(model, glm::vec3(0.0f, -0.1f, 0.0f));
                unsigned int transformLoc = glGetUniformLocation(shaderProgram.ID, "model");
                glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model));
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
            /*code responsible for rotation
            float theta = 10.0f;
            model = glm::rotate(model, (float)glfwGetTime() * 2 * glm::radians(theta), glm::vec3(0.0f, 0.1f, 0.0f));
            //std::cout << (float)glfwGetTime() * 2 * glm::radians(theta) << std::endl;*/

            float x, y, z;
            // 2nd player code (particle generator) (aka teleporter)
            if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
                x = -3 + (rand() % 8);
                y = -3 + (rand() % 8);
                z = -3 + (rand() % 8);
                std::cout << x << y << z << std::endl;
            }
            glm::mat4 model2 = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
            unsigned int transformLoc2 = glGetUniformLocation(shaderProgram.ID, "model");
            glUniformMatrix4fv(transformLoc2, 1, GL_FALSE, glm::value_ptr(model2));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        // Size
        glUniform1f(uniID, -0.7f);

        // Texture
        cirno.BindTex();
        shaderProgram.Activate();
        // glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    VAO1.Delete();
    VBO1.Delete();
    EBO1.Delete();
    cirno.DeleteTex();
    shaderProgram.Delete();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

glm::mat4 translation(glm::mat4 model) {
    return glm::translate(model, glm::vec3(0.5f, 0.0f, 0.0f));
}
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window, Shader shaderProgram)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    const float cameraSpeed = 0.05f; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) 
        cameraPos += cameraUp * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        cameraPos -= cameraUp * cameraSpeed;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
