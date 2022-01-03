#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#define get_proc(name) glXGetProcAddress((const GLubyte *)name)


PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLUSEPROGRAMPROC glUseProgram;

Atom WM_DELETE_WINDOW = {0};


int
main(int argc, char * argv[])
{

  Display * display = XOpenDisplay(NULL);
  Window window = XCreateSimpleWindow(
    display,
    DefaultRootWindow(display),
    10, 10,
    800, 600,
    0, 0,
    0
  );

  XMapWindow(display, window);

  glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)get_proc(
    "glXCreateContextAttribsARB"
  );

  static int visual_attribs[] = {
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_DOUBLEBUFFER, true,
    GLX_RED_SIZE, 1,
    GLX_GREEN_SIZE, 1,
    GLX_BLUE_SIZE, 1,
    None
  };

  int num_fbc = 0;
  GLXFBConfig * fbc = glXChooseFBConfig(
    display,
    DefaultScreen(display),
    visual_attribs,
    &num_fbc
  );

  if (!fbc) {
    printf("%s\n", "glXChooseFBConfig() failed");
    return EXIT_FAILURE;
  }

  static int context_attribs[] = {
    GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
    GLX_CONTEXT_MINOR_VERSION_ARB, 4,
    None
  };

  GLXContext context = glXCreateContextAttribsARB(
    display,
    fbc[0],
    NULL,
    true,
    context_attribs
  );

  if (!context) {
    printf("%s\n", "Failed to create OpenGL context");
    return EXIT_FAILURE;
  }

  glXMakeCurrent(display, window, context);
  int major = 0, minor = 0;
  glGetIntegerv(GL_MAJOR_VERSION, &major);
  glGetIntegerv(GL_MINOR_VERSION, &minor);

  printf("OpenGL context created.\nVersion %d.%d\nVendor %s\nRenderer %s\n",
      major, minor, glGetString(GL_VENDOR), glGetString(GL_RENDERER)
  );

  glCreateShader = (PFNGLCREATESHADERPROC)get_proc("glCreateShader");
  glShaderSource = (PFNGLSHADERSOURCEPROC)get_proc("glShaderSource");
  glCompileShader = (PFNGLCOMPILESHADERPROC)get_proc("glCompileShader");
  glGetShaderiv = (PFNGLGETSHADERIVPROC)get_proc("glGetShaderiv");
  glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)get_proc("glGetShaderInfoLog");

  int shader_success = 0;
  char shader_info_log[512] = {0};

  const char * source_shader_vertex = \
    "#version 330 core\n"
    "layout (location=0) in vec3 aPos;\n"
    "void main() {\n"
    "  gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "};";

  GLuint shader_vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(shader_vertex, 1, &source_shader_vertex, NULL);
  glCompileShader(shader_vertex);

  glGetShaderiv(shader_vertex, GL_COMPILE_STATUS, &shader_success);
  if (!shader_success) {
    glGetShaderInfoLog(shader_vertex, 512, NULL, shader_info_log);
    printf("Failed compiling vertex shader: %s\n", shader_info_log);
    return EXIT_FAILURE;
  }

  const char * source_shader_fragment = \
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "  FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0);\n"
    "};";

  GLuint shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(shader_fragment, 1, &source_shader_fragment, NULL);
  glCompileShader(shader_fragment);

  glGetShaderiv(shader_fragment, GL_COMPILE_STATUS, &shader_success);
  if (!shader_success) {
    glGetShaderInfoLog(shader_fragment, 512, NULL, shader_info_log);
    printf("Failed compiling fragment shader: %s\n", shader_info_log);
    return EXIT_FAILURE;
  }

  glCreateProgram = (PFNGLCREATEPROGRAMPROC)get_proc("glCreateProgram");
  glAttachShader = (PFNGLATTACHSHADERPROC)get_proc("glAttachShader");
  glLinkProgram = (PFNGLLINKPROGRAMPROC)get_proc("glLinkProgram");
  glGetProgramiv = (PFNGLGETPROGRAMIVPROC)get_proc("glGetProgramiv");
  glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)get_proc("glGetProgramInfoLog");
  glDeleteShader = (PFNGLDELETESHADERPROC)get_proc("glDeleteShader");

  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, shader_vertex);
  glAttachShader(shader_program, shader_fragment);
  glLinkProgram(shader_program);
  glGetProgramiv(shader_program, GL_LINK_STATUS, &shader_success);

  if (!shader_success) {
    glGetProgramInfoLog(shader_program, 512, NULL, shader_info_log);
    printf("Failed to link program: %s\n", shader_info_log);
    return EXIT_FAILURE;
  }
  glDeleteShader(shader_vertex);
  glDeleteShader(shader_fragment);

  glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)get_proc("glGenVertexArrays");
  glGenBuffers = (PFNGLGENBUFFERSPROC)get_proc("glGenBuffers");
  glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)get_proc("glBindVertexArray");
  glBindBuffer = (PFNGLBINDBUFFERPROC)get_proc("glBindBuffer");
  glBufferData = (PFNGLBUFFERDATAPROC)get_proc("glBufferData");
  glVertexAttribPointer = \
    (PFNGLVERTEXATTRIBPOINTERPROC)get_proc("glVertexAttribPointer");
  glEnableVertexAttribArray = \
    (PFNGLENABLEVERTEXATTRIBARRAYPROC)get_proc("glEnableVertexAttribArray");
  glUseProgram = (PFNGLUSEPROGRAMPROC)get_proc("glUseProgram");


  GLfloat verts[] = {
    -1.0f,  1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
  };

  GLuint VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(
    GL_ARRAY_BUFFER,
    sizeof(verts),
    verts,
    GL_STATIC_DRAW
  );

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  WM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", false);

  XEvent event = {0};
  XKeyEvent * event_key = NULL;
  #define size_buff_key_name 10
  char buff_key_name[size_buff_key_name] = {0};

  Atom protocols[] = {WM_DELETE_WINDOW};
  XSetWMProtocols(display, window, protocols, sizeof(protocols)/sizeof(Atom));

  XSelectInput(display, window, KeyPressMask | KeyReleaseMask | ButtonPressMask | ExposureMask);
  XClientMessageEvent * client_message = NULL;

  bool window_should_close = false;
  while (!window_should_close) {

    XFlush(display);
    while(XPending(display)) {
      XNextEvent(display, &event);
      switch (event.type) {
        case ClientMessage:
          client_message = (XClientMessageEvent*)&event;
          if (client_message->data.l[0] == WM_DELETE_WINDOW) {
            window_should_close = true;
          }
          break;
        case KeyPress:
        case KeyRelease:
          event_key = (XKeyEvent *)&event;
          XLookupString(event_key, buff_key_name, size_buff_key_name, NULL, NULL);
          printf(
            "key (%d)\t%s\twas %s\tstate: %d\n",
            event_key->keycode,
            buff_key_name,
            event.type == KeyPress ? "pressed" : "released",
            event_key->state
          );
          if (event_key->keycode == 24) { // 'q' key.
            window_should_close = true;
          }
          break;
      }
    }
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader_program);
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, 2);
    glXSwapBuffers(display, window);
  }
}
