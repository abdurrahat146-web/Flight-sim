#include <jni.h>
#include <android/native_window_jni.h>
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <thread>
#include <atomic>
#include <vector>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <string>

struct V{float x,y,z;};
static std::atomic<bool> run{false}; static ANativeWindow* win=nullptr; static std::thread th;
static float ctlX=.5f, ctlY=.5f; static bool down=false;
struct State{float speed=140,alt=2500,pitch=0,roll=0,heading=90,thr=.55f;} s;
static GLuint prog=0,vbo=0; static GLint mvpLoc=-1,colorLoc=-1; static EGLDisplay ed; static EGLSurface es; static EGLContext ec;
static float M[16];
static void mul(const float*a,const float*b,float*r){float t[16];for(int c=0;c<4;c++)for(int r0=0;r0<4;r0++)t[c*4+r0]=a[0*4+r0]*b[c*4+0]+a[1*4+r0]*b[c*4+1]+a[2*4+r0]*b[c*4+2]+a[3*4+r0]*b[c*4+3];for(int i=0;i<16;i++)r[i]=t[i];}
static void perspective(float fovy,float asp,float zn,float zf){float f=1/tanf(fovy*.5f),q=zf/(zn-zf);for(int i=0;i<16;i++)M[i]=0;M[0]=f/asp;M[5]=f;M[10]=q;M[11]=-1;M[14]=q*zn;}
static void translate(float x,float y,float z){M[12]+=M[0]*x+M[4]*y+M[8]*z;M[13]+=M[1]*x+M[5]*y+M[9]*z;M[14]+=M[2]*x+M[6]*y+M[10]*z;}
static GLuint shader(GLenum t,const char*src){GLuint q=glCreateShader(t);glShaderSource(q,1,&src,nullptr);glCompileShader(q);return q;}
static void initGL(){
 const char*vs="#version 300 es\nlayout(location=0)in vec3 p;uniform mat4 u;void main(){gl_Position=u*vec4(p,1.0);}";
 const char*fs="#version 300 es\nprecision mediump float;uniform vec4 c;out vec4 o;void main(){o=c;}";
 GLuint a=shader(GL_VERTEX_SHADER,vs),b=shader(GL_FRAGMENT_SHADER,fs);prog=glCreateProgram();glAttachShader(prog,a);glAttachShader(prog,b);glLinkProgram(prog);glDeleteShader(a);glDeleteShader(b);mvpLoc=glGetUniformLocation(prog,"u");colorLoc=glGetUniformLocation(prog,"c");
 glEnable(GL_DEPTH_TEST); glEnable(GL_CULL_FACE); glGenBuffers(1,&vbo);
}
static void cube(float x,float y,float z,float sx,float sy,float sz,float cr,float cg,float cb){
 V v[]={{-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},{-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1}}; unsigned id[]={0,1,2,2,3,0,1,5,6,6,2,1,5,4,7,7,6,5,4,0,3,3,7,4,3,2,6,6,7,3,4,5,1,1,0,4}; V out[36];for(int i=0;i<36;i++)out[i]={v[id[i]].x*sx+x,v[id[i]].y*sy+y,v[id[i]].z*sz+z};glBufferData(GL_ARRAY_BUFFER,sizeof(out),out,GL_DYNAMIC_DRAW);glVertexAttribPointer(0,3,GL_FLOAT,0,sizeof(V),nullptr);glEnableVertexAttribArray(0);glUniform4f(colorLoc,cr,cg,cb,1);glDrawArrays(GL_TRIANGLES,0,36);
}
static void draw(){
 glClearColor(.28f,.55f,.82f,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glUseProgram(prog);glBindBuffer(GL_ARRAY_BUFFER,vbo);
 int w=0,h=0; // viewport already set
 // Camera projection + simple cockpit/world transform
 float P[16];float aspect=1.777f;perspective(1.05f,aspect,.1f,5000);for(int i=0;i<16;i++)P[i]=M[i];
 // terrain
 glUniformMatrix4fv(mvpLoc,1,0,P);cube(0,-8,-900,1800,5,1800,.20f,.38f,.16f);
 // runway slabs
 cube(0,-1,-650,55,1,650,.10f,.10f,.11f);for(int i=0;i<12;i++)cube(0,.1f,-70-i*100,4,.15f,18,1,1,1);
 // aircraft centered ahead
 cube(0,0,-8,2,1,8,.85f,.87f,.90f);cube(0,0,-3,9,.25f,2,.72f,.74f,.78f);cube(0,1.2f,3,5,.18f,1,.70f,.72f,.75f);
 // engine nacelles
 cube(-5,-.4f,-3,1.3f,1.0f,2,.35f,.36f,.38f);cube(5,-.4f,-3,1.3f,1.0f,2,.35f,.36f,.38f);
}
static void loop(){
 EGLint cfgs;ed=eglGetDisplay(EGL_DEFAULT_DISPLAY);eglInitialize(ed,nullptr,nullptr);const EGLint ca[]={EGL_RENDERABLE_TYPE,EGL_OPENGL_ES3_BIT,EGL_SURFACE_TYPE,EGL_WINDOW_BIT,EGL_BLUE_SIZE,8,EGL_GREEN_SIZE,8,EGL_RED_SIZE,8,EGL_DEPTH_SIZE,24,EGL_NONE};EGLConfig cfg;eglChooseConfig(ed,ca,&cfg,1,&cfgs);const EGLint ctx[]={EGL_CONTEXT_CLIENT_VERSION,3,EGL_NONE};ec=eglCreateContext(ed,cfg,EGL_NO_CONTEXT,ctx);es=eglCreateWindowSurface(ed,cfg,win,nullptr);eglMakeCurrent(ed,es,es,ec);initGL();glViewport(0,0,ANativeWindow_getWidth(win),ANativeWindow_getHeight(win));
 auto last=std::chrono::steady_clock::now();while(run){auto now=std::chrono::steady_clock::now();float dt=std::chrono::duration<float>(now-last).count();last=now;
  float target=80+s.thr*240;s.speed+=(target-s.speed)*std::min(dt*.25f,1.f);s.pitch+=(.5f-ctlY)*dt*.7f;s.pitch=std::clamp(s.pitch,-.6f,.6f);s.roll+=(ctlX-.5f)*dt*2.5f;s.roll=std::clamp(s.roll,-.9f,.9f);s.alt+=s.speed*.5144f*std::sin(s.pitch)*dt;s.alt=std::max(0.f,s.alt);s.heading=fmodf(s.heading+s.roll*20*dt+360,360);
  draw();eglSwapBuffers(ed,es);std::this_thread::sleep_for(std::chrono::milliseconds(8));}
 eglMakeCurrent(ed,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);eglDestroySurface(ed,es);eglDestroyContext(ed,ec);eglTerminate(ed);ANativeWindow_release(win);win=nullptr;
}
extern "C" JNIEXPORT void JNICALL Java_com_rahu_flightsim_MainActivity_nativeStart(JNIEnv*e,jobject,jobject surf){if(run)return;win=ANativeWindow_fromSurface(e,surf);run=true;th=std::thread(loop);th.detach();}
extern "C" JNIEXPORT void JNICALL Java_com_rahu_flightsim_MainActivity_nativeStop(JNIEnv*,jobject){run=false;}
extern "C" JNIEXPORT void JNICALL Java_com_rahu_flightsim_MainActivity_nativeControl(JNIEnv*,jobject,jfloat x,jfloat y,jboolean d){ctlX=x;ctlY=y;down=d;if(y>.65)s.thr=std::clamp(1-y,.05f,.95f);}
