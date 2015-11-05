#ifndef _MESH_H_
#define _MESH_H_

#include <Eigen/Dense>
#include "inc.h"
#include <fstream>
#include <string.h>

using namespace std;

struct Vertex
{
  double x, y, z;
};

struct Face
{
  int x;
  int y;
  int z;
};

struct VBOvertex
{
  float x, y, z;        // Vertex
  float nx, ny, nz;     // Normal
};

class Mesh {
    private:
      GLuint VAO;
      GLuint IBO;
      GLuint VBO;
      
      GLuint  VertexShader;
      GLuint  FragmentShader;
      vector< VBOvertex > verticies;
      //vector< vector< float > > verticies;

      int numFaces;
      int numVerts;
      int ProjectionModelviewMatrix_Loc;
      int buff_size;
      vector< Vertex > verts;
      vector< Face > faces;

      Eigen::MatrixXd V;
      Eigen::MatrixXd TC; //don't need
      Eigen::MatrixXd N;
      Eigen::MatrixXi F;
      Eigen::MatrixXi FTC; //don't need
      Eigen::MatrixXi FN; //don't need

    public: 

        GLuint  ShaderProgram;
        Mesh();
        //make virtual so that other desctructed instances of classes extending
        //this one called as a Mesh get both destructors
        //in java only final methods are not virtual
        virtual ~Mesh(void);
        
        //void draw(Eigen::Matrix4f& proj, Eigen::Affine3f& model);
        void draw();
        int  loadShader(const char* vertexFileName, const char* fragmentFileName);
        //void set(const Eigen::Ref<Eigen::MatrixXd>& V,const Eigen::Ref<Eigen::MatrixXi>& F,int numFaces,int numVerts);
        void set(const char* fileName);
        void generateVerticies();
};

#endif


