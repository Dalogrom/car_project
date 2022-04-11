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

		//****** вычисление значения скорости и подсветки машины ******\\

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
			stayCounter--;
			if (stayCounter == 0) stayCounter = 50;
			colorValue = (float)stayCounter / 50.0f + 1.4f;
		}
		else if (stayCounter > 0) colorValue = 0;
		else colorValue = (currentSpeed / initialSpeed)/2 + 0.3;
		
		//****** отрисовка машины ******\\

		float radius = STARTING_RADIUS - (currentPosition / RADIUS_TURNING_VALUE);

		glm::mat4 transform = glm::mat4(1.0f);

		transform = glm::translate(transform, glm::vec3(0.2f, 0.0f, 0.0f));
		transform = glm::rotate(transform, (ROAD_LENGTH - currentPosition), glm::vec3(0.0f, 0.0f, 1.0f));
		transform = glm::translate(transform, glm::vec3(0.0f, radius, 0.0f));
		transform = glm::scale(transform, glm::vec3(0.1, 0.05, 0));

		GLint transformLoc = glGetUniformLocation(shaderProgram, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

		GLint coloringLoc = glGetUniformLocation(shaderProgram, "carColor");
		glUniform1f(coloringLoc, colorValue);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(glGetUniformLocation(shaderProgram, "carTexture"), 0);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	void drawCar(unsigned int shaderProgram, GLuint VAO, GLuint texture) {

		//****** вычисление значение цвета и отрисовка машины ******\\

		GLfloat colorValue = 0;

		if (isInStateOf == MovementState::BROKEN) {
			stayCounter--;
			if (stayCounter == 0) stayCounter = 50;
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
		glUniform1f(coloringLoc, colorValue);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(glGetUniformLocation(shaderProgram, "carTexture"), 0);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	Car(GLfloat startSpeed, GLfloat startSlow, GLfloat startFasten) {
		initialSpeed = startSpeed;
		currentSpeed = startSpeed;
		slowSpeed = startSlow;
		fastenSpeed = startFasten;
	};
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
		GLfloat frontCarPlace = 0;
		GLfloat backCarPlace = 0;
		GLfloat frontCarRadius = 0;
		GLfloat backCarRadius = 0;
		GLfloat distanceBetweenCars = 0;
		bool nextCarShouldStop = false;
		bool nextCarShouldSlow = false;

		for (int currentCarNumber = 0; currentCarNumber < carsQueueNumber; currentCarNumber++) {
			
			//****** организация очереди машин на дороге ******\\

			bool nextCarIsMoving = false;
			Car currentCar = carsQueue.front();
			frontCarPlace = currentCar.getCurrentPlace();
			frontCarRadius = currentCar.getCurrentRadius();

			if (!currentCar.isBroken()) {
				if (nextCarShouldStop) currentCar.setStateStop();
				else if (nextCarShouldSlow) currentCar.setStateSlow();
				else currentCar.setStateNormalMove();
			}

			carsQueue.pop();
			if (currentCarNumber + 1 < carsQueueNumber) {

				//****** проверка близости двух машин ******\\

				Car nextCar = carsQueue.front();
				if (!nextCar.isStopped()) nextCarIsMoving = true;

				backCarPlace = nextCar.getCurrentPlace();
				backCarRadius = nextCar.getCurrentRadius();
				distanceBetweenCars = frontCarPlace - backCarPlace;

				if (distanceBetweenCars < (0.35 / frontCarRadius))
					nextCarShouldSlow = true;
				else
					nextCarShouldSlow = false;

				if (distanceBetweenCars < (0.105 / frontCarRadius)) {
					if ((nextCarIsMoving && !currentCar.isBroken()) || (nextCarShouldStop && !currentCar.isBroken())) currentCar.setStateStop();
					else if (!currentCar.isStopped() && !currentCar.isSlowed())
						currentCar.setStateNormalMove();
					nextCarShouldStop = true;
				}
				else {
					nextCarShouldStop = false;
				}
			}

			currentCar.moveCar(shaderProgram, VAO, texture);

			if (currentCar.stillRiding()) {
				carsQueue.push(currentCar);
			}
			else {
				nextCarShouldSlow = false;
				nextCarShouldStop = false;
				carsFinished++;
			}
		}
		carsQueueNumber -= carsFinished;
		carsFinished = 0;
	}

	bool goWreckCar(glm::vec2 carPositionToWreck) {

		//****** поломока машины ******\\

		bool resultReached = false;
		for (int currentCarNumber = 0; currentCarNumber < carsQueue.size(); currentCarNumber++) {
			Car currentCar = carsQueue.front();
			GLfloat distanceBetweenTheCursorNCar = 0;
			if (!resultReached) {
				GLfloat currentCarPositionOnTrack = currentCar.getCurrentPlace();
				GLfloat currentCarRadius = currentCar.getCurrentRadius();
				glm::vec4 currentCarVec(0, 0, 0.0, 1.0);
				glm::vec4 currentSearchedCarVec(carPositionToWreck.x, carPositionToWreck.y, 0, 1.0f);

				glm::mat4 transform = glm::mat4(1.0f);

				transform = glm::translate(transform, glm::vec3(0.2f, 0.0f, 0.0f));
				transform = glm::rotate(transform, (ROAD_LENGTH - currentCarPositionOnTrack), glm::vec3(0.0f, 0.0f, 1.0f));
				transform = glm::translate(transform, glm::vec3(0.0f, currentCarRadius, 0.0f));

				currentCarVec = transform * currentCarVec;
				
				distanceBetweenTheCursorNCar = glm::length(currentCarVec - currentSearchedCarVec);
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

		//****** очистка дороги ******\\

		while (carsQueue.size() > 0) {
			carsQueue.pop();
		}
		carsQueueNumber = 0;
	}

	void lifeStopped(unsigned int shaderProgram, GLuint VAO, GLuint texture) {

		//****** отрисовка машин при паузе ******\\

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

	//****** обработка одного момента эксперимента в зависимости от состояния ******\\

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

			if (minTimeForCarToAppear == maxTimeForCarToAppear) {
				timeForCarToAppear = maxTimeForCarToAppear;
			}
			else {
				timeForCarToAppear = std::rand() % (maxTimeForCarToAppear - minTimeForCarToAppear) + minTimeForCarToAppear;
			}
			
			lastCarCreationTime = timePassed;
			
			if (minInitialSpeed == maxInitialSpeed) initialSpeed = maxInitialSpeed;
			else
				initialSpeed = std::rand() % ((int)maxInitialSpeed - (int)minInitialSpeed) + minInitialSpeed;

			mainRoad.addCarToTheQueue(initialSpeed, slowSpeed, fastenSpeed);
		}
	}

	bool wreckTheCarInPlace(glm::vec2 carPositionToWreck) {
		if (mainRoad.goWreckCar(carPositionToWreck)) {
			return true;
		}
		return false;
	}

	Experiment(unsigned int shaderProgram, GLuint VAO, GLuint texture) : mainRoad(initialSpeed, slowSpeed, fastenSpeed) {
		experimentVAO = VAO;
		experimentShaderProgram = shaderProgram;
		textureID = texture;
	};
};












//***********************************************************************************************//
//***********************************************************************************************//
//_________________________________________M_A_I_N_______________________________________________//


int main() {

	//****** glfw инициализация общего окна ******\\

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Polygon", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetWindowPos(window, 200, 50);

	gladLoadGL();

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	glfwSetKeyCallback(window, key_callback);

	int buttonMouseState = 0;

	//***********************************************************************************************//
	//______________________________________T_E_X_T_U_R_E_S__________________________________________//
	
	
	GLuint texture, texture2, texture3;

	//****** загрузка текстуры машины ******\\
	
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int imageWidthForCar, imageHeightForCar;
	unsigned char* storeForImages = SOIL_load_image("rsc/car_image.png", &imageWidthForCar,
												&imageHeightForCar, 0, SOIL_LOAD_RGBA);
	if (storeForImages == 0) {
		std::cout << "Failed to load image\n";
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidthForCar, imageHeightForCar,
		0, GL_RGBA, GL_UNSIGNED_BYTE, storeForImages);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(storeForImages);
	glBindTexture(GL_TEXTURE_2D, 0);

	glEnable(GL_BLEND);

	//****** загрузка текстуры фона ******\\

	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int imageWidthForBg, imageHeightForBg;
	storeForImages = SOIL_load_image("rsc/background.png", &imageWidthForBg,
		&imageHeightForBg, 0, SOIL_LOAD_RGBA);
	if (storeForImages == 0) {
		std::cout << "Failed to load image\n";
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidthForBg, imageHeightForBg,
		0, GL_RGBA, GL_UNSIGNED_BYTE, storeForImages);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(storeForImages);
	glBindTexture(GL_TEXTURE_2D, 0);

	//****** загрузка текстуры выхода тоннеля ******\\

	glGenTextures(1, &texture3);
	glBindTexture(GL_TEXTURE_2D, texture3);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int imageWidthForTunnel, imageHeightForTunnel;
	storeForImages = SOIL_load_image("rsc/tunnel.png", &imageWidthForTunnel,
		&imageHeightForTunnel, 0, SOIL_LOAD_RGBA);

	if (storeForImages == 0) {
		std::cout << "Failed to load image\n";
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidthForTunnel, imageHeightForTunnel,
		0, GL_RGBA, GL_UNSIGNED_BYTE, storeForImages);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(storeForImages);
	glBindTexture(GL_TEXTURE_2D, 0);

	//***********************************************************************************************//
	//________________________________________S_H_A_D_E_R____________________________________________//

	ShaderProgramSource shaderSource = ParseShader("rsc/Basic.shader");

	unsigned int shaderProgram = CreateShader(shaderSource.VertexSource, shaderSource.FragmentSource);
	glUseProgram(shaderProgram);
	
	//***********************************************************************************************//
	//_______________________________________V_E_R_T_E_X_E_S_________________________________________//



	GLfloat vertices[] = {
		0.5f,  0.5f, 0.0f,  1.0f, 0.0f,
		0.5f, -0.5f, 0.0f,  1.0f, 1.0f,
	   -0.5f, -0.5f, 0.0f,  0.0f, 1.0f,
	   -0.5f,  0.5f, 0.0f,  0.0f, 0.0f 
	};

	GLuint indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	GLuint VBO = 0, VAO = 0, EBO = 0;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindVertexArray(VAO);



	//***********************************************************************************************//
	//__________________________________________I_M_G_U_I____________________________________________//

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("rsc/ClassicComic.ttf", 16);
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	//***********************************************************************************************//
	//____________________________________________M_A_I_N____________________________________________//

	int deltaFrames = 0;
	int lastSecondFrameRate = 0;
	int secondsPassed = 0;
	const ImVec2 menuWindowPosition = ImVec2(0.0, 0.0);
	const ImVec2 commentaryWindowPosition = ImVec2(0.0, 575.0);
	const ImVec2 exitWindowPosition = ImVec2(610.0, 0.0);
	const ImVec2 windowPivot = ImVec2(0.0, 0.0);

	std::srand(time(NULL));

	Experiment mainExperiment(shaderProgram, VAO, texture);

	double currentTime = 0;
	double lastTime = 0;
	double delta = 0;

	GLfloat slowSpeed = 1.5;
	GLfloat fastenSpeed = 1.5;
	GLfloat minInitialSpeed = 20;
	GLfloat maxInitialSpeed = 60;
	int minTimeForCarToAppear = 3;
	int maxTimeForCarToAppear = 6;
	
	bool buttonClickedOnceFlag = false;
	bool wreckButtonClickedFlag = false;
	bool pauseButtonClickedFlag = false;
	bool renewButtonClickedFlag = false;
	
	while (!glfwWindowShouldClose(window)) {

		//****** glfw обновление окна ******\\

		glClearColor(0.05f, 0.25f, 0.45f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//****** imgui обновление окна ******\\

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//****** отрисовка заднего фона ******\\

		glm::mat4 transform = glm::mat4(1.0f);
		GLfloat colorValue = 3;
		transform = glm::scale(transform, glm::vec3(2, 2, 0));

		GLint transformLoc = glGetUniformLocation(shaderProgram, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

		GLint coloringLoc = glGetUniformLocation(shaderProgram, "carColor");
		glUniform1f(coloringLoc, colorValue);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);
		glUniform1i(glGetUniformLocation(shaderProgram, "bgTexture"), 1);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		//****** основная итерация эксперимента ******\\

		mainExperiment.setExperiment(minInitialSpeed, maxInitialSpeed, slowSpeed, minTimeForCarToAppear, maxTimeForCarToAppear);
		mainExperiment.oneProcessMomentComputation();

		//****** дополнительные отрисовки ******\\

		transform = glm::mat4(1.0f);
		colorValue = 4;
		transform = glm::translate(transform, glm::vec3(0.31f, 0.08f, 0.0f));
		transform = glm::rotate(transform, glm::radians(135.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		transform = glm::scale(transform, glm::vec3(0.17, 0.17, 0));


		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
		glUniform1f(coloringLoc, colorValue);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, texture3);
		glUniform1i(glGetUniformLocation(shaderProgram, "tunnelTexture"), 2);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		transform = glm::mat4(1.0f);
		colorValue = 4;
		transform = glm::translate(transform, glm::vec3(-0.033f, 0.614f, 0.0f));
		transform = glm::rotate(transform, glm::radians(20.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		transform = glm::scale(transform, glm::vec3(0.17, 0.17, 0));


		//transformLoc = glGetUniformLocation(shaderProgram, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

		//coloringLoc = glGetUniformLocation(shaderProgram, "carColor");
		glUniform1f(coloringLoc, colorValue);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, texture3);
		glUniform1i(glGetUniformLocation(shaderProgram, "tunnelTexture"), 2);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		//****** проверка клика по машине если нажата кнопка поломки машины ******\\

		double positX = 0;
		double positY = 0;

		glfwGetCursorPos(window, &positX, &positY);
		glm::vec2 cursorPosition(positX, positY);
		coordinateChanger(cursorPosition.x, cursorPosition.y);

		buttonMouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
		if (buttonMouseState == GLFW_PRESS && buttonClickedOnceFlag &&
			wreckButtonClickedFlag && mainExperiment.wreckTheCarInPlace(cursorPosition)) {
			wreckButtonClickedFlag = false;
		}

		if (buttonMouseState == GLFW_PRESS) buttonClickedOnceFlag = false;
		else buttonClickedOnceFlag = true; //чтобы кнопка не нажималась несколько раз

		//****** контроль количества кадров/сек, подсчёт их количества ******\\

		do {
			delta = currentTime - lastTime;
			currentTime = glfwGetTime();
		} while (delta < 0.04);
		lastTime = currentTime;
		deltaFrames++;

		//****** отрисовка меню ******\\

		ImGui::SetNextWindowPos(menuWindowPosition, ImGuiCond_Once, windowPivot);
		ImGui::SetNextWindowSize(ImVec2(200.0f, 0.0f));
		ImGui::SetNextWindowBgAlpha(0.0f);

		ImGui::Begin("Options menu", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
			
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

			if (!wreckButtonClickedFlag) {
				if (ImGui::Button("wreck car", ImVec2(100, 50))) {
					renewButtonClickedFlag = false;
					wreckButtonClickedFlag = !wreckButtonClickedFlag;
				}
			}
			else {
				if (ImGui::Button("click car", ImVec2(100, 50))) {
					renewButtonClickedFlag = false;
					wreckButtonClickedFlag = !wreckButtonClickedFlag;
				}
			};

			if (!pauseButtonClickedFlag) {
				if (ImGui::Button("pause", ImVec2(100, 50))) {
					renewButtonClickedFlag = false;
					pauseButtonClickedFlag = !pauseButtonClickedFlag;
					if (pauseButtonClickedFlag) mainExperiment.stopTheExperiment();
					else mainExperiment.continueTheExperiment();
				};
			}
			else {
				if (ImGui::Button("unpause", ImVec2(100, 50))) {
					renewButtonClickedFlag = false;
					pauseButtonClickedFlag = !pauseButtonClickedFlag;
					if (pauseButtonClickedFlag) mainExperiment.stopTheExperiment();
					else mainExperiment.continueTheExperiment();
				};
			}


			if (ImGui::Button("renew", ImVec2(100, 50))) {
				renewButtonClickedFlag = true;
				pauseButtonClickedFlag = false;
				wreckButtonClickedFlag = false;
				mainExperiment.renewTheExperiment();
			};

		ImGui::End();

		//****** отрисовка меню с комментариями ******\\

		ImGui::SetNextWindowPos(commentaryWindowPosition, ImGuiCond_Once, windowPivot);
		ImGui::SetNextWindowSize(ImVec2(650.0f, 75.0f));
		ImGui::SetNextWindowBgAlpha(0.0f);

		ImGui::Begin("commentary menu", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			if (renewButtonClickedFlag) {
				ImGui::Text("Process renewed and started again.");
			}
			else if (wreckButtonClickedFlag) {
				ImGui::Text("Click on the car to wreck it. Click on the button again to disable it.");
				ImGui::Text("Car wrecked twice is fixed and will continue to ride.");
			}
			else if (pauseButtonClickedFlag) {
				ImGui::Text("Process stopped. Click on the button again to continue.");
			}
			else ImGui::Text("Process continues.");

		ImGui::End();

		//****** отрисовка кнопка закрытия процесса ******\\

		ImGui::SetNextWindowPos(exitWindowPosition, ImGuiCond_Once, windowPivot);
		ImGui::SetNextWindowSize(ImVec2(200.0f, 0.0f));
		ImGui::SetNextWindowBgAlpha(0.0f);

		ImGui::Begin(" ", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

		if (ImGui::Button("X", ImVec2(25, 25))) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		};

		ImGui::End();

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
