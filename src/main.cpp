#include <iostream>
#include <cstdlib>
#include <queue>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "shader_handle_lib.h"
#include "SOIL2.h"



GLfloat const STARTING_RADIUS = 0.65f;
GLfloat const RADIUS_TURNING_VALUE = 40;

GLfloat const ROAD_LENGTH = 0.4f;
GLfloat const PROCESS_SPEED = 0.004f;

void coordinateChanger(float &x, float &y) {
	if (x < 0.0f) x = 0.0f;
	if (y < 0.0f) y = 0.0f;
	if (x > 650.0f) x = 650.0f;
	if (y > 650.0f) y = 650.0f;
	x = - (325.0f - x) / 325.0f;
	y = (325.0f - y) / 325.0f;
}

enum class ExperimentState {
	IS_STOPPED,
	IS_RENEWED,
	IS_CONTINUING
};

enum class MovementState {
	MOVING,
	STOPPED,
	SLOWING,
	BROKEN
};


//***********************************************************************************************//
//***********************************************************************************************//
//_____________________________________C_L_A_S_S_E_S_____________________________________________//


//***********************************************************************************************//
//_________________________________________C_A_R_________________________________________________//


class Car {
private:
	GLfloat initialSpeed = 0.008;
	GLfloat currentSpeed = 0;
	GLfloat slowSpeed = 0;
	GLfloat fastenSpeed = 0;
	GLfloat currentPosition = 0;
	int stayCounter = 0;
	MovementState isInStateOf = MovementState::MOVING;
public:
	GLfloat getCurrentSpeed() { return currentSpeed; }
	GLfloat getCurrentPlace() { return currentPosition; }
	GLfloat getCurrentRadius() { return STARTING_RADIUS - (currentPosition / RADIUS_TURNING_VALUE); }
	MovementState getState() { return isInStateOf; }


	bool stillRiding() {
		if (currentPosition / 50 < ROAD_LENGTH) return true;
		return false;
	}

	bool isSlowed() {
		if (isInStateOf == MovementState::SLOWING) return true;
		return false;
	}

	bool isStopped() {
		if (currentSpeed == 0) return true;
		return false;
	}

	bool isBroken() {
		if (isInStateOf == MovementState::BROKEN) return true;
		return false;
	}

	void setStateSlow() {
		isInStateOf = MovementState::SLOWING;
	}

	void setStateNormalMove() {
		isInStateOf = MovementState::MOVING;
	}

	void setStateStop() {
		isInStateOf = MovementState::STOPPED;
		stayCounter = 50;
		currentSpeed = 0;
	}

	void setStateBreak() {
		if (isInStateOf == MovementState::BROKEN) isInStateOf = MovementState::STOPPED;
		else isInStateOf = MovementState::BROKEN;
		stayCounter = 50;
		currentSpeed = 0;
	}

	void moveCar(unsigned int shaderProgram, GLuint VAO, GLuint texture) {

		GLfloat colorValue = 0;

		if (stayCounter > 0) stayCounter--;
		else {
			if (isInStateOf == MovementState::SLOWING)
				if (currentSpeed <= 0) currentSpeed = 0;
				else currentSpeed -= slowSpeed;

			else if ((isInStateOf == MovementState::MOVING)
				&& (currentSpeed < initialSpeed))
				currentSpeed += fastenSpeed;

			currentPosition += currentSpeed;
		}
		
		if (isInStateOf == MovementState::BROKEN) {
			if (stayCounter == 0) stayCounter = 50;
			colorValue = (float)stayCounter / 50.0f + 1.4f;
		}
		else if (stayCounter > 0) colorValue = 0;
		else colorValue = (currentSpeed / initialSpeed)/2 + 0.2;
		
		//ImVec2 pos = ImGui::GetCursorPos();
		ImVec2 pos;
		 pos.x = 0.05;
		 pos.y = 0.03;
		//std::cout << colorValue << std::endl;
		//std::cout << "Imgui: " <<pos.x << ", " << pos.y << std::endl;
		glm::vec4 position(pos.x, pos.y, 0, 1.0f);
		//ImGui::SetCursorPos(pos);


		float radius = STARTING_RADIUS - (currentPosition / RADIUS_TURNING_VALUE);

		glm::mat4 transform = glm::mat4(1.0f);

		transform = glm::translate(transform, glm::vec3(0.2f, 0.0f, 0.0f));
		transform = glm::rotate(transform, (ROAD_LENGTH - currentPosition), glm::vec3(0.0f, 0.0f, 1.0f));
		transform = glm::translate(transform, glm::vec3(0.0f, radius, 0.0f));
		transform = glm::scale(transform, glm::vec3(0.1, 0.05, 0));

		position = transform * position;
		//std::cout << "CurrentCar: (" << position.x << ", " << position.y << ")" << std::endl;

		// Get matrix's uniform location and set matrix
		GLint transformLoc = glGetUniformLocation(shaderProgram, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

		GLint coloringLoc = glGetUniformLocation(shaderProgram, "carColor");
		//glUniformMatrix(coloringLoc, 1, GL_FALSE, &colorValue);
		glUniform1f(coloringLoc, colorValue);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(glGetUniformLocation(shaderProgram, "carTexture"), 0);

		glBindVertexArray(VAO);
		//glDrawArrays(GL_TRIANGLES, 0, 6);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	void drawCar(unsigned int shaderProgram, GLuint VAO, GLuint texture) {

		GLfloat colorValue = 0;

		if (isInStateOf == MovementState::BROKEN) {
			colorValue = (float)stayCounter / 50.0f + 1.4f;
		}
		else if (stayCounter > 0) colorValue = 0;
		else colorValue = (currentSpeed / initialSpeed) / 2 + 0.2;


		float radius = STARTING_RADIUS - (currentPosition / RADIUS_TURNING_VALUE);

		glm::mat4 transform = glm::mat4(1.0f);

		transform = glm::translate(transform, glm::vec3(0.2f, 0.0f, 0.0f));
		transform = glm::rotate(transform, (ROAD_LENGTH - currentPosition), glm::vec3(0.0f, 0.0f, 1.0f));
		transform = glm::translate(transform, glm::vec3(0.0f, radius, 0.0f));
		transform = glm::scale(transform, glm::vec3(0.1, 0.05, 0));

		GLint transformLoc = glGetUniformLocation(shaderProgram, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

		GLint coloringLoc = glGetUniformLocation(shaderProgram, "carColor");
		//glUniformMatrix(coloringLoc, 1, GL_FALSE, &colorValue);
		glUniform1f(coloringLoc, colorValue);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(glGetUniformLocation(shaderProgram, "carTexture"), 0);

		glBindVertexArray(VAO);
		//glDrawArrays(GL_TRIANGLES, 0, 6);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	Car(GLfloat startSpeed, GLfloat startSlow, GLfloat startFasten) {
		initialSpeed = startSpeed;
		currentSpeed = startSpeed;
		slowSpeed = startSlow;
		fastenSpeed = startFasten;
	};
	/*~Car() {
		std::cout << "I'm deleted, don't worry\n";
	};*/
};

//***********************************************************************************************//
//________________________________________R_O_A_D________________________________________________//


class Road {
private:
	std::queue<Car> carsQueue;
	int carsQueueNumber = 0;
	int carsFinished = 0;
public:

	double getFrontCarPlacement() {
		Car a = carsQueue.front();
		return a.getCurrentPlace();
	}

	void addCarToTheQueue(int initialSpeed, int slowSpeed, int fastenSpeed) {
		Car a(((GLfloat)initialSpeed)/1000, ((GLfloat)slowSpeed)/1000, ((GLfloat)fastenSpeed)/1000);
		carsQueue.push(a);
		carsQueueNumber++;
	}

	void live(unsigned int shaderProgram, GLuint VAO, GLuint texture) {
		//carsQueueNumber = carsQueue.size();
		GLfloat frontCarPlace = 0;
		GLfloat backCarPlace = 0;
		GLfloat frontCarRadius = 0;
		GLfloat backCarRadius = 0;
		GLfloat distanceBetweenCars = 0;
		bool nextCarShouldStop = false;
		bool nextCarShouldSlow = false;
		//bool nextCarIsMoving = false;

		//if (carsQueue.size() > 0) {
		for (int currentCarNumber = 0; currentCarNumber < carsQueueNumber; currentCarNumber++) {
			bool nextCarIsMoving = false;
			//std::cout << "Voof!\n";
			Car currentCar = carsQueue.front();
			frontCarPlace = currentCar.getCurrentPlace();
			frontCarRadius = currentCar.getCurrentRadius();

			if (!currentCar.isBroken()) {
				if (nextCarShouldStop) currentCar.setStateStop();
				else if (nextCarShouldSlow) currentCar.setStateSlow();
				else currentCar.setStateNormalMove();
			}
			//nextCarShouldStop = false;
			//nextCarShouldSlow = false;

			carsQueue.pop();
			if (currentCarNumber + 1 < carsQueueNumber) {
				//std::cout << "Voof2!\n";
				Car nextCar = carsQueue.front();
				if (!nextCar.isStopped()) nextCarIsMoving = true;
				//else nextCarIsMoving = false;
				backCarPlace = nextCar.getCurrentPlace();
				backCarRadius = nextCar.getCurrentRadius();
				distanceBetweenCars = frontCarPlace - backCarPlace;
				//std::cout << distanceBetweenCars << " : " << 0.11 / frontCarRadius << std::endl;
				//distanceBetweenCars = STARTING_RADIUS - (frontCarPlace - backCarPlace) / RADIUS_TURNING_VALUE;
				//distanceBetweenCars = (backCarPlace/backCarRadius) / (frontCarPlace/frontCarRadius);
				//backCarPlace = frontCarPlace;
				if (distanceBetweenCars < (0.35 / frontCarRadius))
					nextCarShouldSlow = true;
				else
					nextCarShouldSlow = false;

				if (distanceBetweenCars < (0.105 / frontCarRadius)) {
					if ((nextCarIsMoving && !currentCar.isBroken()) || nextCarShouldStop) currentCar.setStateStop();
					else if (!currentCar.isStopped() && !currentCar.isSlowed())
						currentCar.setStateNormalMove();
					nextCarShouldStop = true;
					//nextCarIsMoving = false;
				}
				else {
					nextCarShouldStop = false;
					//currentCar.setStateNormalMove();
				}
			}

			currentCar.moveCar(shaderProgram, VAO, texture);

			if (currentCar.stillRiding()) {
				//if (nextCarIsMoving && nextCarShouldStop) currentCar.setStateStop();
				//else currentCar.setStateNormalMove();
				//nextCarIsMoving = false;
				carsQueue.push(currentCar);
			}

			else {
				//delete currentCar;
				nextCarShouldSlow = false;
				nextCarShouldStop = false;
				carsFinished++;
			}
		}
		carsQueueNumber -= carsFinished;
		carsFinished = 0;
		//nextCarShouldSlow = false;
		//nextCarShouldStop = false;
	}

	/*
	 0.05f,  0.03f, 0.0f,  // Top Right
	 0.05f, -0.03f, 0.0f,  // Bottom Right
	-0.05f, -0.03f, 0.0f,  // Bottom Left
	-0.05f,  0.03f, 0.0f   // Top Left 
	*/

	bool goWreckCar(glm::vec2 carPositionToWreck) {
		bool resultReached = false;
		//GLfloat distanceBetweenTheCursorNCar = 0;
		for (int currentCarNumber = 0; currentCarNumber < carsQueue.size(); currentCarNumber++) {
			Car currentCar = carsQueue.front();
			GLfloat distanceBetweenTheCursorNCar = 0;
			if (!resultReached) {
				//Car currentCar = carsQueue.front();
				GLfloat currentCarPositionOnTrack = currentCar.getCurrentPlace();
				GLfloat currentCarRadius = currentCar.getCurrentRadius();
				//GLfloat distanceBetweenTheCursorNCar = 0;
				glm::vec4 currentCarVec(0, 0, 0.0, 1.0);
				glm::vec4 currentSearchedCarVec(carPositionToWreck.x, carPositionToWreck.y, 0, 1.0f);


				glm::mat4 transform = glm::mat4(1.0f);

				transform = glm::translate(transform, glm::vec3(0.2f, 0.0f, 0.0f));
				transform = glm::rotate(transform, (ROAD_LENGTH - currentCarPositionOnTrack), glm::vec3(0.0f, 0.0f, 1.0f));
				transform = glm::translate(transform, glm::vec3(0.0f, currentCarRadius, 0.0f));

				currentCarVec = transform * currentCarVec;
				//currentSearchedCarVec = transform * currentSearchedCarVec;

				//std::cout << "first car: (" << currentCarVec.x << ", " << currentCarVec.y << ")" << std::endl;
				//std::cout << "looking for: " << currentSearchedCarVec.x << ", " << currentSearchedCarVec.y << ")" << std::endl;
				distanceBetweenTheCursorNCar = glm::length(currentCarVec - currentSearchedCarVec);
				//std::cout << "distance between: " << distanceBetweenTheCursorNCar << std::endl;
			}
			carsQueue.pop();
			if (distanceBetweenTheCursorNCar < 0.05 && !resultReached) {
				currentCar.setStateBreak();
				carsQueue.push(currentCar);
				resultReached = true;
			}
			else carsQueue.push(currentCar);
		}
		return resultReached;
	}

	void purge() {
		while (carsQueue.size() > 0) {
			carsQueue.pop();
		}
		std::cout << "PUUURGEEEE!\n";
		carsQueueNumber = 0;
	}

	void lifeStopped(unsigned int shaderProgram, GLuint VAO, GLuint texture) {
		for (int currentCarNumber = 0; currentCarNumber < carsQueue.size(); currentCarNumber++) {
			Car currentCar = carsQueue.front();
			currentCar.drawCar(shaderProgram, VAO, texture);
			carsQueue.pop();
			carsQueue.push(currentCar);
		}
	}

	Road(GLfloat initialSpeed, GLfloat slowSpeed, GLfloat fastenSpeed) {
		Car b(((GLfloat)initialSpeed) / 1000, ((GLfloat)slowSpeed) / 1000, ((GLfloat)fastenSpeed) / 1000);
		carsQueue.push(b);
		carsQueueNumber++;
	};
	//~Road();
};

//***********************************************************************************************//
//___________________________________E_X_P_E_R_I_M_E_N_T_________________________________________//

class Experiment {
private:
	unsigned int experimentShaderProgram = 0;
	GLuint experimentVAO = 0;
	GLuint textureID = 0;
	GLfloat slowSpeed = 1.5;
	GLfloat fastenSpeed = 1.5;
	GLfloat minInitialSpeed = 20;
	GLfloat maxInitialSpeed = 60;
	//int initialSpeed = (minInitialSpeed + maxInitialSpeed)/2;
	GLfloat initialSpeed = 20;
	ExperimentState currentExperimentProcess = ExperimentState::IS_CONTINUING;
	int minTimeForCarToAppear = 3;
	int maxTimeForCarToAppear = 6;
	GLfloat timeForCarToAppear = (minTimeForCarToAppear + maxTimeForCarToAppear) / 2;
public:
	GLfloat timePassed = 0;
	GLfloat lastCarCreationTime = 0;
	Road mainRoad;

	double frontCarCoord() {
		return mainRoad.getFrontCarPlacement();
	}

	void setExperiment(GLfloat inputMinSpeed, GLfloat inputMaxSpeed, GLfloat inputSlowSpeed, int inputMinAppearTime, int inputMaxAppearTime) {
		slowSpeed = inputSlowSpeed;
		//fastenSpeed = inputSlowSpeed;
		minInitialSpeed = inputMinSpeed;
		maxInitialSpeed = inputMaxSpeed;
		minTimeForCarToAppear = inputMinAppearTime;
		maxTimeForCarToAppear = inputMaxAppearTime;
	}


	void renewTheExperiment() {
		currentExperimentProcess = ExperimentState::IS_RENEWED;
		slowSpeed = 1.5;
		fastenSpeed = 1.5;
		minInitialSpeed = 20;
		maxInitialSpeed = 60;
		initialSpeed = 20;
		minTimeForCarToAppear = 3;
		maxTimeForCarToAppear = 6;
		timeForCarToAppear = 4.5;
		timePassed = 0;
		lastCarCreationTime = 0;
	}

	void stopTheExperiment() {
		currentExperimentProcess = ExperimentState::IS_STOPPED;
	}

	void continueTheExperiment() {
		currentExperimentProcess = ExperimentState::IS_CONTINUING;
	}

	void oneProcessMomentComputation() {
		if (currentExperimentProcess == ExperimentState::IS_CONTINUING) {
			timePassed += 0.04;
			mainRoad.live(experimentShaderProgram, experimentVAO, textureID);
		}

		if (currentExperimentProcess == ExperimentState::IS_RENEWED) {
			currentExperimentProcess = ExperimentState::IS_CONTINUING;
			mainRoad.purge();
			mainRoad.addCarToTheQueue(initialSpeed, slowSpeed, fastenSpeed);
		}

		if (currentExperimentProcess == ExperimentState::IS_STOPPED) {
			mainRoad.lifeStopped(experimentShaderProgram, experimentVAO, textureID);
		}

		if ((timePassed - lastCarCreationTime > timeForCarToAppear) && 
			(currentExperimentProcess == ExperimentState::IS_CONTINUING)) {

			if (minTimeForCarToAppear == maxTimeForCarToAppear) timeForCarToAppear = maxTimeForCarToAppear;
			else
				timeForCarToAppear = std::rand() % (maxTimeForCarToAppear - minTimeForCarToAppear) + minTimeForCarToAppear;
			lastCarCreationTime = timePassed;
			
			if (minInitialSpeed == maxInitialSpeed) initialSpeed = maxInitialSpeed;
			else
				initialSpeed = std::rand() % ((int)maxInitialSpeed - (int)minInitialSpeed) + minInitialSpeed;

			mainRoad.addCarToTheQueue(initialSpeed, slowSpeed, fastenSpeed);
		}
	}

	bool wreckTheCarInPlace(glm::vec2 carPositionToWreck) {
		if (mainRoad.goWreckCar(carPositionToWreck)) {
			//std::cout << "meow";
			return true;
		}
		return false;
	}

	Experiment(unsigned int shaderProgram, GLuint VAO, GLuint texture) : mainRoad(initialSpeed, slowSpeed, fastenSpeed) {
		//mainRoad(initialSpeed, slowSpeed, fastenSpeed);
		experimentVAO = VAO;
		experimentShaderProgram = shaderProgram;
		textureID = texture;
	};
	//~Experiment();
};












//***********************************************************************************************//
//***********************************************************************************************//
//_________________________________________M_A_I_N_______________________________________________//


int main() {

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Polygon", NULL, NULL);

	if (window == NULL) {
		std::cout << "Failed to create window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	gladLoadGL();

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	// Обработка закрытия окна по нажатию кнопки Esc
	glfwSetKeyCallback(window, key_callback);
	//glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_FALSE);

	//glfwSetMouseButtonCallback(window, mouse_callback);
	int buttonMouseState = 0;
	//bool leftMouseButtonPressed = false;

	/*
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &shaderSource.VertexSource, NULL);
	glCompileShader(vertexShader);

	GLuint fragmentShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(fragmentShader, 1, &shaderSource.FragmentSource, NULL);
	glCompileShader(fragmentShader);

	GLuint shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	*/

	//***********************************************************************************************//
	//______________________________________T_E_X_T_U_R_E_S__________________________________________//
	
	
	GLuint texture, texture2;
	
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);// All upcoming GL_TEXTURE_2D operations now have effect on this texture object
    
	// Set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);	// Set texture wrapping to GL_REPEAT (usually basic wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	
	// Set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int imageWidthForCar, imageHeightForCar;
	unsigned char* storeForImages = SOIL_load_image("rsc/car_image.png", &imageWidthForCar,
												&imageHeightForCar, 0, SOIL_LOAD_RGBA);
	if (storeForImages == 0) {
		std::cout << "Voof!\n";
	}
	//glPixelStore(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidthForCar, imageHeightForCar,
		0, GL_RGBA, GL_UNSIGNED_BYTE, storeForImages);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(storeForImages);
	glBindTexture(GL_TEXTURE_2D, 0);

	glEnable(GL_BLEND);

	//////////////////////////

	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);

	// Set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);	// Set texture wrapping to GL_REPEAT (usually basic wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	// Set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int imageWidthForBg, imageHeightForBg;
	storeForImages = SOIL_load_image("rsc/background.png", &imageWidthForBg,
		&imageHeightForBg, 0, SOIL_LOAD_RGBA);
	if (storeForImages == 0) {
		std::cout << "Voof!\n";
	}
	//glPixelStore(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidthForBg, imageHeightForBg,
		0, GL_RGBA, GL_UNSIGNED_BYTE, storeForImages);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(storeForImages);
	glBindTexture(GL_TEXTURE_2D, 0);

	//***********************************************************************************************//
	//***********************************************************************************************//

	//glBindTexture(GL_TEXTURE_2D, texture);

	ShaderProgramSource shaderSource = ParseShader("rsc/Basic.shader");

	unsigned int shaderProgram = CreateShader(shaderSource.VertexSource, shaderSource.FragmentSource);
	glUseProgram(shaderProgram);
	
	//***********************************************************************************************//
	//***********************************************************************************************//



	GLfloat vertices[] = {
		0.5f,  0.5f, 0.0f,  1.0f, 0.0f, // Top Right
		0.5f, -0.5f, 0.0f,  1.0f, 1.0f, // Bottom Right
	   -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, // Bottom Left
	   -0.5f,  0.5f, 0.0f,  0.0f, 0.0f  // Top Left 
	};

	GLuint indices[] = {  // Note that we start from 0!
		0, 1, 3,  // First Triangle
		1, 2, 3   // Second Triangle
	};

	GLuint VBO = 0, VAO = 0, EBO = 0;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind

	glBindVertexArray(VAO); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO




	//***********************************************************************************************//
	//***********************************************************************************************//

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	//(void)io;
	io.Fonts->AddFontFromFileTTF("rsc/ClassicComic.ttf", 16);
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	//***********************************************************************************************//
	//***********************************************************************************************//

	int deltaFrames = 0;
	int lastSecondFrameRate = 0;
	int secondsPassed = 0;
	const ImVec2 pos = ImVec2(0.0, 0.0);
	const ImVec2 pivot = ImVec2(0.0, 0.0);
	//bool p_open = NULL;
	//glm::mat4 transform = glm::mat4(1.0f);
	GLfloat radius = 0.7f;
	GLfloat way = 0.0f;
	GLfloat speed = 0.004f;
	std::queue<GLfloat> q;
	//std::queue<int> current;
	GLfloat placeOnTheRoad = 0;
	q.push(0);
	q.push(3.125);
	q.push(6.25);
	q.push(9.125);
	q.push(12.25);
	q.push(15.5);
	int queueMax = (int)q.size();
	//
	std::srand(time(NULL));

	Experiment mainExperiment(shaderProgram, VAO, texture);



	double currentTime = 0;
	double lastTime = 0;
	double delta = 0;

	GLfloat slowSpeed = 1.5;
	GLfloat fastenSpeed = 1.5;
	GLfloat minInitialSpeed = 20;
	GLfloat maxInitialSpeed = 60;
	//int initialSpeed = (minInitialSpeed + maxInitialSpeed)/2;
	//int initialSpeed = 20;
	int minTimeForCarToAppear = 3;
	int maxTimeForCarToAppear = 6;
	//float timeForCarToAppear = (minTimeForCarToAppear + maxTimeForCarToAppear) / 2;

	bool buttonClickedOnceFlag = false;
	bool wreckButtonClickedFlag = false;
	bool pauseButtonClickedFlag = false;
	bool renewButtonClickedFlag = false;
	
	while (!glfwWindowShouldClose(window)) {

		//******

		glClearColor(0.15f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//******

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, texture);
		//glUniform1i(glGetUniformLocation(shaderProgram, "ourTexture"), 0);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


		//******
		//q.push(0);
		/*
		if (q.size() > 0) {
			for (int queueProgression = 0; queueProgression < queueMax; queueProgression++) {
				placeOnTheRoad = q.front();
				move_a_car(&placeOnTheRoad, speed, shaderProgram, VAO);
				q.pop();
				if ((placeOnTheRoad / 50) < ROAD_LENGTH) {
					q.push(placeOnTheRoad);
				}
				else queueMax--;
			}
		}
		*/

		glm::mat4 transform = glm::mat4(1.0f);
		GLfloat colorValue = 3;

		transform = glm::scale(transform, glm::vec3(2, 2, 0));

		// Get matrix's uniform location and set matrix
		GLint transformLoc = glGetUniformLocation(shaderProgram, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

		GLint coloringLoc = glGetUniformLocation(shaderProgram, "carColor");
		//glUniformMatrix(coloringLoc, 1, GL_FALSE, &colorValue);
		glUniform1f(coloringLoc, colorValue);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);
		glUniform1i(glGetUniformLocation(shaderProgram, "bgTexture"), 1);

		glBindVertexArray(VAO);
		//glDrawArrays(GL_TRIANGLES, 0, 6);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		mainExperiment.setExperiment(minInitialSpeed, maxInitialSpeed, slowSpeed, minTimeForCarToAppear, maxTimeForCarToAppear);
		//mainExperiment.continueTheExperiment();
		mainExperiment.oneProcessMomentComputation();
		double positX = 0;
		double positY = 0;

		glfwGetCursorPos(window, &positX, &positY);
		glm::vec2 posit(positX, positY);
		coordinateChanger(posit.x, posit.y);

		//glfwGetCursorPos(window, 0, 0);

		buttonMouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
		//if (buttonClickedFlag && buttonMouseState == GLFW_PRESS) buttonClickedFlag = true;
		if (buttonMouseState == GLFW_PRESS && buttonClickedOnceFlag &&
			wreckButtonClickedFlag && mainExperiment.wreckTheCarInPlace(posit)) {
			//std::cout << "SUCCEED: " << posit.x << ", " << posit.y << std::endl;
			wreckButtonClickedFlag = false;
		}

		if (buttonMouseState == GLFW_PRESS) buttonClickedOnceFlag = false;
		else buttonClickedOnceFlag = true;

		//queueMax = q.size();
		/*
		glUseProgram(shaderProgram);

		if (radius > 0.3) {
			transform = glm::mat4(1.0f);
			way += speed;
			radius -= speed / 50;

			transform = glm::rotate(transform, -way, glm::vec3(0.0f, 0.0f, 1.0f));
			transform = glm::translate(transform, glm::vec3(0.0f, radius, 0.0f));

			// Get matrix's uniform location and set matrix
			GLint transformLoc = glGetUniformLocation(shaderProgram, "transform");
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

			glBindVertexArray(VAO);
			//glDrawArrays(GL_TRIANGLES, 0, 6);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
		*/
		//******
		do {
			delta = currentTime - lastTime;
			currentTime = glfwGetTime();
		} while (delta < 0.04);
		lastTime = currentTime;
		deltaFrames++;

		ImGui::SetNextWindowPos(pos, ImGuiCond_Once, pivot);
		ImGui::SetNextWindowSize(ImVec2(200.0f, 0.0f));
		ImGui::SetNextWindowBgAlpha(0.0f);
		ImGui::Begin("Меню", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		//ImGui::Text("Меню");
		ImGui::Text("FPS: %d", lastSecondFrameRate);

		ImGui::SliderFloat("d", &slowSpeed, 0, 3);
		ImGui::Text("- deceleration");

		ImGui::SliderFloat("min v", &minInitialSpeed, 5, 100);
		ImGui::Text("^ - min speed");
		ImGui::SliderFloat("max V", &maxInitialSpeed, 5, 100);
		ImGui::Text("^ - max speed");
		if (minInitialSpeed > maxInitialSpeed) minInitialSpeed = maxInitialSpeed;

		ImGui::SliderInt("min t", &minTimeForCarToAppear, 1, 8);
		ImGui::Text("^ - min time to appear");
		ImGui::SliderInt("max T", &maxTimeForCarToAppear, 1, 8);
		ImGui::Text("^ - max time to appear");
		if (minTimeForCarToAppear > maxTimeForCarToAppear) maxTimeForCarToAppear = minTimeForCarToAppear;

		if (ImGui::Button("wreck the/ncar", ImVec2(100, 100))) {
			wreckButtonClickedFlag = !wreckButtonClickedFlag;
		};

		if (ImGui::Button("pause", ImVec2(100, 100))) {
			pauseButtonClickedFlag = !pauseButtonClickedFlag;
			if (pauseButtonClickedFlag) mainExperiment.stopTheExperiment();
			else mainExperiment.continueTheExperiment();
		};

		if (ImGui::Button("renew", ImVec2(100, 100))) {
			pauseButtonClickedFlag = false;
			wreckButtonClickedFlag = false;
			mainExperiment.renewTheExperiment();
		};
		

		ImGui::End();
		//end of interface
		//

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (((int)lastTime) >= secondsPassed) {
			secondsPassed++;
			//std::cout << secondsPassed << ": " << deltaFrames << std::endl;
			lastSecondFrameRate = deltaFrames;
			deltaFrames = 0;
		}

		//******
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Properly de-allocate all resources once they've outlived their purpose
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteProgram(shaderProgram);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
	//free(carImage);

	return 0;
}
