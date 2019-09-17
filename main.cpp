#pragma warning(disable:4244)
#define STB_IMAGE_IMPLEMENTATION
#include <GL/stb_image.h>
#include <GL/glad/glad.h>
#include <GL/GLFW/glfw3.h>

#include <GL/glm/glm/glm.hpp>
#include <GL/glm/glm/gtc/matrix_transform.hpp>
#include <GL/glm/glm/gtc/type_ptr.hpp>

#include <GL/CS6610_Project/shader.h>
#include "Includes/GL/CS6610_Project/camera.h"
#include "Includes/GL/CS6610_Project/model.h"

#include <iostream>

//=============================================================================
// Function Prototypes
//=============================================================================
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void generatePlaneVBO();
unsigned int loadTexture(char const * path);


//=============================================================================
// Global Variables
//=============================================================================
// window attributes
const unsigned int scr_width = 800;
const unsigned int scr_height = 800;

// camera attributes
Camera camera(glm::vec3(2.0f, 8.0f, 15.0f));
double lastX = (float)scr_width / 2.0;
double lastY = (float)scr_height / 2.0;
bool firstMouse = true;

// lighting attribute
glm::vec3 lightPos(0.2f, -0.2f, 1.2f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

//XY Plane
float planeVert = 25.0f;
unsigned int planeVAO;
unsigned int planeVBO;

// timing
int rotate_object = 0;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//=============================================================================
// Main
//=============================================================================
int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(scr_width, scr_height, "Toon Shading", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	//glfwSetCursorPosCallback(window, mouse_callback);
	//glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//generate plane to place obect
	generatePlaneVBO();

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	// build and compile shaders
	// -------------------------
	Shader planeShader("VS_Plane.txt", "FS_Plane.txt");
	Shader toonShader("VS_Toon1.txt", "FS_Toon1.txt");
	Shader goochShader("VS_Gooch1.txt", "FS_Gooch1.txt");
	Shader stencilShader("VS_Stencil.txt", "FS_StencilColor.txt");

	// load models
	// -----------
	Model ourModel("teapot.obj");

	// load textures for teapot plane and normal mapping
	// ---------------------------------------------------
	unsigned int diffuseMap = loadTexture("EyesWhite.jpg");
	goochShader.setInt("diffuseMap", 0);
	std::cout << "Shader loaded successfully \n";

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // clear the stencil buffer!
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		// model/view/projection transformations
		glm::mat4 model; //value assigned later
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)scr_width / (float)scr_height, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		stencilShader.use();
		stencilShader.setMat4("matProj", projection);
		stencilShader.setMat4("matView", view);

		goochShader.use();
		goochShader.setMat4("matView", view);
		goochShader.setMat4("matProj", projection);
		goochShader.setVec3("lightPos", lightPos);
		goochShader.setVec3("lightColor", lightColor);
		goochShader.setVec3("viewPos", camera.Position);

		toonShader.use();
		toonShader.setMat4("matView", view);
		toonShader.setMat4("matProj", projection);
		toonShader.setVec3("lightPos", lightPos);
		toonShader.setVec3("lightColor", lightColor);
		toonShader.setVec3("viewPos", camera.Position);
		
		//render Plane
		planeShader.use();
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);
		model = glm::mat4(1.0f);
		planeShader.setMat4("matModel", model);
		planeShader.setMat4("matProj", projection);
		planeShader.setMat4("matView", view);
		//model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);


		//translation variables
		float x, z;
		float time_slot;
		time_slot = glfwGetTime() - planeVert *floor(glfwGetTime() / planeVert);
		glm::vec3 translate_toon_by;
		glm::vec3 translate_gooch_by;

		//=============================================================================
		// Gooch Teapot
		//============================================================================
		//1st. render pass, draw objects as normal, writing to the stencil buffer
		// --------------------------------------------------------------------
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
		glEnable(GL_DEPTH_TEST);

		//teapot
		goochShader.use();
		if (time_slot <= planeVert / 2) {
			x = time_slot;
			z = -time_slot;
		}
		else {
			x = planeVert-time_slot;
			z = -planeVert / 2;
		}
		translate_gooch_by = glm::vec3(x, -0.05f, z);
		model = glm::mat4(1.0f);
		//Enable object rotation
		if(rotate_object==1)
			model = glm::rotate(model, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		model = glm::translate(model, translate_gooch_by);
		goochShader.setMat4("matModel", model);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		ourModel.Draw(goochShader);

		// 2nd. render pass: we draw a slightly scaled versions of the objects, this time disabling stencil writing.
		// Because the stencil buffer is now filled with several 1s. The parts of the buffer that are 1 are not drawn, thus only drawing 
		// the objects' size differences, making it look like borders.
		// -----------------------------------------------------------------------------------------------------------------------------
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);

		//// teapot
		stencilShader.use();
		float scale = 1.02;
		model = glm::mat4(1.0f);
		//Enable object rotation
		if (rotate_object == 1)
			model = glm::rotate(model, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		model = glm::translate(model, translate_gooch_by);
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		stencilShader.setMat4("matModel", model);
		ourModel.Draw(stencilShader);
		//glStencilMask(0xFF);
		//glEnable(GL_DEPTH_TEST);

		//=============================================================================
		// Toon Teapot
		//=============================================================================
		//1st. render pass, draw objects as normal, writing to the stencil buffer
		// --------------------------------------------------------------------
		glClear(GL_STENCIL_BUFFER_BIT); // clear the stencil buffer!
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
		glEnable(GL_DEPTH_TEST);

		//teapot
		toonShader.use();
		if (time_slot <= planeVert / 2) {
			x = -planeVert / 2+ time_slot;
			z = -planeVert / 2 + time_slot;
		}
		else {
			x = -planeVert / 2 + time_slot;
			z = -time_slot+ planeVert / 2;
		}
		translate_toon_by = glm::vec3( x, -0.05f, z);
		model = glm::mat4(1.0f);
		//Enable object rotation
		if (rotate_object == 1)
			model = glm::rotate(model, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		model = glm::translate(model, translate_toon_by);
		toonShader.setMat4("matModel", model);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		ourModel.Draw(toonShader);

		// 2nd. render pass: we draw a slightly scaled versions of the objects, this time disabling stencil writing.
		// Because the stencil buffer is now filled with several 1s. The parts of the buffer that are 1 are not drawn, thus only drawing 
		// the objects' size differences, making it look like borders.
		// -----------------------------------------------------------------------------------------------------------------------------
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);

		//// teapot
		stencilShader.use();
		scale = 1.02;
		model = glm::mat4(1.0f);
		model = glm::translate(model, translate_toon_by);
		//Enable object rotation
		if (rotate_object == 1)
			model = glm::rotate(model, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		stencilShader.setMat4("matModel", model);
		ourModel.Draw(stencilShader);;
		glStencilMask(0xFF);
		glEnable(GL_DEPTH_TEST);

		// glfw: swap buffers and poll IO events
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}
//=============================================================================
// GENERATE XY PLANE TO PLACE OBJECT
//=============================================================================
void generatePlaneVBO()
{
	float planeVertices[] = {
		// positions            // normals         // texcoords
		planeVert, -0.5f,  planeVert,  0.0f, 1.0f, 0.0f,  planeVert,  0.0f,
		-planeVert, -0.5f,  planeVert,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-planeVert, -0.5f, -planeVert,  0.0f, 1.0f, 0.0f,   0.0f, planeVert,

		planeVert, -0.5f,  planeVert,  0.0f, 1.0f, 0.0f,  planeVert,  0.0f,
		-planeVert, -0.5f, -planeVert,  0.0f, 1.0f, 0.0f,   0.0f, planeVert,
		planeVert, -0.5f, -planeVert,  0.0f, 1.0f, 0.0f,  planeVert, planeVert
	};

	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);
}
//=============================================================================
// Load Textures
//=============================================================================
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

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
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
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}