#ifndef _MESH_H_
#define _MESH_H_

#include <Eigen/Dense>
#include "inc.h"
#include <fstream>
#include <string.h>

#define M_PI           3.14159265358979323846  /* pi */

using namespace std;
using namespace Eigen;

struct VBOvertex
{
  float x, y, z;        // Vertex
  float nx, ny, nz;     // Normal
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
      GLuint  VertexShader;
      GLuint  FragmentShader;

      string base;
      int buff_size;
      double precomputed_sin[360];

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

        GLuint  ShaderProgram;
        vector< VBOvertex > verticies;
        vector< Vector3f > vecVerts;
        vector< Vector3f > boneCoords;
        //need vector for storing iso-values of the point
        
        /* this points an index in the above array to actual vertex data in the buffer */
        vector< vector <int> > VindexMap;

        /* the average hrbf iso value of all the verticies */
        float avg_iso;

        /* the center and orientation of the bone's cylindrical coordinate system */
        Vector3f origin;
        Vector3f up;
        Vector3f x_axis;

        Mesh();
        Mesh(const char* shaderFile);
        virtual ~Mesh(void);
        
        void draw();
        int  loadShader(const char* vertexFileName, const char* fragmentFileName);
        void set(const char* fileName);
        void generateVerticies();
        void generateHrbfCoefs();
        void writeHrbf();
        void readHrbf();
        void boneCalc();
        float hrbfFunct(Vector3f x);
        

        float l(Vector3f x, int ind){
          return (x - vecVerts[ind]).norm();
        }

        float phi(float t){
          return t*t*t;
        }

        float dphi(float t){
          return 3*t*t;
        }

        float ddphi(float t){
          return 6*t;
        }

        float b(Vector3f x, int ind){
          return dphi(l(x, ind));
        }

        float c(Vector3f x, int ind){
          float dist = l(x,ind);
          if (dist == 0)
            return 0;
          return ( (1/(dist*dist)) * (ddphi(dist) - (dphi(dist)/dist)) );
        }

        void save_Vdata(int f,int j){
          VindexMap[f].push_back(j);
        }

        double tsin(int x) { return precomputed_sin[x & 360]; }
        double tcos(int x) { return precomputed_sin[(x + 90) & 360]; }

};

#endif