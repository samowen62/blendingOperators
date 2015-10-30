#include "mesh.h"

#define BUFFER_OFFSET(i) ((void*)(i))

std::string loadFile(const char *fname)
{
	std::ifstream file(fname);
	if(!file.is_open())
	{
		cout << "Unable to open file " << fname << endl;
		exit(1);
	}
 
	std::stringstream fileData;
	fileData << file.rdbuf();
	file.close();
 
	return fileData.str();
}

void printShaderInfoLog(GLint shader)
{
	int infoLogLen = 0;
	int charsWritten = 0;
	GLchar *infoLog;
 
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);
 
	if (infoLogLen > 0)
	{
		infoLog = new GLchar[infoLogLen];
		// error check for fail to allocate memory omitted
		glGetShaderInfoLog(shader, infoLogLen, &charsWritten, infoLog);
		cout << "InfoLog : " << endl << infoLog << endl;
		delete [] infoLog;
	}
}

int LoadShader(const char *pfilePath_vs, const char *pfilePath_fs, bool bindTexCoord0, bool bindNormal, bool bindColor, GLuint &shaderProgram, GLuint &vertexShader, GLuint &fragmentShader)
{
	shaderProgram=0;
	vertexShader=0;
	fragmentShader=0;
 
	// load shaders & get length of each
	int vlen;
	int flen;
	std::string vertexShaderString = loadFile(pfilePath_vs);
	std::string fragmentShaderString = loadFile(pfilePath_fs);
	vlen = vertexShaderString.length();
	flen = fragmentShaderString.length();
 
	if(vertexShaderString.empty()){return -1;}
	if(fragmentShaderString.empty()){return -1;}
 
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);	
 
	const char *vertexShaderCStr = vertexShaderString.c_str();
	const char *fragmentShaderCStr = fragmentShaderString.c_str();
	glShaderSource(vertexShader, 1, (const GLchar **)&vertexShaderCStr, &vlen);
	glShaderSource(fragmentShader, 1, (const GLchar **)&fragmentShaderCStr, &flen);
 
	GLint compiled;
 
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);
	if(compiled==false)
	{
		cout << "Vertex shader not compiled." << endl;
		printShaderInfoLog(vertexShader);
 
		glDeleteShader(vertexShader);
		vertexShader=0;
		glDeleteShader(fragmentShader);
		fragmentShader=0;
 
		return -1;
	}
 
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compiled);
	if(compiled==false)
	{
		cout << "Fragment shader not compiled." << endl;
		printShaderInfoLog(fragmentShader);
 
		glDeleteShader(vertexShader);
		vertexShader=0;
		glDeleteShader(fragmentShader);
		fragmentShader=0;
 
		return -1;
	}
 
	shaderProgram = glCreateProgram();
 
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
 
	glBindAttribLocation(shaderProgram, 0, "InVertex");
 
	if(bindTexCoord0)
		glBindAttribLocation(shaderProgram, 1, "InTexCoord0");
 
	if(bindNormal)
		glBindAttribLocation(shaderProgram, 2, "InNormal");
 
	if(bindColor)
		glBindAttribLocation(shaderProgram, 3, "InColor");
 
	glLinkProgram(shaderProgram);
 
	GLint IsLinked;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, (GLint *)&IsLinked);
	if(IsLinked==false)
	{
		cout << "Failed to link shader." << endl;
 
		GLint maxLength;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);
		if(maxLength>0)
		{
			char *pLinkInfoLog = new char[maxLength];
			glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, pLinkInfoLog);
			cout << pLinkInfoLog << endl;
			delete [] pLinkInfoLog;
		}
 
		glDetachShader(shaderProgram, vertexShader);
		glDetachShader(shaderProgram, fragmentShader);
		glDeleteShader(vertexShader);
		vertexShader=0;
		glDeleteShader(fragmentShader);
		fragmentShader=0;
		glDeleteProgram(shaderProgram);
		shaderProgram=0;
 
		return -1;
	}
 
	return 1;		//Success
}

Mesh::Mesh() {
	//maybe take params
	//TODO: below was seg faulting
	/*if(LoadShader("shaders/Shader1.vert", "shaders/Shader1.frag", false, false, true, ShaderProgram, VertexShader, FragmentShader)==-1)
	{
		exit(1);
	}
	else
	{
		ProjectionModelviewMatrix_Loc=glGetUniformLocation(ShaderProgram, "ProjectionModelviewMatrix");
	}
	*/
	
}

Mesh::~Mesh(){

}


void Mesh::set(const Eigen::Ref<Eigen::MatrixXd>& V,const Eigen::Ref<Eigen::MatrixXi>& F,int numFaces,int numVerts){
	int i;
	
	for(i = 0; i < numFaces; i++){
		Face f_row;
		
        f_row.x = (int) F(i,0);
        f_row.y = (int) F(i,1);
        f_row.z = (int) F(i,2);
      	faces.push_back(f_row);
    }    

	for(i = 0; i < numVerts; i++){
        Vertex row;
        row.x = (double) V(i,0);
        row.y = (double) V(i,1);
        row.z = (double) V(i,2);
        verts.push_back(row);
    }

	std::cout << "set" << std::endl;    

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(Face), &faces[0], GL_DYNAMIC_DRAW);
 
	//GLenum error=glGetError();
 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
 
	//Create VBO for the quad
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);


	//this function changes the size of the VBO
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), &verts[0], GL_DYNAMIC_DRAW);
 
	//Just testing
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
 
	//Bind the VBO and setup pointers for the VAO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(0));
	glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), BUFFER_OFFSET(sizeof(double)*3));
	glEnableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
 
	//Bind the IBO for the VAO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
}
        
void Mesh::draw(){
	float projectionModelviewMatrix[16];
 
	//Just set it to identity matrix
	memset(projectionModelviewMatrix, 0, sizeof(float)*16);
	projectionModelviewMatrix[0]=1.0;
	projectionModelviewMatrix[5]=1.0;
	projectionModelviewMatrix[10]=1.0;
	projectionModelviewMatrix[15]=1.0;
 
	//Clear all the buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
 
	//Bind the shader that we want to use
	glUseProgram(ShaderProgram);
	//Setup all uniforms for your shader
	glUniformMatrix4fv(ProjectionModelviewMatrix_Loc, 1, false, projectionModelviewMatrix);
	//Bind the VAO
	glBindVertexArray(VAO);
	//At this point, we would bind textures but we aren't using textures in this example
 
	//Draw command
	//The first to last vertex is 0 to 3
	//6 indices will be used to render the 2 triangles. This make our quad.
	//The last parameter is the start address in the IBO => zero
	glDrawRangeElements(GL_TRIANGLES, 0, 3, 6, GL_UNSIGNED_SHORT, NULL);
}

void Mesh::loadShader(const char* vertexFileName, const char* fragmentFileName)
{
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vs, 1, (const GLchar**)&vertexFileName, NULL);
	glCompileShader(vs);
	GLuint vs_program = glCreateProgram();
	glAttachShader(vs_program, vs);
	glProgramParameteri(vs_program, GL_PROGRAM_SEPARABLE, GL_TRUE);
	glLinkProgram(vs_program);

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(fs, 1, (const GLchar**)&fragmentFileName, NULL);
	glCompileShader(fs);
	GLuint fs_program = glCreateProgram();
	glAttachShader(fs_program, fs);
	glProgramParameteri(fs_program, GL_PROGRAM_SEPARABLE, GL_TRUE);
	glLinkProgram(fs_program);

	GLuint program_pipeline;
	glGenProgramPipelines(1, &program_pipeline);
	glUseProgramStages(program_pipeline, GL_VERTEX_SHADER_BIT, vs_program);
	glUseProgramStages(program_pipeline, GL_FRAGMENT_SHADER_BIT, fs_program);
}


