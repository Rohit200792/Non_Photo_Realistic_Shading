#pragma once
//created by Rohit Singh
void loadShaders(const char* VShader, const char* FShader, GLuint &shaderProg);

char* vertexShaderSource;
char* fragmentShaderSource;

//=============================================================================
// LOAD SHADERS
//=============================================================================
void loadShaders(const char* VShader, const char* FShader, GLuint &shaderP) {

	//reading vertex shader source code from file
	std::ifstream in1(VShader);
	std::string contents1((std::istreambuf_iterator<char>(in1)),
		std::istreambuf_iterator<char>());
	vertexShaderSource = (char *)contents1.c_str();

	//reading fragment shader source code from file
	std::ifstream in2(FShader);
	std::string contents2((std::istreambuf_iterator<char>(in2)),
		std::istreambuf_iterator<char>());
	fragmentShaderSource = (char *)contents2.c_str();

	/*
	std::ifstream ift1(VShader, std::fstream::in);
	std::string contents1;
	contents1 = ift1.get();
	while (ift1.good()) {
		contents1 += ift1.get();
	}
	vertexShaderSource = (char*)&contents1;
	std::ifstream ift2(FShader, std::fstream::in);
	std::string contents2;
	contents2 = ift2.get();
	while (ift2.good()) {
		contents2 += ift2.get();
	}
	fragmentShaderSource = (char*)&contents2;
	*/

	//setting up vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	GLint success;
	GLchar infolog[512];

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infolog);
		std::cout << "ERROR::SHADER::VERTEX::" << VShader << "::COMPILATION_FAILED\n" << infolog << "\n";
	}

	//setting up fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infolog);
		std::cout << "ERROR::SHADER::FRAGMENT::" << FShader << "::COMPILATION_FAILED\n" << infolog << "\n";
	}

	shaderP = glCreateProgram();
	glAttachShader(shaderP, vertexShader);
	glAttachShader(shaderP, fragmentShader);

	//glBindAttribLocation(shaderP, 0, "position");
	//glBindAttribLocation(shaderP, 1, "color");
	//glBindAttribLocation(shaderP, 2, "normal");
	//glBindAttribLocation(shaderP, 2, "uv");

	glLinkProgram(shaderP);

	glGetShaderiv(fragmentShader, GL_LINK_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infolog);
		std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n" << infolog << "\n";
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}
