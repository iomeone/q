#include "scenario.h"
#include "resources.h"
#include "gl_helpers.h"
#include "bsp.h"


void TestScenario::startLoading() {
  ResourceManager::getInstance()->loadResource({
    "./data/poptart.jpg",
    ResourceType::IMAGE_FILE,
    TEXTURE_ID
  });

  ResourceManager::getInstance()->loadShaders({
    "./shader/test.vert",
    "./shader/test.frag",
    SHADER_ID
  });
}

bool TestScenario::finishLoading() {
  if (!ResourceManager::getInstance()->finishedLoading()) {
    cerr << "tried to load scenario before resources finished loading\n";
    return false;
  }

  ////////////////////////////////////////////////////////////////////////////
  // Working through open.gl again...

  // Save the following attribute configuration as a vao (vertex array object)
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  ////////////////////////////////////////////////////////////////////////////
  // Generate EBO
  GLuint ebo;
  glGenBuffers(1, &ebo);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

  ////////////////////////////////////////////////////////////////////////////
  // Generate VBO
  GLuint vbo;
  glGenBuffers(1, &vbo);

  // Choose the vbo...
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  // Copy the vertex data into the vbo
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  ////////////////////////////////////////////////////////////////////////////
  // Get the test shader program
  optional<GLuint> shaderProgram = ResourceManager::getInstance()->getShaderProgram(SHADER_ID);
  if (!shaderProgram) {
    cerr << "failed to load shader program\n";
    return false;
  }

  // Use the program...
  glLinkProgram(*shaderProgram);
  glUseProgram(*shaderProgram);

  ////////////////////////////////////////////////////////////////////////////
  // Specify the inputs

  // For the vertices
  GLint posAttrib = glGetAttribLocation(*shaderProgram, "position");
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
  glEnableVertexAttribArray(posAttrib);

  // And colors
  GLint texAttrib = glGetAttribLocation(*shaderProgram, "texCoord");
  glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) (2 * sizeof(float)));
  glEnableVertexAttribArray(texAttrib);

  if (hasErrors()) {
    return false;
  }

  return true;
}

void TestScenario::think() {
  // TODO input?
}

void TestScenario::render() {
  glClearColor(0.0, 1.0, 1.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  // glDrawArrays(GL_TRIANGLES, 0, 3);
  glDrawElements(GL_TRIANGLES, sizeof(elements) / sizeof(elements[0]), GL_UNSIGNED_INT, 0);

  hasErrors();
}

void BSPScenario::startLoading() {
  ResourceManager::getInstance()->loadResource({
    "./data/aerowalk.bsp",
    ResourceType::BSP_FILE,
    BSP_ID
  });

  ResourceManager::getInstance()->loadShaders({
    "./shader/bsp.vert",
    "./shader/bsp.frag",
    SHADER_ID
  });
}

bool BSPScenario::finishLoading() {
  if (!ResourceManager::getInstance()->finishedLoading()) {
    cerr << "tried to load scenario before resources finished loading\n";
    return false;
  }

  const BSPMap* map = ResourceManager::getInstance()->getMap();
  map->print();
  map->printVertices();
  map->printFaces();

  // const BSP::face_t* faces = map->faces();
  // const int numFaces = map->numFaces();
  // const BSP::vertex_t* vertices = map->vertices();
  // const int numVertices = map->numVertices();

  // for (int faceIndex = 0; faceIndex < numFaces; faceIndex ++) {
  //   const BSP::face_t* face = faces + faceIndex;
  //   const int firstVertex = face->vertex;
  //   const int numVertices = face->n_vertices;

  //   for (int j = 0; j < numVertices; j ++) {
  //     const int vertexIndex = firstVertex + j;
  //     const BSP::vertex_t* vertex = vertices + vertexIndex;
      
  //   }
  // }

  // Load the test vertices
  glGenBuffers(1, &(_faceVBOs[TEST_FACE_ID].vertexBuffer));
  glBindBuffer(GL_ARRAY_BUFFER, _faceVBOs[TEST_FACE_ID].vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(testVertices), testVertices, GL_STATIC_DRAW);

  // Load the test elements
  glGenBuffers(1, &(_faceVBOs[TEST_FACE_ID].elementsBuffer));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _faceVBOs[TEST_FACE_ID].elementsBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(testMeshverts), testMeshverts, GL_STATIC_DRAW);

  // Load some random colors for the vertices
  _faceVBOs[TEST_FACE_ID].colorsBuffer = GLHelpers::generateRandomColorsVBO(sizeof(testVertices) / sizeof(testVertices[0]));

  // Load the shader
  GLuint shaderProgram = *ResourceManager::getInstance()->getShaderProgram(SHADER_ID);

  // Use the program...
  glLinkProgram(shaderProgram);
  glUseProgram(shaderProgram);

  // Bind the inputs
  _inPosition = glGetAttribLocation(shaderProgram, "inPosition");
  glVertexAttribPointer(_inPosition, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float) /* stride */, 0 /* offset */);
  glEnableVertexAttribArray(_inPosition);

  _inColor = glGetAttribLocation(shaderProgram, "inColor");
  glVertexAttribPointer(_inColor, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float) /* stride */, 0 /* offset */);
  glEnableVertexAttribArray(_inColor);

  _unifCameraTransform = glGetUniformLocation(shaderProgram, "unifCameraTransform");
  _unifProjTransform = glGetUniformLocation(shaderProgram, "unifProjTransform");

  if (hasErrors()) {
    return false;
  }

  return true;
}

void BSPScenario::think() {
}

void BSPScenario::render() {
  glClearColor(0.6, 0.2, 0.6, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);

  // Update camera transform
  glm::mat4 cameraTransform = glm::lookAt(
    glm::vec3(5.0f, -20.0f, -5.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, -1.0f)
    // glm::vec3(20.0f, 20.0f, 20.0f),
    // glm::vec3(0.0f, 0.0f, 0.0f),
    // glm::vec3(0.0f, 0.0f, 1.0f)
  );
  glUniformMatrix4fv(_unifCameraTransform, 1, GL_FALSE, glm::value_ptr(cameraTransform));

  // And projection transform
  glm::mat4 projectionTransform = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.5f, 10000.0f);
  glUniformMatrix4fv(_unifProjTransform, 1, GL_FALSE, glm::value_ptr(projectionTransform));

  // Bind vertices
  glBindBuffer(GL_ARRAY_BUFFER, _faceVBOs[TEST_FACE_ID].vertexBuffer);
  glVertexAttribPointer(_inPosition, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float) /* stride */, 0 /* offset */);

  // Bind colors
  glBindBuffer(GL_ARRAY_BUFFER, _faceVBOs[TEST_FACE_ID].colorsBuffer);
  glVertexAttribPointer(_inColor, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float) /* stride */, 0 /* offset */);

  // Draw elements
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _faceVBOs[TEST_FACE_ID].elementsBuffer);
  int elementsSize;
  glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &elementsSize);
  glDrawElements(GL_TRIANGLES, elementsSize / sizeof(GLuint), GL_UNSIGNED_INT, 0);

  hasErrors();
}
