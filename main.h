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

      const Uint32 fps = 40;
      const Uint32 minframetime = 1000 / fps;

      Mesh* tipMesh;

      int aroundZ = 0;
      int aroundX = 0;
      float zoom = 0.f;//100
      float zoomY = 0.f;
      float zoomZ = -0.5;
      int w=800,h=600;
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

};
 
#endif


