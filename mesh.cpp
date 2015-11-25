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

/*
 *	This is the value of the iso surface at x
 */
double Mesh::hrbfFunct(Vector3d x){
	double val = 0;
	Vector3d diff;

	for(int i = 0; i < num_verts; i++){
		diff = x - vecVerts[i];
		double l = diff.norm();

		val += l != 0 ? 
			alpha_beta(i,0) * phi(l) 
			+ (dphi(l) / l) * (alpha_beta(i,1) * diff[0] + alpha_beta(i,2) * diff[1] + alpha_beta(i,3) * diff[2]) 
			: 0;
	}

	return val;
}

/*
 *	This is the gradient of the iso surface at x
 */
Vector3d Mesh::hrbfGrad(Vector3d x){
	Vector3d val, diff;
	val << 0, 0, 0;

	for(int i = 0; i < num_verts; i++){
		diff = x - vecVerts[i];
		double l = diff.norm();

		if(l == 0)
			continue;

		Vector3d c_step;
			c_step << alpha_beta(i,1), alpha_beta(i,2), alpha_beta(i,3);
		c_step = 
		 	diff * (alpha_beta(i,0) * dphi(l) / l)
		 	+ c_step * (dphi(l)/l)
		 	+ diff * (diff.dot(c_step) * (1/(l*l)) * (ddphi(l) - (dphi(l)/l)));

		val += c_step;
	}
	
	return val;
}

double Mesh::compositionHrbf(Vector3d x){
	double max = hrbfFunct(x);
	for(int i = 0; i < neighbors.size(); i++){
		double f_i = neighbors[i]->hrbfFunct(x);
		if(f_i > max)
			max = f_i;
	}

	return max;
}


void Mesh::generateVerticies(){
	int j = 0;
	int f1,f2,f3,fn;
	num_verts = V.rows();
	vecVerts.resize(num_verts);
	vecNorms.resize(num_verts);
	vecIsos.resize(num_verts);

	for(int i = 0; i < num_verts; i++){
		vector <int> newColumn;
		vector <int> newInd;
		vector <double> newBary;
		VindexMap.push_back(newColumn);
		edgeMap.push_back(newInd);
		baryCoords.push_back(newBary);

		Vector3d v;
		v << V(i,0),V(i,1),V(i,2);
		vecVerts[i] = v;
	}

	for (unsigned int i = 0; i < F.rows(); i++) {

		f1 = F(i,0);
		fn = FN(i,0);
		VindexMap[f1].push_back(j);
		verticies[j].x = V(f1,0); verticies[j].y = V(f1,1); verticies[j].z = V(f1,2);
		verticies[j].nx = N(fn,0); verticies[j].ny = N(fn,1); verticies[j].nz = N(fn,2);
		j++;


		f2 = F(i,1);
		fn = FN(i,0);
		VindexMap[f2].push_back(j);
		verticies[j].x = V(f2,0); verticies[j].y = V(f2,1); verticies[j].z = V(f2,2);
		verticies[j].nx = N(fn,0); verticies[j].ny = N(fn,1); verticies[j].nz = N(fn,2);
		j++;

		f3 = F(i,2);
		fn = FN(i,0);
		VindexMap[f3].push_back(j);
		verticies[j].x = V(f3,0); verticies[j].y = V(f3,1); verticies[j].z = V(f3,2);
		verticies[j].nx = N(fn,0); verticies[j].ny = N(fn,1); verticies[j].nz = N(fn,2);
		j++;

		save_face(f1,f2,f3);
		save_face(f2,f3,f1);
		save_face(f3,f1,f2);

	}

	for(unsigned int i = 0; i < num_verts; i++){
		//per vertex normals
		vector < double > n_avg = {0, 0, 0};
		int size = VindexMap[i].size();
		for(int j = 0; j < size; j++) {
			VBOvertex vert = verticies[VindexMap[i][j]];
			n_avg[0] += vert.nx;
			n_avg[1] += vert.ny;
			n_avg[2] += vert.nz;
		}

		Vector3d n;
		n << (size == 0 ? 0 : n_avg[0] / size), (size == 0 ? 0 : n_avg[1] / size), (size == 0 ? 0 : n_avg[2] / size);
		vecNorms[i] = n;
	}
}

/*
 * This method calulates the barycentric coordinates of each vertex
 * Two things to keep in mind:
 *	1)	This is done on a per triangle (not per edge) basis, so the coefficients more accurately
 *		represent barycentric coordinates with the midpoint of opposite edges as the basis vectors
 *	2)	This should be called after unions between shapes are done so that contributions from other
 *		meshes count as well
 */
void Mesh::generateBaryCoords(){

	for (int i = 0; i < num_verts; ++i){
		int sharedInd = -1;
		Mesh* sharedMesh;

		//tries to find out if this is a shared edge
		for(int j = 0; j < neighbors.size(); j++){
			for(int k = 0; k < neighborIndex[j].size(); k+=2){
				if(i == neighborIndex[j][k]){
					//sharedInd is now effectively i for the shared mesh
					sharedInd = neighborIndex[j][k+1];
					sharedMesh = neighbors[j];
					goto break_bary;
				}
			}
		}

		break_bary:

		double area = 0.0;
		int size = edgeMap[i].size() / 2;
		for(int j = 0; j < size; j++) {
			Vector3d v1 = vecVerts[edgeMap[i][2*j]];
			Vector3d v2 = vecVerts[edgeMap[i][2*j+1]];
			double triangle = 0.5 * (v1.cross(v2).norm());	
			baryCoords[i].push_back(triangle);
			area += triangle;
		}

		//this is shown to contribute. We assume it is accurate
		if(sharedInd != -1){
			int size = sharedMesh->edgeMap[sharedInd].size() / 2;
			for(int j = 0; j < size; j++) {
				Vector3d v1 = sharedMesh->vecVerts[sharedMesh->edgeMap[sharedInd][2*j]];
				Vector3d v2 = sharedMesh->vecVerts[sharedMesh->edgeMap[sharedInd][2*j+1]];
				double triangle = 0.5 * (v1.cross(v2).norm());	

				//this vector now is not half the size of the corresponding edge index vector
				baryCoords[i].push_back(triangle);
				area += triangle;
			}
		}

		for(int j = 0; j < size; j++)
			baryCoords[i][j] = baryCoords[i][j] / area;
	}
}

void Mesh::tangentalRelax(int iterations){
	int sharedInd = -1, index, edgeSize;
	double mu, scalar, f_i, alpha = 0.01;//depends on scale of model
	Mesh* sharedMesh;
	Vector3d sum, n_sum;

	while(iterations > 0){
		iterations--;

		//should look at edges here and gloss over them in the next traversal 
		for(int i = 0; i < neighbors.size(); i++){
			sharedMesh = neighbors[i];

			for(int j = 0; j < neighborIndex[i].size(); j+=2){
				index = neighborIndex[i][j];
				sharedInd = neighborIndex[i][j+1];

				mu = max(
					max(
						0.0, 
						1.0 - pow(abs(hrbfFunct(vecVerts[index]) - vecIsos[index]) - 1, 4)
					), 
					1.0 - pow(abs(hrbfFunct(sharedMesh->vecVerts[sharedInd]) - sharedMesh->vecIsos[sharedInd]) - 1, 4)
				);


				sum = (1 - mu)*vecVerts[index];
				n_sum = (1 - mu)*vecNorms[index];

				edgeSize = edgeMap[index].size() / 2;
				for(int k = 0; k < edgeSize; k++ ){
					Vector3d mid = 0.5 * (vecVerts[edgeMap[index][2*k]] + vecVerts[edgeMap[index][2*k+1]]);
					Vector3d n_mid = 0.5 * (vecNorms[edgeMap[index][2*k]] + vecNorms[edgeMap[index][2*k+1]]);

					sum += mu*baryCoords[index][k]*mid;
					n_sum += mu*baryCoords[index][k]*n_mid;
				}

				//this made diff smaller cuz didn't account for all the points yet
				edgeSize = sharedMesh->edgeMap[sharedInd].size() / 2;
				for(int k = 0; k < edgeSize; k++ ){
					Vector3d mid = 0.5 * (sharedMesh->vecVerts[sharedMesh->edgeMap[sharedInd][2*k]] + sharedMesh->vecVerts[sharedMesh->edgeMap[sharedInd][2*k+1]]);
					Vector3d n_mid = 0.5 * (sharedMesh->vecNorms[sharedMesh->edgeMap[sharedInd][2*k]] + sharedMesh->vecNorms[sharedMesh->edgeMap[sharedInd][2*k+1]]);

					sum += mu*sharedMesh->baryCoords[sharedInd][k]*mid;
					n_sum += mu*sharedMesh->baryCoords[sharedInd][k]*n_mid;
				}

				n_sum.normalize();
				//cout << "n diff: " << n_sum.dot(vecNorms[i]) << endl;

				vecVerts[index] = sum;
				vecNorms[index] = n_sum;
				sharedMesh->vecVerts[sharedInd] = sum;
				sharedMesh->vecNorms[sharedInd] = n_sum;
			}
		}

		//normal points
		for(int i = 0; i < num_verts; i++){
			//this just makes sure we gloss over the points above
			for(int j = 0; j < neighbors.size(); j++)
				for(int k = 0; k < neighborIndex[j].size(); k+=2)
					if(i == neighborIndex[j][k])
						goto skip;

			f_i = hrbfFunct(vecVerts[i]);
			mu = max(0.0, 1 - pow(abs(f_i - vecIsos[i]) - 1, 4));
			//cout << "mu: " << mu << " f(v_i): " << f_i << " iso_i " << vecIsos[i] << endl;

			sum = (1 - mu)*vecVerts[i];
			n_sum = (1 - mu)*vecNorms[index];

			edgeSize = edgeMap[i].size() / 2;
			for(int j = 0; j < edgeSize; j++ ){
				Vector3d mid = 0.5 * (vecVerts[edgeMap[i][2*j]] + vecVerts[edgeMap[i][2*j+1]]);
				Vector3d n_mid = 0.5 * (vecNorms[edgeMap[i][2*j]] + vecNorms[edgeMap[i][2*j+1]]);

				sum += mu*baryCoords[i][j]*mid;
				n_sum += mu*baryCoords[i][j]*n_mid;
			}

			
			n_sum.normalize();
			vecVerts[i] = sum;
			vecNorms[i] = n_sum;

			skip:;		
		}

		//vertex projection
		vector< Vector3d > iso_grads(num_verts);
		for(int i = 0; i < num_verts; i++){
			Vector3d grad_f = hrbfGrad(vecVerts[i]);
			double 	f_i = compositionHrbf(vecVerts[i]),
					norm = grad_f.norm();
			norm = norm != 0 ? norm * norm : 1; //the latter shouldn't happen

			iso_grads[i] = (alpha * (vecIsos[i] - f_i) / norm) * grad_f;
		}

		//we need 2 loops for this part since the values
		//of the verticies effect the hrbf function
		for(int i = 0; i < num_verts; i++)
			vecVerts[i] += iso_grads[i];

		//update neighbors as well
		for(int i = 0; i < neighbors.size(); i++)
			for(int j = 0; j < neighborIndex[i].size(); j+=2)
				neighbors[i]->vecVerts[neighborIndex[i][j+1]] += iso_grads[neighborIndex[i][j]];

	}

	//for(int i = 0; i < num_verts; i++)
	//	cout << i << ": " << hrbfFunct(vecVerts[i]) << ' ' <<  vecIsos[i] << endl;

	for(int i = 0; i < neighbors.size(); i++)
		neighbors[i]->regenVerts();
	regenVerts();

}

/* this method just transfers points from vecVerts to the buffer */
void Mesh::regenVerts(){
	for(int i = 0; i < VindexMap.size(); i++){
		vector <int> indicies = VindexMap[i];

		for(int j = 0; j < indicies.size(); j ++){
			verticies[VindexMap[i][j]].x = vecVerts[i](0); 
			verticies[VindexMap[i][j]].y = vecVerts[i](1);
			verticies[VindexMap[i][j]].z = vecVerts[i](2);
			verticies[VindexMap[i][j]].nx = vecNorms[i](0); 
			verticies[VindexMap[i][j]].ny = vecNorms[i](1);
			verticies[VindexMap[i][j]].nz = vecNorms[i](2);
		}
	}
}

void Mesh::generateHrbfCoefs(){
	int num_verts = V.rows();
	B.resize(num_verts * 4, 1);
	vecVerts.resize(num_verts);

	for(unsigned int i = 0; i < num_verts; i++){
		vector < double > n_avg = {0, 0, 0};
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

		Vector3d v1;
		v1 << V(i,0), V(i,1), V(i,2);
		vecVerts[i] = v1;
	}


	A.resize(num_verts * 4,num_verts * 4); //the 4 is because of the scalar and vector component we are solving for
	int x_o, y_o;
	double a1, a2, a3, _c, d1, d2, d3;
	Vector3d x;
	Vector3d _d;

	for(int i = 0; i < num_verts; i++){
		for(int j = 0; j < num_verts; j++){
			//x and y offset and current x (x_j)
			x_o = i*4;
			y_o = j*4;
			x = vecVerts[j];
			double grad = b(x, i);
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

  	double sum = 0;
    for(int i = 0; i < num_verts; i++){
      	vecIsos[i] = hrbfFunct(vecVerts[i]);
    	sum += vecIsos[i];
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
		Vector3d top, diff;
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
 *
 *	updates normals and verticies as well
 */
void Mesh::transform(Matrix3d rot){

	Vector3d 	vec, norm;
	Vector3d	x_proj = rot * x_axis, 
				y_proj = rot * y_axis, 
				z_proj = rot * z_axis;
	for(int i = 0; i < VindexMap.size(); i++){
		vec = boneCoords[i];
		vec = vec(0)*x_proj + vec(1)*y_proj + vec(2)*z_proj + origin;
		norm = rot * vecNorms[i];

		vecVerts[i] = vec;
		vecNorms[i] = norm;
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
 */ 
void Mesh::boneCalc(){
	Vector3d rel_vec;

	for(int i = 0; i < num_verts; i++){
		Vector3d bone_c;
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
	glColor3d(0.f,0.f,0.f);
	for(auto & v : verticies) {
		glNormal3d(v.nx,v.ny,v.nz);
	    glVertex3d(v.x, v.y, v.z);
	}

	glEnd();
	glUseProgram(0);
	glPopMatrix();

}

void Mesh::createUnion(Mesh* m1, Mesh* m2){
	vector< int > newNeigbor1;
	vector< int > newNeigbor2;

	m1->neighbors.push_back(m2);
	m2->neighbors.push_back(m1);
	m1->neighborIndex.push_back(newNeigbor1);
	m2->neighborIndex.push_back(newNeigbor2);

	int m1size = m1->neighborIndex.size() - 1;
	int m2size = m2->neighborIndex.size() - 1;
	
	for(int i = 0; i < m1->num_verts; i++){
		double minDist = 20;
		int minInd = -1;

		for(int j = 0; j < m2->num_verts; j++){
			double dist = (m1->vecVerts[i] - m2->vecVerts[j]).norm();

			if(dist < minDist){
				minDist = dist;
				minInd = j;
			}
		}	

		if(minDist < EDGE_ERROR){
			//i is the index in m1 and minInd in m2
			m1->neighborIndex[m1size].push_back(i);
			m1->neighborIndex[m1size].push_back(minInd);
			m2->neighborIndex[m1size].push_back(minInd);
			m2->neighborIndex[m1size].push_back(i);
			//seems to work fine
			//cout << "connecting " << i << ' ' << minInd << endl;
		}
	}
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


