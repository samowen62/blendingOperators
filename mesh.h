#ifndef _MESH_H_
#define _MESH_H_

#include <Eigen/Dense>
#include "inc.h"
#include <fstream>
#include <string.h>

#define EDGE_ERROR      0.0015

using namespace std;
using namespace Eigen;

//TODO: eventually change draw to make it use this as an actual VBO
struct VBOvertex
{
  double x, y, z;        // Vertex
  double nx, ny, nz;     // Normal
};

/*
 *  This struct is created to make the parameters for interpolation composition more clear
 *  The details of these are outlined in Gourmel et. al.
 */
struct CompParams
{
  double a_0, a_1, a_2, theta_0, theta_1, theta_2, w_0, w_1;
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

//function used in the interpolating composition. Made inline for clarity
inline double kappa(double x, double w){
  pow(1 - exp(1 - ( 1/( 1 - exp(1 - (1/x) ) ) ) ), w);
}


class Mesh {
    private:
      GLuint  ShaderProgram;
      GLuint  VertexShader;
      GLuint  FragmentShader;

      string base;
      int buff_size;
      vector< VBOvertex > verticies;

      MatrixXd A;
      MatrixXd B;
      /* used to both generate the coefficients and store them when read in */
      MatrixXd alpha_beta;

    public:  
        //make more of these private but use friend class mesh
        int num_verts;

        vector< Vector3d > vecVerts;
        vector< Vector3d > vecNorms;
        vector< Vector3d > boneCoords;
        vector< double > vecIsos;
        
        
        /* This maps indicies in vecVerts/vecNorms/boneCoords to verticies in the VBO buffer */
        vector< vector <int> > VindexMap;

        /* This maps indicies in vecVerts/vecNorms/boneCoords to the neighboring points */
        vector< vector <int> > edgeMap;

        /* This vector is half the size as edgeMap since it gives barycentric coordinates for each pair */
        vector< vector <double> > baryCoords;

        /* the average hrbf iso value of all the verticies */
        double avg_iso;

        /* user specified parameter (0.35 in Vaillant) that controls the degree we rely on isosurface tracking */
        double alpha = 0.35;

        /* user specified parameter that controls how much we rely on neighboring verticies when smoothing */
        double lambda = 0.3;

        CompParams comp_params;

        /* the center and orientation of the bone's cylindrical coordinate system */
        Vector3d origin;
        Vector3d x_axis;
        Vector3d y_axis;
        Vector3d z_axis;

        Mesh();
        Mesh(double _alpha, double _lambda, const char* _base, const char* shaderFile);
        void construct();
        virtual ~Mesh(void);
        
        int  loadShader(const char* vertexFileName, const char* fragmentFileName);
        int  setCompParams(double _a_0, double _a_1, double _a_2, double _theta_0, double _theta_1, double _theta_2, double _w_0, double _w_1);
        void set(const char* fileName);
        void setView(const char* fileName);
        void generateHrbfCoefs();
        void generateBaryCoords();
        void generateBoneCoords();
        void writeHrbf();
        void readHrbf();
        void regenVerts();
        void draw();

        void transform(int _type, int relax_steps, float factor, float param, Vector3d axis);


        double hrbfFunct(Vector3d x);
        //_Scalar hrbfFunct(Vector3d x);
        Vector3d hrbfGrad(Vector3d x);

        //simple blending operators
        double _union_comp(Vector3d x, int neighbor_ind, float t){
          return max(hrbfFunct(x), neighbors[neighbor_ind]->hrbfFunct(x));
        }

        double _average_comp(Vector3d x, int neighbor_ind, float t){
          return (hrbfFunct(x) + neighbors[neighbor_ind]->hrbfFunct(x)) / 2.0;
        }

        double _clean_union_comp(Vector3d x, int neighbor_ind, float t){
          double  f_1 = hrbfFunct(x),
              f_2 = neighbors[neighbor_ind]->hrbfFunct(x);
          return f_1 + f_2 - sqrt(f_1*f_1 + f_2*f_2);
        }

        double _bending_comp(Vector3d x, int neighbor_ind, float t){
          t = t == 0 ? 1 : t;
          return pow(pow(hrbfFunct(x), t) + pow(neighbors[neighbor_ind]->hrbfFunct(x), t), 1/t);
        }

        double _interpolating_comp(Vector3d x, int neighbor_ind, float t){
          Vector3d grad_0 = hrbfGrad(x), grad_1 = neighbors[neighbor_ind]->hrbfGrad(x);
          double den = grad_0.norm() * grad_1.norm();

          if(den == 0){
            cout << "Error: norm of gradients are 0" << endl;
            return 1.0;
          }

          double alpha = acos(grad_0.dot(grad_1) / den);
          //cout << alpha << endl;

          double theta = comp_params.theta_2;

          if(comp_params.a_0 >= alpha){
            theta = comp_params.theta_0;
          }else if(comp_params.a_1 >= alpha){
            theta = kappa((alpha - comp_params.a_1)/(comp_params.a_0 - comp_params.a_1), comp_params.w_0);
            theta *= (comp_params.theta_0 - comp_params.theta_1);
            theta += comp_params.theta_1;
          }else if(comp_params.a_2 >= alpha){
            theta = kappa((alpha - comp_params.a_1)/(comp_params.a_2 - comp_params.a_1), comp_params.w_1);
            theta *= (comp_params.theta_2 - comp_params.theta_1);
            theta += comp_params.theta_1;
          }

          double  f_1 = hrbfFunct(x),
                  f_2 = neighbors[neighbor_ind]->hrbfFunct(x),
                  factor = (theta * 4)/M_PI;

          double  _max = max(f_1, f_2),
                  blend = pow(pow(f_1, t) + pow(f_2, t), 1/t);

          //cout << "f: " << factor << endl;

          //need to restrict this to values near joints
          return factor*_max + (1 - factor)*blend;

        }


        typedef double (Mesh::*comp_funct)(Vector3d,int,float);

        comp_funct unionComp = &Mesh::_union_comp;
        comp_funct averageComp = &Mesh::_average_comp;
        comp_funct cleanUnionComp = &Mesh::_clean_union_comp;
        comp_funct blendingComp = &Mesh::_bending_comp;
        comp_funct interpolatingComp = &Mesh::_interpolating_comp;

        void tangentalRelax(int iterations, comp_funct g, int neighbor, float param);

        vector< comp_funct > composition_functions;

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
          edgeMap[f1].push_back(f2);
          edgeMap[f1].push_back(f3);
        }

        //double tsin(int x) { return precomputed_sin[x & 360]; }
        //double tcos(int x) { return precomputed_sin[(x + 90) & 360]; }

};

#endif