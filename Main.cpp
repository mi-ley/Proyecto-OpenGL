#include"Model.h"
#include"glm/gtc/matrix_transform.hpp"


const unsigned int width = 2000;
const unsigned int height = 1800;


int main()
{
	// Initialize GLFW
	glfwInit();

	// Tell GLFW what version of OpenGL we are using 
	// In this case we are using OpenGL 3.3
	glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Tell GLFW we are using the CORE profile
	// So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a GLFWwindow object of 800 by 800 pixels, naming it "YoutubeOpenGL"
	GLFWwindow* window = glfwCreateWindow(width, height, "OpenGL", NULL, NULL);
	// Error check if the window fails to create
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// Introduce the window into the current context
	glfwMakeContextCurrent(window);

	//Load GLAD so it configures OpenGL
	gladLoadGL();
	// Specify the viewport of OpenGL in the Window
	// In this case the viewport goes from x = 0, y = 0, to x = 800, y = 800
	glViewport(0, 0, width, height);





	// Generates Shader object using shaders default.vert and default.frag
	Shader shaderProgram("default.vert", "default.frag");

	// Take care of all the light related things
	glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.5f);
	glm::mat4 lightModel = glm::mat4(1.0f);
	lightModel = glm::translate(lightModel, lightPos);

	shaderProgram.Activate();
	glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);





	// Enables the Depth Buffer
	glEnable(GL_DEPTH_TEST);

	// Creates camera object
	Camera camera(width, height, glm::vec3(0.0f, -15.0f, 150.0f));

	// Load in a model
	
	Model room("room/scene.gltf", /* rotacion */ 0.0f, 0.0f, 90.0f, /* traslacion */ 0.0f, 0.0, 0.0f, /* Escalacion */ 18.0f, 8.0f, 8.0f);
	Model Personaje("personaje/scene.gltf", /* rotacion */ 0.0f, 0.0f, 90.0f, /* traslacion */ 0.0f, 8.0, 0.0f, /* Escalacion */ 1.0f, 1.0f, 1.0f);
	//Model Avocado("Avocado/scene.gltf", /* rotacion */ 0.0f, 0.0f, 0.0f, /* traslacion */ 0.0f, 8.0, 0.0f, /* Escalacion */ 100.0f, 100.0f, 100.0f);


	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 bellsprooutModel = glm::mat4(1.0f);

	view = glm::translate(view, glm::vec3(0.0f, -1.0f, 0.0f));
	//
	//

	// Main while loop
	while (!glfwWindowShouldClose(window))
	{
		// Specify the color of the background
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		// Clean the back buffer and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Handles camera inputs
		camera.Inputs(window);
		// Updates and exports the camera matrix to the Vertex Shader
		camera.updateMatrix(120.0f, 0.1f, 1000.0f);

		// Draw a model
		//bellsproout.Draw(shaderProgram, camera);

		
		room.Draw(shaderProgram, camera);
	    Personaje.Draw(shaderProgram, camera);
		


		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);
		// Take care of all GLFW events
		glfwPollEvents();
	}



	// Delete all the objects we've created
	shaderProgram.Delete();
	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();
	return 0;
}