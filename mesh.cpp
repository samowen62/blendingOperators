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

Mesh::Mesh(const char* shaderFile){
	string file, vert, frag;
	file.append(shaderFile);
	vert = "shaders/" + file + ".vert";
	frag = "shaders/" + file + ".frag";
	cout << vert << frag << endl;

	if(loadShader(vert.c_str(), frag.c_str())==-1)
	{
		exit(1);
	}
};

Mesh::Mesh() {
	
}

Mesh::~Mesh(){
	delete [] &VindexMap;
	delete [] &verticies;
	delete [] &vecVerts;
}

float Mesh::hrbfFunct(Vector3f x){
	float val = 0;
	Vector3f diff;

	for(int i = 0; i < vecVerts.size(); i++){
		diff = x - vecVerts[i];
		float l = diff.norm();

		val += l != 0 ? alpha_beta(i,0) * phi(l) + (dphi(l) / l) * (alpha_beta(i,1) * diff[0] + alpha_beta(i,2) * diff[1] + alpha_beta(i,3) * diff[2]) : 0;
	}

	return val;
}

void Mesh::generateVerticies(){
	int j = 0;
	int f,fn;
	int num_verts = V.rows();
	vecVerts.resize(num_verts);

	for(int i = 0; i < num_verts; i++){
		vector <int> newColumn;
		VindexMap.push_back(newColumn);

		Vector3f v;
		v << V(i,0),V(i,1),V(i,2);
		vecVerts[i] = v;
	}

	for (unsigned int i = 0; i < F.rows(); i++) {

		f = F(i,0);
		fn = FN(i,0);
		save_Vdata(f,j);
		verticies[j].x = V(f,0); verticies[j].y = V(f,1); verticies[j].z = V(f,2);
		verticies[j].nx = N(fn,0); verticies[j].ny = N(fn,1); verticies[j].nz = N(fn,2);
		j++;


		f = F(i,1);
		fn = FN(i,0);
		save_Vdata(f,j);
		verticies[j].x = V(f,0); verticies[j].y = V(f,1); verticies[j].z = V(f,2);
		verticies[j].nx = N(fn,0); verticies[j].ny = N(fn,1); verticies[j].nz = N(fn,2);
		j++;

		f = F(i,2);
		fn = FN(i,0);
		save_Vdata(f,j);
		verticies[j].x = V(f,0); verticies[j].y = V(f,1); verticies[j].z = V(f,2);
		verticies[j].nx = N(fn,0); verticies[j].ny = N(fn,1); verticies[j].nz = N(fn,2);
		j++;

	}

}

void Mesh::generateHrbfCoefs(){
	int num_verts = V.rows();
	B.resize(num_verts * 4, 1);
	vecVerts.resize(num_verts);

	for(unsigned int i = 0; i < num_verts; i++){
		vector < float > n_avg = {0, 0, 0};
		int size = VindexMap[i].size();
		for(int j = 0; j < size; j++) {
			VBOvertex vert = verticies[VindexMap[i][j]];
			n_avg[0] += vert.x;
			n_avg[1] += vert.y;
			n_avg[2] += vert.z;
		}
		B(4*i, 0) = 0.5;
		B(4*i+1, 0) = size == 0 ? 0 : n_avg[0] / size;
		B(4*i+2, 0) = size == 0 ? 0 : n_avg[1] / size;
		B(4*i+3, 0) = size == 0 ? 0 : n_avg[2] / size;

		Vector3f v1;
		v1 << V(i,0), V(i,1), V(i,2);
		vecVerts[i] = v1;
	}


	A.resize(num_verts * 4,num_verts * 4); //the 4 is because of the scalar and vector component we are solving for
	int x_o, y_o;
	float a1, a2, a3, _c, d1, d2, d3;
	Vector3f x;
	Vector3f _d;

	for(int i = 0; i < num_verts; i++){
		for(int j = 0; j < num_verts; j++){
			//x and y offset and current x (x_j)
			x_o = i*4;
			y_o = j*4;
			x = vecVerts[j];
			float grad = b(x, i);
			a1 = grad * (x(0) - vecVerts[i](0));
			a2 = grad * (x(1) - vecVerts[i](1));
			a3 = grad * (x(2) - vecVerts[i](2));

			A(x_o,y_o) = phi(l(x, i));
			A(x_o + 1,y_o) = a1; A(x_o,y_o + 1) = a1;
			A(x_o + 2,y_o) = a2; A(x_o,y_o + 2) = a2;
			A(x_o + 3,y_o) = a3; A(x_o,y_o + 3) = a3;

			_c = c(x, i);
			_d = x - vecVerts[i];
			d1 = _d(0);
			d2 = _d(1);
			d3 = _d(2);

			A(x_o+1,y_o+1) = _c*d1*d1+grad; A(x_o+1,y_o+2) = _c*d1*d2; A(x_o+1,y_o+3) = _c*d1*d3;
			A(x_o+2,y_o+1) = _c*d2*d1; A(x_o+2,y_o+2) = _c*d2*d2+grad; A(x_o+2,y_o+3) = _c*d2*d3;
			A(x_o+3,y_o+1) = _c*d3*d1; A(x_o+3,y_o+2) = _c*d3*d2; A(x_o+3,y_o+3) = _c*d3*d3+grad;

		}
	}

	cout << "solving the equation..." << endl;

	alpha_beta = A.fullPivLu().solve(B);
	double error = (A*alpha_beta - B).norm() / B.norm();

	cout << "solved! Error: " << error << endl;
}

//call this function after we have V,N and F
void Mesh::readHrbf(){
	string line;
	string file;
	file = base + ".hrbf";
  	ifstream in (file);
  	vector <string> sp_line;

  	int num_verts = V.rows();
	alpha_beta.resize(num_verts, 4);

	if(num_verts <= 0)
		return;

  	if (in.is_open())
  	{
  		int i = 0;
    	while ( getline (in,line) )
    	{
    		sp_line = split(line, ' ');
    		if(sp_line.size() != 4){
    			fprintf(stderr, "Error in hrbf file. Needs 4 columns\n");
    			return;
    		}

    		alpha_beta(i, 0) = stod(sp_line[0]);
    		alpha_beta(i, 1) = stod(sp_line[1]);
    		alpha_beta(i, 2) = stod(sp_line[2]);
    		alpha_beta(i, 3) = stod(sp_line[3]);
      		i++;
    	}
    	in.close();
    }
  	else 
  		fprintf(stderr, "Unable to process hrbf file\n");

  	float sum = 0;
    for(int i = 0; i < num_verts; i++){
      sum += hrbfFunct(vecVerts[i]);
    }
    avg_iso = sum / num_verts;
    cout << "average iso value: " << avg_iso << endl;
}

void Mesh::writeHrbf(){
	ofstream out;
	string file;
	file = base + ".hrbf";
	out.open(file);
	int num_verts = V.rows();

	for(int i = 0; i < num_verts; i++){
		int b = 4 * i;
		out << alpha_beta(b,0) << " ";
		out << alpha_beta(b+1,0) << " ";
		out << alpha_beta(b+2,0) << " ";
		out << alpha_beta(b+3,0) << "\n";
	}
	out.close();
}

void Mesh::set(const char* fileName){
	string b;
	b.append(fileName);
	vector< string > sp_string = split(b, '.');
	base = sp_string[0];

	int i;
	igl::readOBJ(fileName,V,TC,N,F,FTC,FN);

    buff_size = F.rows() * 3;
    verticies.resize(buff_size);    
		    
  	//V.rowwise() -= V.colwise().mean();
  	//V /= (V.colwise().maxCoeff()-V.colwise().minCoeff()).maxCoeff();

    /*cout << "V size: " << V.rows() << "x" << V.cols() << endl;
    cout << "N size: " << N.rows() << "x" << N.cols() << endl;
    cout << "F size: " << F.rows() << "x" << F.cols() << endl;
    cout << "FN size: " << FN.rows() << "x" << FN.cols() << endl;*/

}
        
void Mesh::draw(){
	glPushMatrix();
	glUseProgram(ShaderProgram);
	glBegin(GL_TRIANGLES);
	glColor3f(0.f,0.f,0.f);
	for(auto & v : verticies) {
		glNormal3f(v.nx,v.ny,v.nz);
	    glVertex3f(v.x, v.y, v.z);
	}

	glEnd();
	glUseProgram(0);
	glPopMatrix();

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


