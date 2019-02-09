#include <glad/glad.h> //include glad before everything else, glad will have all the openGL headers included already
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <list>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Cube.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

list<Cube> cubes;
list<Model> models;
unsigned int diffuseMapTextureID;
unsigned int specularMapTextureID;

Shader* cubeShader;
bool cubeButtonPressed = false;
bool modelButtonPressed = false;

unsigned int loadTexture(char const * path);

//window resize call back
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

//mouse move callback
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

//wheel scroll callback
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

//user inputs
void processInput(GLFWwindow *window);

//Camera Details
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool firstMouse = true;
float fov = 45.0f;

//My lamps position
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
glm::vec3 lightColour(1.0f, 1.0f, 1.0f);

//mouse details
float lastX = 400, lastY = 300;

//time management
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

void showFPS(GLFWwindow* window);

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //3.3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //core means ignore all the backwards compat crap before

	//build window
	GLFWwindow* window = glfwCreateWindow(800, 600, "Final Project", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	//initialise GLAD (openGL)
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		system("pause");
		return -1;
	}

	//setup openGL viewport x,y,w,h
	glViewport(0, 0, 800, 600);


	stbi_set_flip_vertically_on_load(true);

	//hide cursor but also capture it inside this window
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//NOTE: openGL viewPort coordinates range -1 to 1 on x and y axis and transform to window size. E.G viewport(0,0) is windows(400,300)
	//register callback with window resize func
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//setup mouse move callback
	glfwSetCursorPosCallback(window, mouse_callback);

	//scroll wheel callback
	glfwSetScrollCallback(window, scroll_callback);

	//Configure global opengl state
	glEnable(GL_DEPTH_TEST);//Z buffer depth testing on!

	//Using our shader class
	Shader objectLightShader("projectionVertexShader.txt", "objectLightFragmentShader.txt");
	Shader cubeLightShader("projectionVertexShader.txt", "objectLightFragmentShader1.txt");
	cubeShader = &cubeLightShader;
	Shader lightShader("modelShader.vs", "modelShader.fs");
	Shader lampShader("shader6.vs", "lampShader.fs");

	// load models
	// -----------
	Model ourModel("assets/robot.obj");
	Model ourModel1("assets/nanosuit.obj");
	
	//set cursor position
	glfwSetCursorPos(window, lastX, lastY);

	//load up our container texture
	diffuseMapTextureID = loadTexture("container2.png");
	specularMapTextureID = loadTexture("container2_specular.png");

	glm::vec3 cubePositions[] = {
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(2.0f, 5.0f, -15.0f),
	glm::vec3(-1.5f, -2.2f, -2.5f),
	glm::vec3(-3.8f, -2.0f, -12.3f),
	glm::vec3(2.4f, -0.4f, -3.5f),
	glm::vec3(-1.7f, 3.0f, -7.5f),
	glm::vec3(1.3f, -2.0f, -2.5f),
	glm::vec3(1.5f, 2.0f, -2.5f),
	glm::vec3(1.5f, 0.2f, -1.5f),
	glm::vec3(-1.3f, 1.0f, -1.5f)
	};

	for (int i = 0; i < 10; i++)
	{
		Cube cube;

		cube.pos = cubePositions[i];
		cube.shader = &cubeLightShader;
		cube.diffuseMapTextureID = diffuseMapTextureID;
		cube.specularMapTextureID = specularMapTextureID;

		//add to the list
		cubes.push_back(cube);
	}

	//setting up our lighting source
	Cube lampCube;
	lampCube.pos = glm::vec3(1.0f, 1.0f, 2.0f);
	lampCube.shader = &lampShader;

	//set up light and object colours
	glm::vec3 lightColour(1.0f, 1.0f, 1.0f);
	glm::vec3 objectColour(1.0f, 0.5f, 0.3f);

	bool lightZDirection = true;

	bool modelXDirection = true;

	ourModel.pos.x = 0;
	ourModel.pos.y = 1;
	//glfw game loop
	while (!glfwWindowShouldClose(window))
	{
		//time management
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		//set draw colour
		glClearColor(0.2f, 0.3f, 0.5f, 1.0f);
		//clear screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 view = camera.GetViewMatrix();

		//projection matrix
		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);

		//update light position
		if (lightZDirection)
			lampCube.pos.z += 5 * deltaTime;
		else
			lampCube.pos.z -= 5 * deltaTime;

		if (lampCube.pos.z < -10)
			lightZDirection = true;
		else if (lampCube.pos.z > 10)
			lightZDirection = false;
		
		//update model position
		
		if (modelXDirection)
			ourModel.pos.x += 5 * deltaTime;
		else
			ourModel.pos.x -= 5 * deltaTime;

		if (ourModel.pos.x < -5)
			modelXDirection = true;
		else if (ourModel.pos.x > 5)
			modelXDirection = false;
	
		////LIGHT/LAMP
		lampShader.use();
		lampShader.setVec3("lightColor", lightColour);

		//draw lamp
		glm::mat4 lampModel = glm::mat4(1.0f); //build identity matrix
		lampModel = glm::translate(lampModel, lampCube.pos); //setup matrix to move our cube to the lampCube's	position
		glUniformMatrix4fv(glGetUniformLocation(lampCube.shader->ID, "model"), 1, GL_FALSE, value_ptr(lampModel));
		glUniformMatrix4fv(glGetUniformLocation(lampCube.shader->ID, "view"), 1, GL_FALSE, value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(lampCube.shader->ID, "projection"), 1, GL_FALSE, value_ptr(projection));
		lampCube.draw();

		objectLightShader.use();
		glUniform3fv(glGetUniformLocation(objectLightShader.ID, "objectColour"), 1, &objectColour[0]);
		glUniform3fv(glGetUniformLocation(objectLightShader.ID, "lightColour"), 1, &lightColour[0]);
		glUniform3fv(glGetUniformLocation(objectLightShader.ID, "lightPos"), 1, &lampCube.pos[0]);
		glUniform3fv(glGetUniformLocation(objectLightShader.ID, "viewPos"), 1, &camera.Position[0]);

		cubeLightShader.use();
		glUniform3fv(glGetUniformLocation(cubeLightShader.ID, "objectColour"), 1, &objectColour[0]);
		glUniform3fv(glGetUniformLocation(cubeLightShader.ID, "lightColour"), 1, &lightColour[0]);
		glUniform3fv(glGetUniformLocation(cubeLightShader.ID, "lightPos"), 1, &lampCube.pos[0]);
		glUniform3fv(glGetUniformLocation(cubeLightShader.ID, "viewPos"), 1, &camera.Position[0]);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, ourModel.pos); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
		objectLightShader.setMat4("model", model);
		ourModel.Draw(objectLightShader);

		for (Model randomModel : models)
		{
			glm::mat4 randomModel1 = glm::mat4(1.0f);
			randomModel1 = glm::translate(randomModel1, randomModel.pos); // translate it down so it's at the center of the scene
			randomModel1 = glm::scale(randomModel1, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
			objectLightShader.setMat4("model", randomModel1);
			randomModel.Draw(objectLightShader);
		}

		glm::mat4 model1 = glm::mat4(1.0f);
		model1 = glm::translate(model1, glm::vec3(2.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		model1 = glm::scale(model1, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down
		objectLightShader.setMat4("model", model1);
		ourModel1.Draw(objectLightShader);
		//draw textured cubes
		for (Cube cube : cubes)
		{
			
			//transform local model space to world space 
			glm::mat4 cubeModel = glm::mat4(1.0f);

			//move our rectangle on x axis 
			cubeModel = glm::translate(cubeModel, cube.pos);
			//rotate on y axis
			cubeModel = glm::rotate(cubeModel, (float)(glfwGetTime()), glm::vec3(1.0f, 1.0f, 1.0f));

			//apply all these matrices onto our shader
			glUniformMatrix4fv(glGetUniformLocation(cube.shader->ID, "model"), 1, GL_FALSE, value_ptr(cubeModel));
			glUniformMatrix4fv(glGetUniformLocation(cube.shader->ID, "view"), 1, GL_FALSE, value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(cube.shader->ID, "projection"), 1, GL_FALSE, value_ptr(projection));

			//make first texture slot active(0-16 slots)
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, cube.diffuseMapTextureID);

			// set uniform value for fragmentShader's texture1
			// 1. get uniforms location 
			int texture1UniformLoacation = glGetUniformLocation(cube.shader->ID, "diffuseMap");
			// 2. set texture1 uniform to use texture in slot 0
			glUniform1i(texture1UniformLoacation, 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, cube.specularMapTextureID);
			//tell fragment shader that its texture2 variable should reference texture slot 1
			glUniform1i(glGetUniformLocation(cube.shader->ID, "specularMap"), 1);
			cube.draw();
		}

		//for (Cube cube : cubes)
		//{
		//	//transform local model space to world space 
		//	glm::mat4 cubeModel = glm::mat4(1.0f);

		//	//move our rectangle on x axis 
		//	model = translate(model, cube.pos);
		//	//rotate on y axis 
		//	model = rotate(model, (float)(glfwGetTime()), glm::vec3(1.0f, 1.0f, 1.0f));

		//	//apply all these matrices onto our shader
		//	glUniformMatrix4fv(glGetUniformLocation(cube.shader->ID, "model"), 1, GL_FALSE, value_ptr(model));
		//	glUniformMatrix4fv(glGetUniformLocation(cube.shader->ID, "view"), 1, GL_FALSE, value_ptr(view));
		//	glUniformMatrix4fv(glGetUniformLocation(cube.shader->ID, "projection"), 1, GL_FALSE, value_ptr(projection));

		//	//make first texture slot active(0-16 slots)
		//	glActiveTexture(GL_TEXTURE0);
		//	glBindTexture(GL_TEXTURE_2D, cube.diffuseMapTextureID);

		//	// set uniform value for fragmentShader's texture1
		//	// 1. get uniforms location 
		//	int texture1UniformLoacation = glGetUniformLocation(cube.shader->ID, "diffuseMap");
		//	// 2. set texture1 uniform to use texture in slot 0
		//	glUniform1i(texture1UniformLoacation, 0);

		//	glActiveTexture(GL_TEXTURE1);
		//	glBindTexture(GL_TEXTURE_2D, cube.specularMapTextureID);
		//	//tell fragment shader that its texture2 variable should reference texture slot 1
		//	glUniform1i(glGetUniformLocation(cube.shader->ID, "specularMap"), 1);
		//	cube.draw();
		//}

		//Input
		glfwPollEvents();
		//swap buffers to show completed rendered screen
		glfwSwapBuffers(window);

		showFPS(window);
	}

	//kill window propers
	glfwTerminate();

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	float cameraSpeed = 2.5f * deltaTime; // adjust accordingly
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		lightColour.r += deltaTime;
		lightColour.g += deltaTime;
		lightColour.b += deltaTime;
		if (lightColour.r > 1)
			lightColour.r = 1;
		if (lightColour.g > 1)
			lightColour.g = 1;
		if (lightColour.b > 1)
			lightColour.b = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		lightColour.r -= deltaTime;
		lightColour.g -= deltaTime;
		lightColour.b -= deltaTime;
		if (lightColour.r < 0)
			lightColour.r = 0;
		if (lightColour.g < 0)
			lightColour.g = 0;
		if (lightColour.b < 0)
			lightColour.b = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
	{
		if (!cubeButtonPressed)
		{
			cubeButtonPressed = true;
			Cube cube;

			cube.pos.x = rand() % 21 - 10;
			cube.pos.y = rand() % 21 - 10;
			cube.pos.z = rand() % 21 - 10;
			cube.shader = cubeShader;
			cube.diffuseMapTextureID = diffuseMapTextureID;
			cube.specularMapTextureID = specularMapTextureID;

			//add to the list
			cubes.push_back(cube);
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_C) != GLFW_PRESS)
	{
		cubeButtonPressed = false;
	}

	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
	{
		if (!modelButtonPressed)
		{
			modelButtonPressed = true;
			Model randomModel("assets/robot.obj");

			randomModel.pos.x = rand() % 21 - 10;
			randomModel.pos.y = rand() % 21 - 10;
			randomModel.pos.z = rand() % 21 - 10;

			//add to the list
			models.push_back(randomModel);
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_V) != GLFW_PRESS)
	{
		modelButtonPressed = false;
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
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

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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

//show frames per second
void showFPS(GLFWwindow* window)
{
	static double previousSeconds = 0.0;
	static int frameCount = 0;
	double elapsedSeconds;
	double currentSeconds = glfwGetTime();//number of seconds since start of program

	elapsedSeconds = currentSeconds - previousSeconds;
	if (elapsedSeconds > 0.25)
	{
		previousSeconds = currentSeconds;
		double fps = (double)frameCount / elapsedSeconds;
		double msPerFrame = 1000.0 / fps;

		stringstream ss;
		ss.precision(3);//3 decimal places
		ss << std::fixed << "FTS: " << fps << " Frame Time: " << msPerFrame << "(ms)   Camera x:"
			<< camera.Position.x << " y:" << camera.Position.y << " z:" << camera.Position.z << endl;

		glfwSetWindowTitle(window, ss.str().c_str());

		frameCount = 0;

	}

	frameCount++;
}