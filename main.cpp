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
unsigned int loadTexture(char const * path);
void renderToTexture();
void setDepthBuffer();
void setQuadPlane();


//=============================================================================
// Global Variables
//=============================================================================
// window attributes
const unsigned int scr_width = 800;
const unsigned int scr_height = 800;

const unsigned int win_width = 1920;
const unsigned int win_height = 1080;

// camera attributes
Camera camera(glm::vec3(2.0f, 8.0f, 13.5f));
double lastX = (float)scr_width / 2.0;
double lastY = (float)scr_height / 2.0;
bool firstMouse = true;

// lighting attribute
glm::vec3 lightPos(0.2f, -0.2f, 1.2f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

//Render to texture attributes
GLuint frameBuffer;
GLuint depthRenderBuffer;
GLuint colorTexBuffer;
GLuint quadVAO;
GLuint quadVBO;

// timing
int rotate_object = 1;
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
	GLFWwindow* window = glfwCreateWindow(win_width, win_height, "Stippling Shading", NULL, NULL);
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

	// configure global opengl state
	// -----------------------------
	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	// build and compile shaders
	// -------------------------
	Shader stencilShader("VS_Stencil.txt", "FS_StencilColor.txt");

	Shader stipplingShader("VS_Stipling1.txt", "FS_Stipling1.txt");
	Shader quadStipling("VS_QuadPlane.txt", "FS_QuadPlane_Stippling.txt");

	Shader goochShader("VS_Gooch1.txt", "FS_Gooch1.txt");
	Shader quadGooch("VS_QuadPlane.txt", "FS_QuadPlane_Gooch.txt");

	Shader toonShader("VS_Toon1.txt", "FS_Toon1.txt");
	Shader quadToon("VS_QuadPlane.txt", "FS_QuadPlane_Toon.txt");

	//Shader stiplingShader("VS_Stipling1.txt", "FS_Stipling1.txt");

	// load models
	// -----------
	Model ourModel("teapot.obj");

	//render to texture setup
	renderToTexture();
	setQuadPlane();

	// load textures for teapot plane and normal mapping
	// ---------------------------------------------------
	unsigned int diffuseMap = loadTexture("newspaper.jpg");
	stipplingShader.use();
	stipplingShader.setInt("diffuseMap", 0);
	goochShader.use();
	goochShader.setInt("diffuseMap", 0);
	toonShader.use();
	toonShader.setInt("diffuseMap", 0);
	quadStipling.use();
    quadStipling.setInt("colorTexBuffer", 0);
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

		// model/view/projection transformations
		glm::mat4 model; //value assigned later
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)scr_width / (float)scr_height, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		stencilShader.use();
		stencilShader.setMat4("matProj", projection);
		stencilShader.setMat4("matView", view);

		quadStipling.use();
		quadStipling.setMat4("matView", view);
		quadStipling.setMat4("matProj", projection);

		quadGooch.use();
		quadGooch.setMat4("matView", view);
		quadGooch.setMat4("matProj", projection);

		quadToon.use();
		quadToon.setMat4("matView", view);
		quadToon.setMat4("matProj", projection);

		stipplingShader.use();
		stipplingShader.setMat4("matView", view);
		stipplingShader.setMat4("matProj", projection);
		stipplingShader.setVec3("lightPos", lightPos);
		stipplingShader.setVec3("lightColor", lightColor);
		stipplingShader.setVec3("viewPos", camera.Position);

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

		//=============================================================================
		// Render to Texture- Gooch
		//=============================================================================
		//Bind to custom framebuffer for rendering to texture
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glViewport(0, 0, scr_width, scr_height);

		//=============================================================================
		// Gooch
		//=============================================================================
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // clear the stencil buffer!
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		// 1st. render pass, draw objects as normal, writing to the stencil buffer
		// --------------------------------------------------------------------
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
		glEnable(GL_DEPTH_TEST);
		//teapot
		goochShader.use();
		model = glm::mat4(1.0f);
		//Enable object rotation
		if (rotate_object == 1)
			model = glm::rotate(model, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		model = glm::translate(model, glm::vec3(-0.2f, 0.0f, 0.0f));
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
		float scale = 1.035;
		model = glm::mat4(1.0f);
		//Enable object rotation
		if (rotate_object == 1)
			model = glm::rotate(model, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		stencilShader.setMat4("matModel", model);
		ourModel.Draw(stencilShader);

		//=============================================================================
		// Render to default Frame Buffer
		//=============================================================================
		//Unbind to custom framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, scr_width, scr_height);
		glScissor(0, 0, scr_width, scr_height);
		glEnable(GL_SCISSOR_TEST);

		// clear window
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // clear the stencil buffer!
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_SCISSOR_TEST);
		//=============================================================================
		// Render QuadPlane-Begin
		//=============================================================================
		quadGooch.use();

		model = glm::mat4(1.0f);
		scale = 5.0f;
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		//model = glm::rotate(model, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		model = glm::rotate(model, glm::radians(-33.0f), glm::normalize(glm::vec3(1.0, 0.0, 0.0)));
		model = glm::rotate(model, glm::radians(7.5f), glm::normalize(glm::vec3(0.0, 1.0, 0.0)));
		model = glm::rotate(model, glm::radians(5.0f), glm::normalize(glm::vec3(0.0, 0.0, 1.0)));
		quadGooch.setMat4("matModel", model);

		//activate vertex atribute array for quad plane
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// bind texture to render unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorTexBuffer);

		//apply filtering- Bilinear Interpolation and Anisotropic
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		float anIso = 0.0f;
		glGenerateMipmap(GL_TEXTURE_2D);

		// Draw the triangles
		glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
		//=============================================================================
		// Render QuadPlane-End
		//=============================================================================

		//=============================================================================
		// Render to Texture-Toon
		//=============================================================================
		//Bind to custom framebuffer for rendering to texture
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glViewport(0, 0, scr_width, scr_height);

		//=============================================================================
		// Toon
		//=============================================================================
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // clear the stencil buffer!
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		// 1st. render pass, draw objects as normal, writing to the stencil buffer
		// --------------------------------------------------------------------
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
		glEnable(GL_DEPTH_TEST);
		//teapot
		toonShader.use();
		model = glm::mat4(1.0f);
		//Enable object rotation
		if (rotate_object == 1)
			model = glm::rotate(model, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		//model = glm::translate(model, glm::vec3(-1.0f, 1.0f, 0.0f));
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
		scale = 1.035;
		model = glm::mat4(1.0f);
		//Enable object rotation
		if (rotate_object == 1)
			model = glm::rotate(model, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		stencilShader.setMat4("matModel", model);
		ourModel.Draw(stencilShader);

		//=============================================================================
		// Render to default Frame Buffer
		//=============================================================================
		//Unbind to custom framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(1200, 0, scr_width, scr_height);
		glScissor(1200, 0, scr_width, scr_height);
		glEnable(GL_SCISSOR_TEST);

		// clear window
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // clear the stencil buffer!
		//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_SCISSOR_TEST);
		//=============================================================================
		// Render QuadPlane-Begin
		//=============================================================================
		quadToon.use();

		model = glm::mat4(1.0f);
		scale = 5.0f;
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		//model = glm::rotate(model, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		model = glm::rotate(model, glm::radians(-33.0f), glm::normalize(glm::vec3(1.0, 0.0, 0.0)));
		model = glm::rotate(model, glm::radians(7.5f), glm::normalize(glm::vec3(0.0, 1.0, 0.0)));
		model = glm::rotate(model, glm::radians(5.0f), glm::normalize(glm::vec3(0.0, 0.0, 1.0)));
		quadToon.setMat4("matModel", model);

		//activate vertex atribute array for quad plane
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// bind texture to render unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorTexBuffer);

		//apply filtering- Bilinear Interpolation and Anisotropic
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		anIso = 0.0f;
		glGenerateMipmap(GL_TEXTURE_2D);

		// Draw the triangles
		glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
		//=============================================================================
		// Render QuadPlane-End
		//=============================================================================

		//=============================================================================
		// Render to Texture- Stippling
		//=============================================================================
		//Bind to custom framebuffer for rendering to texture
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glViewport(0, 0, scr_width, scr_height);

		//=============================================================================
		// Stippling
		//=============================================================================
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // clear the stencil buffer!
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		// 1st. render pass, draw objects as normal, writing to the stencil buffer
		// --------------------------------------------------------------------
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
		glEnable(GL_DEPTH_TEST);

		//teapot
		stipplingShader.use();
		model = glm::mat4(1.0f);
		if (rotate_object == 1)
			model = glm::rotate(model, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		model = glm::translate(model, glm::vec3(-0.02f, -0.05f, 0.0f));
		stipplingShader.setMat4("matModel", model);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		ourModel.Draw(stipplingShader);

		// 2nd. render pass: we draw a slightly scaled versions of the objects, this time disabling stencil writing.
		// Because the stencil buffer is now filled with several 1s. The parts of the buffer that are 1 are not drawn, thus only drawing 
		// the objects' size differences, making it look like borders.
		// -----------------------------------------------------------------------------------------------------------------------------
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);

		//// teapot
		stencilShader.use();
		scale = 1.035;
		model = glm::mat4(1.0f);
		if (rotate_object == 1)
			model = glm::rotate(model, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		stencilShader.setMat4("matModel", model);
		ourModel.Draw(stencilShader);

		//=============================================================================
		// Render to default Frame Buffer
		//=============================================================================
		//Unbind to custom framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(600, 200, scr_width, scr_height);
		glScissor(600, 200, scr_width, scr_height);
		glEnable(GL_SCISSOR_TEST);

		// clear window
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // clear the stencil buffer!
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_SCISSOR_TEST);
		//=============================================================================
		// Render QuadPlane-Begin
		//=============================================================================
		quadStipling.use();

		model = glm::mat4(1.0f);
		scale = 5.0f;
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		//model = glm::rotate(model, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		model = glm::rotate(model, glm::radians(-33.0f), glm::normalize(glm::vec3(1.0, 0.0, 0.0)));
		model = glm::rotate(model, glm::radians(7.5f), glm::normalize(glm::vec3(0.0, 1.0, 0.0)));
		model = glm::rotate(model, glm::radians(5.0f), glm::normalize(glm::vec3(0.0, 0.0, 1.0)));
		quadStipling.setMat4("matModel", model);

		//activate vertex atribute array for quad plane
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// bind texture to render unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorTexBuffer);

		//apply filtering- Bilinear Interpolation and Anisotropic
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		anIso = 0.0f;
		glGenerateMipmap(GL_TEXTURE_2D);

		// Draw the triangles
		glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
		//=============================================================================
		// Render QuadPlane-End
		//=============================================================================

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
//=============================================================================
// RENDER TO TEXTURE
//=============================================================================
void setDepthBuffer() {
	glGenRenderbuffers(1, &depthRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, scr_width, scr_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}
//=============================================================================
// SET UP FOR RENDER TO TEXTURE
//=============================================================================
void renderToTexture() {
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	/*Texture Color Object********************************************************************************************/

	// generate color texture buffer we're going to render to
	glGenTextures(1, &colorTexBuffer);

	// bind the newly created texture
	glBindTexture(GL_TEXTURE_2D, colorTexBuffer);

	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, scr_width, scr_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	// Apply filtering
	//glGenerateMipmap(GL_TEXTURE_2D);
	/****************************************************************************************************************/

	// create the depth render buffer object
	setDepthBuffer();

	//set the color texture buffer we're going to render to
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexBuffer, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

								   // Check if framebuffer is created succesfully
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

	//unbind the framebuffer object
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
//=============================================================================
// GENERATE QUADPLANE FOR FBO
//=============================================================================
void setQuadPlane() {
	static const GLfloat quadVBOData[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
	};

	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVBOData), quadVBOData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			rotate_object = 0;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
		rotate_object = 1;
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