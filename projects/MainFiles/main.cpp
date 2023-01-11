// Include standard headers
#include "shaders.h"
#include "util.hpp"
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
using namespace glm;
#include <fstream>
#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

static bool imgui = true;
int pid = 0;
int res = 10;

struct point{
  float x;
  float y;
  float z;
};

struct curve{
  int curvetype = 0;
  int k = 3;
  std::vector<point>ctrlpts;
  std::vector<point>finalpts;
  std::vector<float>knots;
};

std::vector<curve>curves;

static bool
CapturedByGUI()
{
  ImGuiIO& io = ImGui::GetIO();
  return (io.WantCaptureMouse);
}

static void
ErrorCallback(int error, const char* description)
{
  fprintf(stderr, "Error: %s\n", description);
}

static void
KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    // close window
    //glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
  else if (key == GLFW_KEY_G && action == GLFW_PRESS) {
    //imgui = !imgui;
  }
}

static void
CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
  if (!CapturedByGUI()) {
    int left_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    int right_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    // left click
    if (left_state == GLFW_PRESS) {
    }
    else {
    }

    // right click
    if (right_state == GLFW_PRESS) {
    }
    else {
    }
  }
}

static void
WindowSizeCallback(GLFWwindow* window, int width, int height)
{
}



template<typename T>
T
SafeRead(std::ifstream& is)
{
  T ret;
  is >> ret;
  auto rdstate = is.rdstate();
  /*if ((rdstate & std::ifstream::eofbit) != 0)
    throw std::runtime_error("End-of-File reached on input operation");
  if ((rdstate & std::ifstream::failbit) != 0)
    throw std::runtime_error("Logical error on i/o operation");
  if ((rdstate & std::ifstream::badbit) != 0)
    throw std::runtime_error("Read/writing error on i/o operation");*/
  return ret;
}

void createKnots(){
  std::vector<float>knots;
  
  int n = curves[pid].k + curves[pid].ctrlpts.size();
  
  for(int i = 0;i<n;i++){
    knots.push_back(float(i));
  }

  curves[pid].knots = knots;

}

void readfile(const std::string& input){
  std::ifstream file(input);
  if (!file.is_open())
    throw std::runtime_error("cannot open input: " + input);

  int n = SafeRead<int>(file);
  std::cout << "there are " << n << " curves" << std::endl;

  curves.resize(n);

  for (auto& p : curves) {
    int m = SafeRead<int>(file);
    for (auto i = 0; i < m; ++i) {
      float x = SafeRead<float>(file), y = SafeRead<float>(file), z = 0.0;
      p.ctrlpts.push_back(point{x, y, z});
    }
  }
  file.close();

  createKnots();

}

void writefile(){
  std::ofstream myfile("output.txt");

  myfile<<curves.size();
  myfile<<"\n";

  for(int i = 0;i<curves.size();i++){
    myfile<<curves[i].ctrlpts.size();
    myfile<<"\n";
    for(int j = 0;j<curves[i].ctrlpts.size();j++){
      myfile<<curves[i].ctrlpts[j].x;
      myfile<<" ";
      myfile<<curves[i].ctrlpts[j].y;
      myfile<<"\n";
    }
  }
}

void deBoor(){

  int size = curves[pid].ctrlpts.size();
  int I = curves[pid].k - 1;
  int k = curves[pid].k;
  float ubar = curves[pid].knots[I];
  std::vector<point>push;
  std::vector<std::vector<point>>buildup;
  std::vector<point>finalspline;

  for(int i = 0;i<curves[pid].ctrlpts.size();i++){
    push.push_back(curves[pid].ctrlpts[i]);
  }

  buildup.push_back(push);

  for(int n = 0; n < size - curves[pid].k + 1;n++){
    float increment = (curves[pid].knots[I+1] - curves[pid].knots[I]) / res;

    for(int d = 0;d<res;d++){

      buildup.resize(1);

      for(int j = 1;j<=k-1;j++){
        buildup.resize(j+1);

        std::vector<point>temp;
        for(int i = I - (k-1);i<=I-j;i++){
          buildup[j].resize(i+1);
          point temp_point;
          
          temp_point.x = ((curves[pid].knots[i+k] - ubar) / (curves[pid].knots[i+k] - curves[pid].knots[i+j])) * buildup[j-1][i].x
                         + ((ubar - curves[pid].knots[i+j]) / (curves[pid].knots[i+k] - curves[pid].knots[i+j])) * buildup[j-1][i+1].x;

                   temp_point.y = ((curves[pid].knots[i+k] - ubar) / (curves[pid].knots[i+k] - curves[pid].knots[i+j])) * buildup[j-1][i].y
                         + ((ubar - curves[pid].knots[i+j]) / (curves[pid].knots[i+k] - curves[pid].knots[i+j])) * buildup[j-1][i+1].y;

          temp_point.z = 0.0;

          if (i == I - (k-1) and j == k-1){
            finalspline.push_back(temp_point);
          }

          temp.push_back(temp_point);
           buildup[j][i].x = temp_point.x;
           buildup[j][i].y = temp_point.y;
           buildup[j][i].z = temp_point.z;

        }
        
      }

            ubar += increment;

    }

    I = I + 1;
  }

  curves[pid].finalpts = finalspline;


}


void BezierCurve(){
  float t = 0.0;
  float increment = 1.0 / float(res);
  std::vector<point>push;
  int n = curves[pid].ctrlpts.size();
  std::vector<std::vector<point>>BuildUp;
  std::vector<point>final_points;

  for(int i = 0;i<curves[pid].ctrlpts.size();i++){
    push.push_back(curves[pid].ctrlpts[i]);
  }

  BuildUp.push_back(push);

  while(t < 1.0){
    BuildUp.resize(1);
    for(int j = 1;j<n;j++){
      std::vector<point>temp;
      for (int i = 0;i<n-j;i++){
        point temp_point;
        temp_point.x = (1-t)*(BuildUp[j-1][i].x) + (t)*(BuildUp[j-1][i+1].x);
        temp_point.y = (1-t)*(BuildUp[j-1][i].y) + (t)*(BuildUp[j-1][i+1].y);
        temp_point.z = 0.0;

        if(i == n - j -1 and j == n - 1)
          final_points.push_back(temp_point);

        temp.push_back(temp_point);
      }
      BuildUp.push_back(temp);
    }

    float check = t + increment;

    if(check > 1.0){
      t = 1.0;
    }
    else{
      t += increment;
    }

  }

  curves[pid].finalpts = final_points;

}

int
main(const int argc, const char** argv)
{

  if (argc < 2)
    throw std::runtime_error("missing input file");

  readfile(argv[1]);

  // Initialise GLFW
  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW");
  }

  const char* glsl_version = "#version 150";


  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  // We don't want the old OpenGL
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Open a window and create its OpenGL context
  window = glfwCreateWindow(1024, 768, "ECS 175 Demo (Press 'n'...)", NULL, NULL);
  if (window == NULL) {
    glfwTerminate();
    throw std::runtime_error("Failed to open GLFW window. If you have a GPU that is "
                             "not 3.3 compatible, try a lower OpenGL version.");
  }

  glfwSetKeyCallback(window, KeyCallback);
  glfwSetWindowSizeCallback(window, WindowSizeCallback);
  glfwSetCursorPosCallback(window, CursorPositionCallback);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  int err = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0;
  if (err) {
    throw std::runtime_error("Failed to initialize OpenGL loader!");
  }

  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);


  {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark(); // or ImGui::StyleColorsClassic();

    // Initialize Dear ImGui
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
  }

  // Load GLAD symbols

  // Ensure we can capture the escape key being pressed below

  // Dark blue background
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

  GLuint vertex_array_id;
  glGenVertexArrays(1, &vertex_array_id);
  glBindVertexArray(vertex_array_id);

  // Create and compile our GLSL program from the shaders
  GLuint program_id =
    load_program_from_embedding((char*)vertexshader, vertexshader_size, (char*)fragmentshader, fragmentshader_size);

  // Get a handle for our "MVP" uniform
  GLuint matrix_id = glGetUniformLocation(program_id, "Matrix");
  glm::mat4 matrix = { 1.086396,  -0.993682, -0.687368, -0.685994, 0.000000, 2.070171, -0.515526, -0.514496,
                       -1.448528, -0.745262, -0.515526, -0.514496, 0.000000, 0.000000, 5.642426,  5.830953 };

  GLuint vbo_triangles;
  glGenBuffers(1, &vbo_triangles);

  GLuint vbo_lines;
  glGenBuffers(1, &vbo_lines);

  GLuint vbo_line_strip;
  glGenBuffers(1, &vbo_line_strip);

  GLuint vbo_line_loop;
  glGenBuffers(1, &vbo_line_loop);

  GLuint curve;
  glGenBuffers(1, &curve);

  float t = 0.1;
  std::vector<std::vector<point>>BuildUp;
  point d1;
  d1.x = 0.0f;
  d1.y = 0.0f;
  d1.z = 0.0f;

  point d2;
  d2.x = 0.0f;
  d2.y = 1.0f;
  d2.z = 0.0f;
  point d3;
  d3.x = 1.0f;
  d3.y = 1.0f;
  d3.z = 0.0f;

  point d4;
  d4.x = 1.0f;
  d4.y = 0.0f;
  d4.z = 0.0f;
  std::vector<point>setup;
  setup.push_back(d1);
  setup.push_back(d2);
  setup.push_back(d3);
  setup.push_back(d4);

  BuildUp.push_back(setup);

  std::vector<point>final_points;

  int k = 2;
  std::vector<float>knots;
  knots.push_back(0.0);
  knots.push_back(1.0);
  knots.push_back(2.0);
  knots.push_back(3.0);
  knots.push_back(4.0);
  knots.push_back(5.0);

  std::vector<point>finalspline;


  float ubar = knots[1];
  int rest = 10;
  int I = 1;
  std::vector<std::vector<point>>buildup2;

  buildup2.push_back(setup);

  for(int n = 0; n < 4 - k + 1;n++){
    float increment = (knots[I+1] - knots[I]) / rest;

    for(int d = 0;d<rest;d++){

      buildup2.resize(1);

      for(int j = 1;j<=k-1;j++){
        buildup2.resize(j+1);

        std::vector<point>temp;
        for(int i = I - (k-1);i<=I-j;i++){
          buildup2[j].resize(i+1);
          point temp_point;

          temp_point.x = ((knots[i+k] - ubar) / (knots[i+k] - knots[i+j])) * buildup2[j-1][i].x
                         + ((ubar - knots[i+j]) / (knots[i+k] - knots[i+j])) * buildup2[j-1][i+1].x;


          temp_point.y = ((knots[i+k] - ubar) / (knots[i+k] - knots[i+j])) * buildup2[j-1][i].y
                         + ((ubar - knots[i+j]) / (knots[i+k] - knots[i+j])) * buildup2[j-1][i+1].y;


          temp_point.z = 0.0;

          if (i == I - (k-1) and j == k-1){
              finalspline.push_back(temp_point);
          }

          temp.push_back(temp_point);
         buildup2[j][i].x = temp_point.x;
         buildup2[j][i].y = temp_point.y;


        }
      }

      ubar += increment;


    }

    I = I + 1;
  }


    while(t < 1.0){
    BuildUp.resize(1);
    for(int j = 1;j<4;j++){
      std::vector<point>temp;
      for (int i = 0;i<4-j;i++){
        point temp_point;
        temp_point.x = (1-t)*(BuildUp[j-1][i].x) + (t)*(BuildUp[j-1][i+1].x);

        temp_point.y = (1-t)*(BuildUp[j-1][i].y) + (t)*(BuildUp[j-1][i+1].y);

        temp_point.z = 0.0;

        if(i == 4 - j -1 and j == 3)
          final_points.push_back(temp_point);

        temp.push_back(temp_point);
      }
      BuildUp.push_back(temp);
    }


    t+=.1;
  }

  do {


    BezierCurve();

    if(curves[pid].curvetype == 0){
      BezierCurve();
    }
    else{
      createKnots();
      deBoor();
    }

    glClear(GL_COLOR_BUFFER_BIT);

    std::vector<point>Points;
    point c1;
    c1.x = 0.0f;
    c1.y = 0.0f;
    c1.z = 0.0f;

    point c2;
    c2.x = 0.0f;
    c2.y = 1.0f;
    c2.z = 0.0f;
    point c3;
    c3.x = 1.0f;
    c3.y = 1.0f;
    c3.z = 0.0f;

    point c4;
    c4.x = 1.0f;
    c4.y = 0.0f;
    c4.z = 0.0f;


    Points.push_back(c1);
    Points.push_back(c2);
    Points.push_back(c3);
    Points.push_back(c4);


    static const GLfloat g_data_triangles[] = {
      -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };

    static const GLfloat g_data_poly[] = {
      0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0, 0.0f,
    };


    static const GLfloat g_data_lines[] = {
      0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    };



    static const GLfloat g_data_line_strip[] = { 0.0f, 0.0f, 0.0f, 0.0f,  1.0f, 0.0f,
                                                 1.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f };

    static const GLfloat g_data_line_loop[] = { -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f };

    glBindBuffer(GL_ARRAY_BUFFER, vbo_line_strip);
    glBufferData(GL_ARRAY_BUFFER,  curves[pid].ctrlpts.size() * 3 * sizeof(float) , &curves[pid].ctrlpts[0] , GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, curve);
    glBufferData(GL_ARRAY_BUFFER,  curves[pid].finalpts.size() * 3 * sizeof(float) , &curves[pid].finalpts[0] , GL_DYNAMIC_DRAW);

    // Clear the screen

    // Use our shader
    glUseProgram(program_id);

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &matrix[0][0]);

    // 1st attribute buffer : vertices


    glEnableVertexAttribArray(0);

     //glBindBuffer(GL_ARRAY_BUFFER, vbo_triangles);
     //glBindBuffer(GL_ARRAY_BUFFER, vbo_lines);
    glBindBuffer(GL_ARRAY_BUFFER, curve);
    //glBindBuffer(GL_ARRAY_BUFFER, vbo_line_strip);

    // glBindBuffer(GL_ARRAY_BUFFER, vbo_line_loop);

    glVertexAttribPointer(0, // attribute. No particular reason for 0, but must
                             // match the layout in the shader.
                          3, // size
                          GL_FLOAT, // type
                          GL_FALSE, // normalized?
                          0, // stride
                          (void*)0 // array buffer offset
    );

     //glDrawArrays(GL_TRIANGLES, 0, 3); // 3 vertices -> 1 triangle
     //glDrawArrays(GL_LINES, 0, 3); // 6 vertices -> 3 lines
    glDrawArrays(GL_LINE_STRIP, 0, curves[pid].finalpts.size()); // 4 vertices -> 3 lines
    // glDrawArrays(GL_LINE_LOOP, 0, 3); // 3 vertices -> 3 lines


    glBindBuffer(GL_ARRAY_BUFFER, vbo_line_strip);
    glVertexAttribPointer(0, // attribute. No particular reason for 0, but must
      // match the layout in the shader.
                          3, // size
                          GL_FLOAT, // type
                          GL_FALSE, // normalized?
                          0, // stride
                          (void*)0 // array buffer offset
    );

    glDrawArrays(GL_LINE_STRIP, 0, curves[pid].ctrlpts.size()); // 4 vertices -> 3 lines

    glDisableVertexAttribArray(0);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Render GUI
    ImGui::Begin("Options");
    ImGui::Text("Choose Curve to Manipulate");
    ImGui::SliderInt("Curve",&pid,0,curves.size()-1);
    ImGui::Text("Bezier or De Boor");


    ImGui::SliderInt("Algorithim",&curves[pid].curvetype,0,1);

  
    ImGui::Text("Choose Control point to change");
    static int ctrlindex = 0;
    ImGui::SliderInt("Point",&ctrlindex,0,curves[pid].ctrlpts.size()-1);
    ImGui::Text("Manipulate Control Point Values");
    static float val[2] = {curves[pid].ctrlpts[ctrlindex].x,curves[pid].ctrlpts[ctrlindex].y};
    ImGui::SliderFloat("X",&curves[pid].ctrlpts[ctrlindex].x,-1.0,1.0);
    ImGui::SliderFloat("Y",&curves[pid].ctrlpts[ctrlindex].y,-1.0,1.0);
    ImGui::Text("Adjust Resolution");
    ImGui::SliderInt("Resolution",&res,0,30);
    ImGui::Text("Delete control Point");
    static bool delte = false;
    static int delteindx = 0;
    ImGui::Checkbox("delete point",&delte);
    ImGui::SliderInt("Control Point",&delteindx,0,curves[pid].ctrlpts.size()-1);
    if (delte){
      curves[pid].ctrlpts.erase(curves[pid].ctrlpts.begin() + delteindx);
      delte = false;
    }

    static bool add = false;
    ImGui::Text("New Control Point");
    static float pt[2] = {0.0,0.0};
    ImGui::SliderFloat2("New Point",&pt[0],-1.0,1.0);
    static bool begin = false;
    static bool end = false;
    ImGui::Checkbox("Add in front",&begin);
    ImGui::Checkbox("Add in Back",&end);
    if(begin){
      point temp;
      temp.x = pt[0];
      temp.y = pt[1];
      temp.z = 0.0;

      curves[pid].ctrlpts.insert(curves[pid].ctrlpts.begin(),temp);

      begin = false;

    }

    if(end){
      point temp;
      temp.x = pt[0];
      temp.y = pt[1];
      temp.z = 0.0;

      curves[pid].ctrlpts.push_back(temp);
      end = false;
    }

    ImGui::Text("Or insert at Position");
    static int ind = 0;
    ImGui::SliderInt("Index",&ind,0,curves[pid].ctrlpts.size()-1);
    static bool insert = false;
    ImGui::Checkbox("Insertion",&insert);

    if(insert){
      point temp;
      temp.x = pt[0];
      temp.y = pt[1];
      temp.z = 0.0;

      curves[pid].ctrlpts.insert(curves[pid].ctrlpts.begin() + ind,temp);
      insert = false;

    }

    static bool file = false;
    ImGui::Text("Output Results");
    ImGui::Checkbox("File",&file);
    if(file){
      writefile();
      file = false;
    }


    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();

  }
  // Check if the ESC key was pressed or the window was closed
  while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);


  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();


  // Cleanup VBO and shader
  glDeleteBuffers(1, &vbo_triangles);
  glDeleteBuffers(1, &vbo_lines);
  glDeleteBuffers(1, &vbo_line_strip);
  glDeleteBuffers(1, &vbo_line_loop);
  glDeleteBuffers(1,&curve);
  glDeleteProgram(program_id);
  glDeleteVertexArrays(1, &vertex_array_id);

  // Close OpenGL window and terminate GLFW
  glfwTerminate();

  return 0;
}
