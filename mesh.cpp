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

Mesh::Mesh() {
	//maybe take params
	//TODO: below was seg faulting
	if(loadShader("shaders/Shader1.vert", "shaders/Shader1.frag")==-1)
	{
		exit(1);
	}
	else
	{
		ProjectionModelviewMatrix_Loc=glGetUniformLocation(ShaderProgram, "ProjectionModelviewMatrix");
	}
	
	
}

Mesh::~Mesh(){

}

void Mesh::generateVerticies(){
	int j = 0;
	int f;

	for (unsigned int i = 0; i < F.rows(); i++) {
		
/*
		f = F(i,0);		
		verticies.push_back({ V(f,0), V(f,1), V(f,2) });
		f = F(i,1);		
		verticies.push_back({ V(f,0), V(f,1), V(f,2) });
		f = F(i,2);		
		verticies.push_back({ V(f,0), V(f,1), V(f,2) });
*/		
		f = F(i,0);
		verticies[j].x = V(f,0); verticies[j].y = V(f,1); verticies[j].z = V(f,2);
		verticies[j].nx = N(f,0); verticies[j].ny = N(f,1); verticies[j].nz = N(f,2);
		j++;


		f = F(i,1);
		verticies[j].x = V(f,0); verticies[j].y = V(f,1); verticies[j].z = V(f,2);
		verticies[j].nx = N(f,0); verticies[j].ny = N(f,1); verticies[j].nz = N(f,2);
		j++;

		f = F(i,2);
		verticies[j].x = V(f,0); verticies[j].y = V(f,1); verticies[j].z = V(f,2);
		verticies[j].nx = N(f,0); verticies[j].ny = N(f,1); verticies[j].nz = N(f,2);
		j++;

	}
	//delete [] verticies;//to delete

	//for(auto k: verticies)
	//	cout << k.x << ' ' << k.y << ' ' << k.z << endl;
}

//void Mesh::set(const Eigen::Ref<Eigen::MatrixXd>& V,const Eigen::Ref<Eigen::MatrixXi>& F,int numFaces,int numVerts){
void Mesh::set(const char* fileName){
	int i;
	//igl::readOBJ(fileName,V,F);
	igl::readOBJ(fileName,V,TC,N,F,FTC,FN);

/*
	for(i = 0; i < F.rows(); i++){
		Face f_row;
		
        f_row.x = (int) F(i,0);
        f_row.y = (int) F(i,1);
        f_row.z = (int) F(i,2);
      	faces.push_back(f_row);
    }    

	for(i = 0; i < V.rows(); i++){
        Vertex row;
        row.x = (double) V(i,0);
        row.y = (double) V(i,1);
        row.z = (double) V(i,2);
        verts.push_back(row);
    }
*/
    buff_size = F.rows() * 3;
    //VBOvertex verticies[buff_size];
    verticies.resize(buff_size);
    this->generateVerticies();
		

    
  	V.rowwise() -= V.colwise().mean();
  	V /= (V.colwise().maxCoeff()-V.colwise().minCoeff()).maxCoeff();

    //std::cout << "V size: " << V.rows() << "x" << V.cols() << std::endl;
    //std::cout << "F size: " << F.rows() << "x" << F.cols() << std::endl;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &IBO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);

 
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//this function changes the size of the VBO
	//glBufferData(GL_ARRAY_BUFFER, sizeof(VBOvertex) * buff_size, &verticies, GL_DYNAMIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticies), &verticies, GL_DYNAMIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(float)*V.size(), V.data(), GL_DYNAMIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), &verts[0], GL_DYNAMIC_DRAW);
 	
 	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(Face), &faces[0], GL_DYNAMIC_DRAW);
  	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*F.size(), F.data(), GL_DYNAMIC_DRAW);

 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0); 
	glBindVertexArray(0);

}
        
//void Mesh::draw(Eigen::Matrix4f& proj, Eigen::Affine3f& model){
void Mesh::draw(){
	glPushMatrix();
	glBegin(GL_LINE_LOOP);
	glColor3f(0.f,0.f,0.f);
	for(auto & v : verticies) {
	    glVertex3f(v.x, v.y, v.z);
	}
//		glNormal3f(0.f,0.f,1.f);

	glEnd();
	glPopMatrix();
/*
	glUseProgram(ShaderProgram);
	GLint proj_loc = glGetUniformLocation(ShaderProgram,"proj");
	glUniformMatrix4fv(proj_loc,1,GL_FALSE,proj.data());
	GLint model_loc = glGetUniformLocation(ShaderProgram,"model");
	glUniformMatrix4fv(model_loc,1,GL_FALSE,model.matrix().data());

	  // Draw mesh as wireframe
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, sizeof(VBOvertex) * buff_size);
	//glDrawElements(GL_TRIANGLES, F.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
*/
}

int Mesh::loadShader(const char* vertexFileName, const char* fragmentFileName)
{
	GLenum err = glewInit();

	ShaderProgram = 0;
    VertexShader = 0;
    FragmentShader = 0;
 
	// load shaders & get length of each
	int vlen;
	int flen;
	std::string vertexShaderString = loadFile(vertexFileName);
	std::string fragmentShaderString = loadFile(fragmentFileName);
	vlen = vertexShaderString.length();
	flen = fragmentShaderString.length();
 
	if(vertexShaderString.empty()){return -1;}
	if(fragmentShaderString.empty()){return -1;}

	VertexShader = glCreateShader(GL_VERTEX_SHADER);
	FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);	

	const char *vertexShaderCStr = vertexShaderString.c_str();
	const char *fragmentShaderCStr = fragmentShaderString.c_str();

	glShaderSource(VertexShader, 1, (const GLchar **)&vertexShaderCStr, &vlen);
	glShaderSource(FragmentShader, 1, (const GLchar **)&fragmentShaderCStr, &flen);
 
	GLint compiled;
 
	glCompileShader(VertexShader);
	glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &compiled);
	if(compiled==false)
	{
		cout << "Vertex shader not compiled." << endl;
		printShaderInfoLog(VertexShader);
 
		glDeleteShader(VertexShader);
		VertexShader=0;
		glDeleteShader(FragmentShader);
		FragmentShader=0;
 
		return -1;
	}
 
	glCompileShader(FragmentShader);
	glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &compiled);
	if(compiled==false)
	{
		cout << "Fragment shader not compiled." << endl;
		printShaderInfoLog(FragmentShader);
 
		glDeleteShader(VertexShader);
		VertexShader=0;
		glDeleteShader(FragmentShader);
		FragmentShader=0;
 
		return -1;
	}
 
	ShaderProgram = glCreateProgram();
 
	glAttachShader(ShaderProgram, VertexShader);
	glAttachShader(ShaderProgram, FragmentShader);
 
	glBindAttribLocation(ShaderProgram, 0, "InVertex");
 
	//if(bindTexCoord0)
		glBindAttribLocation(ShaderProgram, 1, "InTexCoord0");
 
	//if(bindNormal)
		glBindAttribLocation(ShaderProgram, 2, "InNormal");
 
	//if(bindColor)
		glBindAttribLocation(ShaderProgram, 3, "InColor");
 
	glLinkProgram(ShaderProgram);
 
	GLint IsLinked;
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, (GLint *)&IsLinked);
	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);

	if(IsLinked==false)
	{
		cout << "Failed to link shader." << endl;
 
		GLint maxLength;
		glGetProgramiv(ShaderProgram, GL_INFO_LOG_LENGTH, &maxLength);
		if(maxLength>0)
		{
			char *pLinkInfoLog = new char[maxLength];
			glGetProgramInfoLog(ShaderProgram, maxLength, &maxLength, pLinkInfoLog);
			cout << pLinkInfoLog << endl;
			delete [] pLinkInfoLog;
		}
 
		glDetachShader(ShaderProgram, VertexShader);
		glDetachShader(ShaderProgram, FragmentShader);
		glDeleteShader(VertexShader);
		VertexShader=0;
		glDeleteShader(FragmentShader);
		FragmentShader=0;
		glDeleteProgram(ShaderProgram);
		ShaderProgram=0;
 
		return -1;
	}
 
	return 1;		//Success
}


