#include <glew.h>
#include "Shader.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <QFile>
#include <QTextStream>

using namespace std;

/**
 * 指定されたvertex shader、fragment shaderを読み込んでコンパイルし、
 * プログラムにリンクする。
 *
 * @param vertex_file		vertex shader file
 * @param fragment_file		frament shader file
 * @return					program id
 */
uint Shader::createProgram(const string& vertex_file, const string& fragment_file) {
	std::string source;
	loadTextFile(vertex_file, source);
	vertex_shader = compileShader(source, GL_VERTEX_SHADER);

	loadTextFile(fragment_file, source);
	fragment_shader = compileShader(source,GL_FRAGMENT_SHADER);

	// create program
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glBindFragDataLocation(program, 0, "outputF");
	glLinkProgram(program);
	
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint logLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		char* logText = new char[logLength];
		glGetProgramInfoLog(program, logLength, NULL, logText);

		stringstream ss;
		ss << "Error linking program:" << endl << logText << endl;
		delete [] logText;

		glDeleteProgram(program);
		throw runtime_error(ss.str());
	}

	return program;
}

uint Shader::createProgram(const string& vertex_file, const string& geometry_file, const string& fragment_file) {
	std::string source;
	loadTextFile(vertex_file, source);
	vertex_shader = compileShader(source, GL_VERTEX_SHADER);

	loadTextFile(geometry_file, source);
	geometry_shader = compileShader(source, GL_GEOMETRY_SHADER);

	loadTextFile(fragment_file, source);
	fragment_shader = compileShader(source,GL_FRAGMENT_SHADER);

	// create program
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, geometry_shader);
	glAttachShader(program, fragment_shader);
	glBindFragDataLocation(program, 0, "outputF");
	glLinkProgram(program);
	
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint logLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		char* logText = new char[logLength];
		glGetProgramInfoLog(program, logLength, NULL, logText);

		stringstream ss;
		ss << "Error linking program:" << endl << logText << endl;
		cout << "Error linking program:" << endl << logText << endl;
		delete [] logText;

		glDeleteProgram(program);
		throw runtime_error(ss.str());
	}

	return program;
}

/**
 * Load a text from a file.
 *
 * @param filename		file name
 * @param str [OUT]		the text in the file
 */
void Shader::loadTextFile(const string& filename, string& str) {
	/*
	ifstream file(filename);
	if (file.fail()) {
		stringstream ss;
		ss << "Could not open file: " << filename;
		cout << ss << endl;
		throw ss.str();
	}

	QString text;
	char line[1024];
	while (!file.eof()) {
		char line[1024];
		file.getline(line, 1024);
		text += line;
	}
	str = std::string(text.toAscii().constData());
	*/
	
	QFile file(filename.c_str());
	if (!file.open(QIODevice::ReadOnly)) {
		stringstream ss;
		ss << "Could not open file: " << filename;
		throw ss.str();
	}
	QTextStream in(&file);
	QString text;
	while (!in.atEnd()) {
		QString line = in.readLine();    
		text += line + "\n"; 
	}
	file.close();
	str = std::string(text.toUtf8().constData());
}

/**
 * compile a shader.
 *
 * @param source	shader text
 * @param mode		GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
 * @return			shader id
 */
GLuint Shader::compileShader(const string& source, GLuint mode) {
	GLenum err;
	uint shader = glCreateShader(mode);
	const char* csource = source.c_str();
	GLint length = source.length();
	glShaderSource(shader, 1, &csource, &length);//NULL);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint logLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
		char* logText = new char[logLength];
		glGetShaderInfoLog(shader, logLength, NULL, logText);

		if (mode == GL_VERTEX_SHADER) {
			cout << "Vertex shader compilation error:" << endl;
		} else if (mode == GL_GEOMETRY_SHADER) {
			cout << "Geometry shader compilation error:" << endl;
		} else {
			cout << "Fragment shader compilation error:" << endl;
		}
		cout << logText << endl;
		delete [] logText;

		glDeleteShader(shader);
		throw logText;
	}

	return shader;
}
