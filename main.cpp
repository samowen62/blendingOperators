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
  source.generateHrbfCoefs();
  source.writeHrbf();
  return;
}

int main(int argc, char* argv[]) {
    std::string mode;
    mode.append(argv[1]);

    if(mode == "viewer"){

      if(argc < 4){
        fprintf(stderr, "Usage: ./out viewer <float> <float>\n" );
        exit(1);
      }

      App viewer;
      viewer.alpha = stod(argv[2]);
      viewer.lambda = stod(argv[3]);
 
      return viewer.OnExecute();
    }else if(mode == "hrbf"){
      if(argc < 3){
        fprintf(stderr, "Usage: ./out hrbf objs/<file>.obj\n" );
        exit(1);
      }

      genHrbf(argv[2]);
      //seg faults for some reason?

    }else{
      fprintf(stderr, "Usage: ./out [viewer/hrbf] [<double>/<file>.obj]\n" );
      exit(1);
    }
    return 0;
}

void App::initializeMesh(){
    meshes.resize(2);

    //TODO: wrap mesh class with boost and then just call python script in main
    meshes[0] = new Mesh(alpha, lambda, "forearm", "shader1");
    meshes[1] = new Mesh(alpha, lambda, "arm", "shader1");

    if(0 > meshes[0]->setCompParams(0, M_PI/2, M_PI, M_PI/4, 0, M_PI/4, 1, 1)){
      fprintf(stderr, "Bad Composition Parameters\n" );
      exit(1);
    };
    //meshes[0] = new Mesh(alpha, "fingerTip", "shader1");
    //meshes[1] = new Mesh(alpha, "fingerMid", "shader1");

    
    //put this before any rotation since the verticies and normals change
    Mesh::createUnion(meshes[0], meshes[1]);

    //this method should only be called on meshes[i] if all desired unions with meshes[i] have been made
    meshes[0]->generateBaryCoords();
    meshes[1]->generateBaryCoords();

    Vector3d axis = meshes[0]->z_axis.cross(meshes[1]->z_axis).normalized();
    float theta = -0.5;//as a factor of pi

    meshes[0]->transform(CLEAN_UNION_COMP, theta, -1.0, axis);

    //./out viewer 0.01 0.4   for arm           cleanUnionComp
    //./out viewer 0.0001 0.4 for fingerTip   cleanUnionComp
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
 
 
    SDL_CreateWindowAndRenderer(w, h, SDL_WINDOW_OPENGL, &win, &ren);
    SDL_GL_CreateContext(win);
    SDL_GetRendererInfo(ren, &renderInfo);
    if ((renderInfo.flags & SDL_RENDERER_ACCELERATED) == 0 ||  (renderInfo.flags & SDL_RENDERER_TARGETTEXTURE) == 0) {
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

    //comment out for regular surfaces
    glPolygonMode(GL_FRONT, GL_LINE);
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    glFrontFace( GL_CCW );

    initializeMesh();
    
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
          k_down[0] = false;
          break;
        case SDLK_d:
          k_down[1] = false;
          break;
        case SDLK_w:
          k_down[2] = false;
          break;
        case SDLK_s:
          k_down[3] = false;
          break;
        case SDLK_z:
          k_down[4] = false;
          break;
        case SDLK_x:
          k_down[5] = false;
          break;
        case SDLK_c:
          k_down[6] = false;
          break;
        case SDLK_v:
          k_down[7] = false;
          break;
        case SDLK_b:
          k_down[8] = false;
          break;
        case SDLK_n:
          k_down[9] = false;
          break;

       }
    }
    else if(Event->type == SDL_KEYDOWN){
      switch(Event->key.keysym.sym){
        case SDLK_a:
          k_down[0] = true;
          break;
        case SDLK_d:
          k_down[1] = true;
          break;
        case SDLK_w:
          k_down[2] = true;
          break;
        case SDLK_s:
          k_down[3] = true;
          break;
        case SDLK_z:
          k_down[4] = true;
          break;
        case SDLK_x:
          k_down[5] = true;
          break;
        case SDLK_c:
          k_down[6] = true;
          break;
        case SDLK_v:
          k_down[7] = true;
          break;
        case SDLK_b:
          k_down[8] = true;
          break;
        case SDLK_n:
          k_down[9] = true;
          break;

       }
    }
}

void App::key_read(){
    if(k_down[0])
      aroundZ = (aroundZ + 3) % 360;
    if(k_down[1])
      aroundZ = (aroundZ - 3) % 360;
    if(k_down[2])
      aroundX = (aroundX + 3) % 360;
    if(k_down[3])
      aroundX = (aroundX - 3) % 360;
    if(k_down[4])
      zoom += step;
    if(k_down[5])
      zoom -= step;
    if(k_down[6])
      zoomY += step;
    if(k_down[7])
      zoomY -= step;
    if(k_down[8])
      zoomZ += step;
    if(k_down[9])
      zoomZ -= step;
}

void App::OnRender() {
    key_read();

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    glTranslatef(0.f,0.f,zoomZ);
    glTranslatef(-zoom,0.f,0.f);
    glTranslatef(0.f,-zoomY,0.f);
    glRotatef(aroundZ, 0.f, 0.f, 1.f);
    glRotatef(aroundX, 0.f, 1.f, 0.f);

    for(auto it = meshes.begin(); it != meshes.end(); ++it)
      (*it)->draw();

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

