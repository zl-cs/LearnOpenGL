#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <filesystem>
#include <time.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>
#include <thread>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

struct QueueElement {
    std::vector<unsigned char> pixelData;
    int flag;
};

std::queue<QueueElement> myQueue;

std::mutex mtx;
std::condition_variable cv;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
void write_png(std::vector<unsigned char> pixels, int i, GLint viewport[4], string type);
void write_png_area(std::vector<GLubyte>pixels, int i, GLsizei width, GLsizei height, string type);
void cube_rendering(int i,GLint width, GLint height);
void floor_rendering(int i,GLint width, GLint height);
void window_rendering(int i,GLint width, GLint height);
void check_frame_buffer(GLenum status);



// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void setupOpenGLState() {
    // configure global opengl state
    // -----------------------------
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int main()
{
    


    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);


    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    setupOpenGLState();

    // Shader screenShader("3.2.blending.vs", "3.2.blending.fs");
    Shader screenShader("/home/zhang/code/LearnOpenGL/src/4.advanced_opengl/3.4.blending_thread/5.1.framebuffers_screen.vs", "/home/zhang/code/LearnOpenGL/src/4.advanced_opengl/3.4.blending_thread/5.0.framebuffers_screen.fs");
    Shader screenShader2("/home/zhang/code/LearnOpenGL/src/4.advanced_opengl/3.4.blending_thread/5.1.framebuffers_screen.vs", "/home/zhang/code/LearnOpenGL/src/4.advanced_opengl/3.4.blending_thread/3.1.window_blending.fs");
    // Shader screenShader("3.2.blending.vs", "/home/zhang/code/LearnOpenGL/src/4.advanced_opengl/3.4.blending_thread/5.0.framebuffers_screen.fs");


    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    // screen quad VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))); 


    screenShader.use();
    screenShader.setInt("screenTexture", 0);

    
   
    // render loop
    // -----------
    unsigned int i = 0;
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        
        clock_t start,end; //定义clock_t变量
        start = clock(); 
        i += 1;
        thread cube_rendering_thread(cube_rendering,i,viewport[2],viewport[3]);
        thread floor_rendering_thread(floor_rendering,i,viewport[2],viewport[3]);
        thread window_rendering_thread(window_rendering,i,viewport[2],viewport[3]);
        cube_rendering_thread.join(); 
        floor_rendering_thread.join();
        window_rendering_thread.join();
        // cube_rendering(i,viewport[2],viewport[3]);
        // floor_rendering(shader, i,viewport[2],viewport[3]);
        int m = 0;
        
        std::map<int, std::vector<unsigned char>,std::greater<int> > myMap;
        std::vector<unsigned char> whole_pixels(viewport[2]*viewport[3]*4); // *4 for RGBA

        while (true) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] {return !myQueue.empty(); });
            QueueElement val = myQueue.front();
            myMap[val.flag] = val.pixelData;
            myQueue.pop();
            lock.unlock();
            m += 1;
            // std::cout << "Frame: " << i << " Consumer got: " << m << " Size of val: " << val.pixelData.size() << std::endl;
  
            if (m == 3) break;
        }
        // glDisable(GL_DEPTH_TEST);
        // glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for (const auto& pair : myMap) {
            std::cout << "Key: " << pair.first << std::endl;
#ifdef DEBUG
            if (pair.second.size() >= 4) {
                unsigned char alpha = pair.second[3];
                std::cout << "Alpha bit of source texture: " << static_cast<int>(alpha) << std::endl;
            } else {
                std::cout << "Not enough data for an RGBA pixel" << std::endl;
            }
 #endif
            if (pair.first == 1) {
                glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                screenShader.use();
            } else {
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                screenShader.use();
            }
            // Set pixel transfer parameters to include alpha
            glPixelStorei(GL_PACK_ALIGNMENT, 4);
            
            // Create a texture to hold the pixel data
            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport[2], viewport[3], 0, GL_RGBA, GL_UNSIGNED_BYTE, pair.second.data());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // screenShader.use();
            glBindVertexArray(quadVAO);
            glBindTexture(GL_TEXTURE_2D, texture); // use the color attachment texture as the texture of the quad plane
            glDrawArrays(GL_TRIANGLES, 0, 6);


#ifdef DEBUG
            glReadPixels(0, 0, viewport[2], viewport[3], GL_RGBA, GL_UNSIGNED_BYTE, whole_pixels.data());
            if (whole_pixels.size() >= 4) {
                unsigned char alpha = whole_pixels[3];
                std::cout << "Alpha bit of reloaded texture: " << static_cast<int>(alpha) << std::endl;
            } else {
                std::cout << "Not enough data for an RGBA pixel" << std::endl;
            }
            write_png_area(whole_pixels, i, viewport[2],viewport[3], "wholetest"+std::to_string(pair.first));
#endif
        }
        end = clock(); 
        cout << "whole time = " << double(end-start)/CLOCKS_PER_SEC << "s" << endl;  

        glReadPixels(0, 0, viewport[2],viewport[3], GL_RGBA, GL_UNSIGNED_BYTE, whole_pixels.data());
        write_png_area(whole_pixels, i, viewport[2],viewport[3], "whole");

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    // glDeleteVertexArrays(1, &cubeVAO);
    // glDeleteVertexArrays(1, &planeVAO);
    // glDeleteBuffers(1, &cubeVBO);
    // glDeleteBuffers(1, &planeVBO);

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void cube_rendering(int i,GLint width, GLint height){  //, GLFWwindow* window_main
    clock_t start_cube,end_cube;
    start_cube = clock();
    
    // Initialize GLFW
    glfwInit();

    // Set up OpenGL context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
    //GLFWwindow* windowsub = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "cube_rendering LearnOpenGL", NULL, window_main);
    GLFWwindow* windowsub = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "cube_rendering LearnOpenGL", NULL,NULL);
    if (windowsub == NULL)
    {
        std::cout << "Failed to create cube_rendering GLFW window" << std::endl;
    }
    glfwMakeContextCurrent(windowsub);

    Shader shader("3.2.blending.vs", "3.2.blending.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float cubeVertices[] = {
        // positions          // texture Coords
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    // cube VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    // load textures
    unsigned int cubeTexture = loadTexture(FileSystem::getPath("resources/textures/marble.jpg").c_str());

     
    // shader configuration
    shader.use();
    shader.setInt("texture1", 0);
    
    glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)

    // make sure we clear the framebuffer's content
    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw objects
    shader.use();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);

    // cubes
    glBindVertexArray(cubeVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cubeTexture);
    model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
    shader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);  // start rendering 
    // std::cout << "******* Draw first cude **********" << std::endl;
    //--------------------------------------------------
    std::vector<GLubyte> pixels(width * height * 4);
    glReadPixels(0,0,  width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    std::unique_lock<std::mutex> lock(mtx);
    // myQueue.push(pixels);
    // std::cout << "queue push: cube 1" << std::endl;
    // write_png_area(pixels, i, width, height, "cube1");

    //---------------------------------------------------
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
    shader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    end_cube = clock();
    std::cout << "Cube rendering time = " << double(end_cube-start_cube)/CLOCKS_PER_SEC << "s" << endl;
    // std::cout << "******* Draw second cude **********" << std::endl;
    //--------------------------------------------------
    std::fill(pixels.begin(), pixels.end(), 0);
    glReadPixels(0,0,  width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    // write to queue
    QueueElement ele;
    ele.pixelData = pixels;
    ele.flag = 2;
    myQueue.push(ele);
    lock.unlock();
    cv.notify_one();
    // std::cout <<ele.flag<< "queue push: cube 2" << std::endl;

    write_png_area(pixels, i, width, height, "cube");

    // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
    // clear all relevant buffers
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
    glClear(GL_COLOR_BUFFER_BIT);
    glfwDestroyWindow(windowsub);


}

void floor_rendering(int i,GLint width, GLint height){
    clock_t start_floor,end_floor;
    start_floor = clock();

    // Initialize GLFW
    glfwInit();

    // Set up OpenGL context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
    //GLFWwindow* windowsub = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "cube_rendering LearnOpenGL", NULL, window_main);
    GLFWwindow* windowsub = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "cube_rendering LearnOpenGL", NULL,NULL);
    if (windowsub == NULL)
    {
        std::cout << "Failed to create cube_rendering GLFW window" << std::endl;
    }
    glfwMakeContextCurrent(windowsub);

    Shader shader("3.2.blending.vs", "3.2.blending.fs");
    
    float planeVertices[] = {
        // positions          // texture Coords 
         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
         5.0f, -0.5f, -5.0f,  2.0f, 2.0f
    };

    // plane VAO
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    
    unsigned int floorTexture = loadTexture(FileSystem::getPath("resources/textures/metal.png").c_str());
    
    // shader configuration
    // --------------------
    shader.use();
    shader.setInt("texture1", 0);

    glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)

    // make sure we clear the framebuffer's content
    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw objects
    shader.use();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);

    //---------------------------------------------------
    // floor
    glBindVertexArray(planeVAO);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    end_floor = clock();
    std::cout << "Floor rendering time = " << double(end_floor-start_floor)/CLOCKS_PER_SEC << "s" << endl;
    // std::cout << "******* Draw Floor **********" << std::endl;
    //--------------------------------------------------

    std::vector<GLubyte> pixels(width * height * 4);
    glReadPixels(0,0,  width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    // write to queue 
    std::unique_lock<std::mutex> lock(mtx);
    QueueElement ele;
    ele.pixelData = pixels;
    ele.flag = 3;
    myQueue.push(ele);
    // std::cout <<ele.flag<< "queue push: floor" << std::endl;
    lock.unlock();
    cv.notify_one();
    write_png_area(pixels, i, width, height, "floor");

    // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
    // clear all relevant buffers
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
    glClear(GL_COLOR_BUFFER_BIT);
    glfwDestroyWindow(windowsub);


}

void window_rendering(int i,GLint width, GLint height){
    clock_t start_window,end_window;
    start_window = clock();

    // Initialize GLFW
    glfwInit();

    // Set up OpenGL context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
    //GLFWwindow* windowsub = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "cube_rendering LearnOpenGL", NULL, window_main);
    GLFWwindow* windowsub = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "cube_rendering LearnOpenGL", NULL,NULL);
    if (windowsub == NULL)
    {
        std::cout << "Failed to create cube_rendering GLFW window" << std::endl;
    }
    glfwMakeContextCurrent(windowsub);

    std::vector<GLubyte> pixels(width * height * 4);

    Shader shader("/home/zhang/code/LearnOpenGL/src/4.advanced_opengl/3.4.blending_thread/3.2.blending.vs", "/home/zhang/code/LearnOpenGL/src/4.advanced_opengl/3.4.blending_thread/3.2.blending.fs");
    // Shader shader("3.2.blending.vs", "/home/zhang/code/LearnOpenGL/src/4.advanced_opengl/3.4.blending_thread/3.1.window_blending.fs");
    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // build and compile shaders
    // -------------------------
    // Shader shader("3.2.blending.vs", "3.2.blending.fs");

    float transparentVertices[] = {
        // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
        0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
        0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

        0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
        1.0f,  0.5f,  0.0f,  1.0f,  0.0f
    };

    // transparent VAO
    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    unsigned int transparentTexture = loadTexture(FileSystem::getPath("resources/textures/window.png").c_str());

       // transparent window locations
    // --------------------------------
    vector<glm::vec3> windows
    {
        glm::vec3(-1.5f, 0.0f, -0.48f),
        glm::vec3( 1.5f, 0.0f, 0.51f),
        glm::vec3( 0.0f, 0.0f, 0.7f),
        glm::vec3(-0.3f, 0.0f, -2.3f),
        glm::vec3( 0.5f, 0.0f, -0.6f)
    };

    // shader configuration
    // --------------------
    shader.use();
    shader.setInt("texture1", 0);
    std::map<float, glm::vec3> sorted;
    for (unsigned int i = 0; i < windows.size(); i++)
    {
        float distance = glm::length(camera.Position - windows[i]);
        sorted[distance] = windows[i];
    }

    // render
    // ------
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw objects
    shader.use();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);

    glBindVertexArray(transparentVAO);
    glBindTexture(GL_TEXTURE_2D, transparentTexture);
    unsigned int z = 0;
    for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model, it->second);
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        z++;

    }
    end_window = clock();
    std::cout << "Window rendering time = " << double(end_window-start_window)/CLOCKS_PER_SEC << "s" << endl;

    glReadPixels(0,0,  width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    // write to queue 
    std::unique_lock<std::mutex> lock(mtx);
    QueueElement ele;
    ele.pixelData = pixels;
    ele.flag = 1;
    myQueue.push(ele);
    // std::cout <<ele.flag<< " queue push: window" << z <<std::endl;
    // if (ele.pixelData.size() >= 4) {
    //             unsigned char alpha = ele.pixelData[3];
    //             std::cout << "Alpha of window pixel: " << static_cast<int>(alpha) << std::endl;
    //         } else {
    //             std::cout << "Not enough data for an RGBA pixel" << std::endl;
    //         }
    lock.unlock();
    cv.notify_one();
    write_png_area(pixels, i, width, height, "windows");

    // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
    // clear all relevant buffers
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
    glClear(GL_COLOR_BUFFER_BIT);
    glfwDestroyWindow(windowsub);

}

//define a non return function,parameters is string
void write_png(std::vector<unsigned char> pixels, int i, GLint viewport[4], string type)
{
    if (!std::filesystem::exists("output")) {
        std::cout << "Directory 'output' does not exist" << std::endl;
        std::filesystem::create_directory("output"); 
    } 
    // else {
    //     std::cout << "Directory 'output' exists" << std::endl;
    // }
    int result = stbi_write_png(("output/" + std::to_string(i) + "_"+type+".png").c_str(), viewport[2], viewport[3], 3, pixels.data(), viewport[2] * 3);
    if (result == 0) {
        std::cout << "Failed to write PNG:" << std::to_string(i) << std::endl;
    } 
    // else {
    //     std::cout << "Successfully wrote PNG:" << std::to_string(i) <<std::endl;
    // }
    return;
}

void write_png_area(std::vector<GLubyte>pixels, int i, GLsizei width, GLsizei height, string type)
{
    if (!std::filesystem::exists("output_thread")) {
        std::cout << "Directory 'output_thread' does not exist" << std::endl;
        std::filesystem::create_directory("output_thread"); 
    } 
    // else {
    //     std::cout << "Directory 'output_thread' exists" << std::endl;
    // }
    int result = stbi_write_png(("output_thread/" + std::to_string(i) + "_"+type+".png").c_str(), width, height, 4, pixels.data(), width * 4);
    if (result == 0) {
        std::cout << "Failed to write PNG:" << std::to_string(i) << std::endl;
    } 
    // else {
    //     std::cout << "Successfully wrote PNG:" << std::to_string(i) <<std::endl;
    // }
    return;
}

void check_frame_buffer(GLenum status){
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        switch(status) {
            case GL_FRAMEBUFFER_UNDEFINED:
                std::cout << "GL_FRAMEBUFFER_UNDEFINED: The specified framebuffer is the default read or draw framebuffer, but the default framebuffer does not exist." << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                std::cout << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: Any of the framebuffer attachment points are framebuffer incomplete." << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: The framebuffer does not have at least one image attached to it." << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                std::cout << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: The value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment point(s) named by GL_DRAW_BUFFERi." << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                std::cout << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_READ_BUFFER." << std::endl;
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                std::cout << "GL_FRAMEBUFFER_UNSUPPORTED: The combination of internal formats of the attached images violates an implementation-dependent set of restrictions." << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: The value of GL_RENDERBUFFER_SAMPLES is not the same for all attached renderbuffers." << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                std::cout << "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: Any framebuffer attachment is layered, and any populated attachment is not layered, or all populated color attachments are not from textures of the same target." << std::endl;
                break;
            default:
                std::cout << "Unknown error: " << status <<std::endl;
                if(status == 0) {
                    GLenum error = glGetError();
                    std::cout << "Opengl error: " << error <<std::endl;
                    if(error != GL_NO_ERROR) {
                        std::cout << "OpenGL Error: " << error << std::endl;
                    }
                }
                break;
        }
}
}