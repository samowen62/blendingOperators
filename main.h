#ifndef _MAIN_H_
#define _MAIN_H_

#include "inc.h"
#include "cleanup.h"
#include "mesh.h"
//viewer requires nanovg
//#include <igl/viewer/Viewer.h>
//using namespace mesh;

class App {
    private:
	   bool Running; 
       
       //SDL_Surface*    Surf_Display;
      SDL_Window* win;
      SDL_Renderer* ren;
      SDL_GLContext maincontext; /* Our opengl context handle */
      SDL_RendererInfo renderInfo;

      Eigen::MatrixXd V_tip;
      Eigen::MatrixXd TC_tip;
      Eigen::MatrixXd N_tip;
      Eigen::MatrixXi F_tip;
      Eigen::MatrixXi FTC_tip;
      Eigen::MatrixXi FN_tip;

      const Uint32 fps = 40;
      const Uint32 minframetime = 1000 / fps;

      Mesh* tipMesh;

      int aroundZ = 0;
      int aroundX = 0;
      int zoom = 0;//100
      int zoomY = 0;

      //ViewerData* vd;

    public: 
        App();
 
        virtual ~App(void);
        int OnExecute();
 
        bool OnInit();
        void OnEvent(SDL_Event* Event);
        void OnLoop();
        void OnRender();
        void OnCleanup();
        void tip();
        //void cube(float faces[9]);

};
 
#endif


