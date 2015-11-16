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
	for (int i=0; i<360; i++) {
        precomputed_sin[i] = sin(i*2*M_PI/360);
    }

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
	for (int i=0; i<360; i++) {
        precomputed_sin[i] = sin(i*2*M_PI/360);
    }
}

Mesh::~Mesh(){
	delete [] &VindexMap;
	delete [] &verticies;
	delete [] &vecVerts;
	delete [] &vecNorms;
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
	vecNorms.resize(num_verts);

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

	for(unsigned int i = 0; i < num_verts; i++){
		vector < float > n_avg = {0, 0, 0};
		int size = VindexMap[i].size();
		for(int j = 0; j < size; j++) {
			VBOvertex vert = verticies[VindexMap[i][j]];
			n_avg[0] += vert.nx;
			n_avg[1] += vert.ny;
			n_avg[2] += vert.nz;
		}

		Vector3f n;
		n << (size == 0 ? 0 : n_avg[0] / size), (size == 0 ? 0 : n_avg[1] / size), (size == 0 ? 0 : n_avg[2] / size);
		vecNorms[i] = n;
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
			n_avg[0] += vert.nx;
			n_avg[1] += vert.ny;
			n_avg[2] += vert.nz;
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
//also read in .cen files
void Mesh::readHrbf(){
	string line;
	string file;
	file = "objs/" + base + ".hrbf";
  	ifstream in (file);
  	vector <string> sp_line;

  	int num_verts = V.rows();
	alpha_beta.resize(num_verts, 4);
	boneCoords.resize(num_verts);

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


    file = "objs/" + base + ".cen";
    ifstream inCen (file);
  	
  	if (inCen.is_open())
  	{
    	getline (inCen,line);
		sp_line = split(line, ' ');
		origin << stod(sp_line[0]) , stod(sp_line[1]) , stod(sp_line[2]);

		getline (inCen,line);
		sp_line = split(line, ' ');
		Vector3f top, diff;
		top << stod(sp_line[0]) , stod(sp_line[1]) , stod(sp_line[2]);
		z_axis = top - origin;

		//pick any random vector to try this with
		diff = origin - vecVerts[0];
		//just in case, this won't happen twice guaranteed by constructing the origin
		if(diff.norm() == 0)
			diff = origin - vecVerts[1];

		x_axis = diff.cross(z_axis);
		x_axis.normalize();
		z_axis.normalize();
		y_axis = z_axis.cross(x_axis);

    	inCen.close();
    }
  	else 
  		fprintf(stderr, "Unable to process cen file\n");
}

void Mesh::writeHrbf(){
	ofstream out;
	string file;
	file = "objs/" + base + ".hrbf";
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

/*
 *	rot is a rotation matrix rotating the x,y and z axis 
 * 	about the origin of this bone (a joint)
 */
void Mesh::transform(Matrix3f rot){

	Vector3f 	vec, norm;
	Vector3f	x_proj = rot * x_axis, 
				y_proj = rot * y_axis, 
				z_proj = rot * z_axis;
	for(int i = 0; i < VindexMap.size(); i++){
		vec = boneCoords[i];
		vec = vec(0)*x_proj + vec(1)*y_proj + vec(2)*z_proj + origin;
		norm = rot * vecNorms[i];

		vector <int> indicies = VindexMap[i];

		for(int j = 0; j < indicies.size(); j ++){
			verticies[VindexMap[i][j]].x = vec(0); 
			verticies[VindexMap[i][j]].y = vec(1);
			verticies[VindexMap[i][j]].z = vec(2);

			verticies[VindexMap[i][j]].nx = norm(0); 
			verticies[VindexMap[i][j]].ny = norm(1); 
			verticies[VindexMap[i][j]].nz = norm(2);
		}
	}

}

/* 
 * calculate each vertex in cartesian (bone) coordinates at origin 
 * with up=z and x_axis=x
 */ 
void Mesh::boneCalc(){
	Vector3f rel_vec;
	//just make this a global already
	int num_verts = V.rows();
	

	for(int i = 0; i < V.rows(); i++){
		Vector3f bone_c;
		double x, y, z;

		rel_vec = vecVerts[i] - origin;
		x = rel_vec.dot(x_axis);
		y = rel_vec.dot(y_axis);
		z = rel_vec.dot(z_axis);

		bone_c << x, y, z;
		boneCoords[i] = bone_c;
	}
}

/*
 * This function just wraps most of the essential viewing functions for brevity
 */
void Mesh::setView(const char* fileName){
	set(fileName);
    generateVerticies();
    readHrbf();
    boneCalc();
}

void Mesh::set(const char* fileName){
	string b;
	b.append(fileName);
	vector< string > sp_string = split(b, '.');
	b = sp_string[0];
	vector< string > sp_base = split(b, '/');
	base = sp_base[sp_base.size() - 1]; 

	int i;
	igl::readOBJ(fileName,V,TC,N,F,FTC,FN);

    buff_size = F.rows() * 3;
    verticies.resize(buff_size);    

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


