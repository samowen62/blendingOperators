#ifndef _MAIN_H_
#define _MAIN_H_

#include "inc.h"
#include "cleanup.h"
#include "mesh.h"
//viewer requires nanovg
//#include <igl/viewer/Viewer.h>

#define M_PI  3.14159265358979323846  /* pi */

class App {
    private:
	    bool Running; 
       
      SDL_Window* win;
      SDL_Renderer* ren;
      SDL_GLContext maincontext;
      SDL_RendererInfo renderInfo;
      std::string meshFile;

      const Uint32 fps = 40;
      const Uint32 minframetime = 1000 / fps;
      const double step = 0.03;

      vector< Mesh* > meshes;

      int aroundZ = 0;
      int aroundX = 0;
      float zoom = 0.f;
      float zoomY = 0.f;
      float zoomZ = -0.5;
      int w=900,h=675;

      vector<bool> k_down = {0,0,0,0,0,0,0,0,0,0};

      Eigen::Matrix3d forwardRot;
      Eigen::Matrix3d backRot;


    public: 
        double alpha, lambda;

        App();
 
        virtual ~App(void);
        int OnExecute();
 
        bool OnInit();
        void OnEvent(SDL_Event* Event);
        void OnLoop();
        void OnRender();
        void OnCleanup();
        void setFile(const char* file);
        void key_read();
        void initializeMesh();

};
 
#endif


