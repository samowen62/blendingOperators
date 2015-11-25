#ifndef _MESH_H_
#define _MESH_H_

#include <Eigen/Dense>
#include "inc.h"
#include <fstream>
#include <string.h>

#define M_PI            3.14159265358979323846  /* pi */
#define EDGE_ERROR      0.0015

using namespace std;
using namespace Eigen;

//TODO: eventually change draw to make it use this
struct VBOvertex
{
  double x, y, z;        // Vertex
  double nx, ny, nz;     // Normal
};

inline vector<string> &split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

inline vector<string> split(const string &s, char delim) {
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}


class Mesh {
    private:
      GLuint  ShaderProgram;
      GLuint  VertexShader;
      GLuint  FragmentShader;

      string base;
      int buff_size;
      double precomputed_sin[360];
      vector< VBOvertex > verticies;

      MatrixXd V;
      MatrixXd TC; //don't need for anything
      MatrixXd N;
      MatrixXi F;
      MatrixXi FTC; //don't need for anything
      MatrixXi FN;

      MatrixXd A;
      MatrixXd B;
      /* used to both generate the coefficients and store them when read in */
      MatrixXd alpha_beta;

    public:  
        int num_verts;

        vector< Vector3d > vecVerts;
        vector< Vector3d > vecNorms;
        vector< Vector3d > boneCoords;
        vector< double > vecIsos;
        
        
        /* This maps indicies in vecVerts/vecNorms/boneCoords to verticies in the VBO buffer */
        vector< vector <int> > VindexMap;

        /* This maps indicies in vecVerts/vecNorms/boneCoords to the neighboring points */
        vector< vector <int> > edgeMap;

        /* This vector is half the size as edgeMap since it gives barycentric coordinates for each pair*/
        vector< vector <double> > baryCoords;

        /* the average hrbf iso value of all the verticies */
        double avg_iso;

        /* the center and orientation of the bone's cylindrical coordinate system */
        Vector3d origin;
        Vector3d x_axis;
        Vector3d y_axis;
        Vector3d z_axis;

        Mesh();
        Mesh(const char* shaderFile);
        virtual ~Mesh(void);
        
        void draw();
        int  loadShader(const char* vertexFileName, const char* fragmentFileName);
        void set(const char* fileName);
        void setView(const char* fileName);
        void generateVerticies();
        void generateHrbfCoefs();
        void generateBaryCoords();
        void writeHrbf();
        void readHrbf();
        void boneCalc();
        void tangentalRelax(int iterations);
        void regenVerts();
        void transform(Matrix3d rot);

        double hrbfFunct(Vector3d x);
        double compositionHrbf(Vector3d x);
        Vector3d hrbfGrad(Vector3d x);
        

        vector< Mesh* > neighbors;
        vector< vector< int > >neighborIndex;
        /* determines which verticies correspond and finds the tangents to them as well */
        static void createUnion(Mesh* m1, Mesh* m2);

        double l(Vector3d x, int ind){
          return (x - vecVerts[ind]).norm();
        }

        double phi(double t){
          return t*t*t;
        }

        double dphi(double t){
          return 3*t*t;
        }

        double ddphi(double t){
          return 6*t;
        }

        double b(Vector3d x, int ind){
          return dphi(l(x, ind));
        }

        double c(Vector3d x, int ind){
          double dist = l(x,ind);
          if (dist == 0)
            return 0;
          return ( (1/(dist*dist)) * (ddphi(dist) - (dphi(dist)/dist)) );
        }

        void save_face(int f1, int f2, int f3){
          /*bool found1 = false, found2 = false;
          for(auto it = edgeMap[f1].begin(); it != edgeMap[f1].end(); ++it) {
              if(*it == f2)
                found1 = true;
              else if(*it == f3)
                found2 = true;
          }*/
          //if(!found1)
            edgeMap[f1].push_back(f2);
         // if(!found2)
            edgeMap[f1].push_back(f3);

            //we want to repeat since we need to know adjacent edges
        }

        double tsin(int x) { return precomputed_sin[x & 360]; }
        double tcos(int x) { return precomputed_sin[(x + 90) & 360]; }

};

#endif