#include "main.h"
 
App::App() {
    win = NULL;
    ren = NULL;

	Running = true;
}
 
int App::OnExecute() {
	if(OnInit() == false) {
        std::cout << "error" << std::endl;
    	return -1;
	}

	SDL_Event Event;
    Uint32 frametime;

	while(Running) {
        frametime = SDL_GetTicks ();

    	while(SDL_PollEvent(&Event)) {
        		OnEvent(&Event);
    	}

        if (SDL_GetTicks () - frametime < minframetime)
            SDL_Delay (minframetime - (SDL_GetTicks () - frametime));

    	OnRender();
	}

	OnCleanup();
	return 0;
}

void App::setFile(const char* file){
  meshFile.append(file);
}
/*
 *  Generates an .hrbf file with alpha and beta coefficients corresponding
 *  to the order of verticies in the original obj file
 *
 *  obj: name of .obj file
 */
void genHrbf(const char* obj){
  Mesh source;
  source.set(obj);
  source.generateVerticies();
  source.generateHrbfCoefs();
  source.writeHrbf();
  return;
}

int main(int argc, char* argv[]) {
    std::string mode;
    mode.append(argv[1]);

    if(mode == "viewer"){

      if(argc < 3){
        fprintf(stderr, "Usage: ./out viewer <file>.obj\n" );
        exit(1);
      }

      App theApp;
      theApp.setFile(argv[2]);
 
      return theApp.OnExecute();
    }else if(mode == "hrbf"){
      if(argc < 3){
        fprintf(stderr, "Usage: ./out hrbf <file>.obj\n" );
        exit(1);
      }

      genHrbf(argv[2]);
      //seg faults for some reason?

    }else{
      fprintf(stderr, "Usage: ./out [viewer/hrbf] <file>.obj\n" );
      exit(1);
    }
    return 0;
}



bool App::OnInit() {
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        return false;
    }
 
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) /* Initialize SDL's Video subsystem */
        return false;
 
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
 
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
 
 
    SDL_CreateWindowAndRenderer(800, 600, SDL_WINDOW_OPENGL, &win, &ren);
    SDL_GL_CreateContext(win);
    SDL_GetRendererInfo(ren, &renderInfo);
    if ((renderInfo.flags & SDL_RENDERER_ACCELERATED) == 0 || 
        (renderInfo.flags & SDL_RENDERER_TARGETTEXTURE) == 0) {
    }
 
    SDL_GL_SetSwapInterval(1);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearDepth( 1.0f );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat lmodel_ambient[] = { 0.0, 0.0, 0.0, 0.0 };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    glFrontFace( GL_CCW );

    mesh = new Mesh("Shader1");
    mesh->set(meshFile.c_str());
    mesh->generateVerticies();

    float ratio = (float) w / (float) h;

    glClearColor ( 0.8, 0.8, 0.9, 1.0 );

    glViewport( 0, 0, ( GLsizei )w, ( GLsizei )h );

    /* change to the projection matrix and set our viewing volume. */
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
    gluPerspective( 45.0f, ratio, 0.1f, 1000.0f );
    glMatrixMode( GL_MODELVIEW );

    glLoadIdentity( );

    return true;
}

void App::OnEvent(SDL_Event* Event) {
    if(Event->type == SDL_QUIT) {
        Running = false;
    }

    if(Event->type == SDL_KEYUP){
        switch(Event->key.keysym.sym){
          case SDLK_a:
             aroundZ = (aroundZ + 3) % 360;
             break;
          case SDLK_d:
             aroundZ = (aroundZ - 3) % 360;
             break;
          case SDLK_w:
             aroundX = (aroundX + 3) % 360;
             break;
          case SDLK_s:
             aroundX = (aroundX - 3) % 360;
             break;
          case SDLK_z:
             zoom += 0.1;
             break;
          case SDLK_x:
             zoom -= 0.1;
             break;
          case SDLK_c:
             zoomY += 0.1;
             break;
          case SDLK_v:
             zoomY -= 0.1;
             break;
          case SDLK_b:
             zoomZ += 0.1;
             break;
          case SDLK_n:
             zoomZ -= 0.1;
             break;

       }
       //fprintf(stdout, "%f, %f, %f\n", zoom, zoomY, zoomZ);
    }
}

void App::OnRender() {

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

/*  Eigen::Matrix4f proj = Eigen::Matrix4f::Identity();
    float near = 0.01;
    float far = 100;
    float top = tan(35./360.*M_PI)*near;
    float right = top * w/h;
    igl::frustum(-right,right,-top,top,near,far,proj);
    Eigen::Affine3f model = Eigen::Affine3f::Identity();
    model.translate(Eigen::Vector3f(zoom,zoomY,-5));
    model.rotate(Eigen::AngleAxisf(aroundX,Eigen::Vector3f(0,1,0)));

*/

    glLoadIdentity();
    glTranslatef(0.f,0.f,zoomZ);
    glTranslatef(-zoom,0.f,0.f);
    glTranslatef(0.f,-zoomY,0.f);
    glRotatef(aroundZ, 0.f, 0.f, 1.f);
    glRotatef(aroundX, 0.f, 1.f, 0.f);

    //tipMesh->draw(proj, model);
    mesh->draw();

    SDL_RenderPresent(ren);
    SDL_GL_SwapWindow(win);
}

void App::OnCleanup() {
    cleanup(ren,win);
    SDL_Quit();
}

App::~App(void)
{
    OnCleanup();
}

