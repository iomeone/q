#include "scenario.h"
#include "resources.h"
#include "renderable.h"
#include "gl_helpers.h"
#include "bsp.h"
#include "pprint.hpp"


void TestScenario::startLoading() {
  ResourceManager::getInstance()->loadResource({
    "./data/textures/poptart.jpg",
    ResourceType::IMAGE_FILE,
    TEXTURE_ID
  });

  ResourceManager::getInstance()->loadShaders({
    "./src/glsl/test.vert",
    "./src/glsl/test.frag",
    SHADER_ID
  });
}

bool TestScenario::loadDependencies() {
  return false;
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
    "./src/glsl/render_scene.vert",
    "./src/glsl/render_scene.frag",
    SCENE_SHADER_ID
  });
}

bool BSPScenario::loadDependencies() {
  if (!_renderableMap) {
    ResourcePtr<const BSPMap> mapResource = ResourceManager::getInstance()->getMap();
    if (!mapResource.get()) {
      cerr << "map failed to load\n";
      return false;
    }
    _renderableMap = make_shared<RenderableBSP>(mapResource);
  }

  if (_renderableMap) {
    return _renderableMap->loadDependencies();
  }

  cerr << "failed to load dependencies\n";
  return false;
}

bool BSPScenario::finishLoading() {
  if (!ResourceManager::getInstance()->finishedLoading()) {
    cerr << "tried to load scenario before resources finished loading\n";
    return false;
  }

  if (loadDependencies()) {
    cerr << "tried to finish loading when continue loading is still dirty\n";
    return false;
  }

  { // Finish loading the map. This generates textures, VBOs, & EBOs
    bool success = _renderableMap->finishLoading();
    if (!success) {
      return false;
    }
  }

  // Get the screen size, this will be used for FBOs later
  GLint viewportSize[4];
  glGetIntegerv(GL_VIEWPORT, viewportSize);
  int screenWidth = viewportSize[2];
  int screenHeight = viewportSize[3];

  // Load the shader
  _sceneShader = *ResourceManager::getInstance()->getShaderProgram(SCENE_SHADER_ID);

  // Use the program...
  glLinkProgram(_sceneShader);
  glUseProgram(_sceneShader);

  // Bind the inputs
  _inPosition = glGetAttribLocation(_sceneShader, "inPosition");
  glEnableVertexAttribArray(_inPosition);

  _inTextureCoords = glGetAttribLocation(_sceneShader, "inTextureCoords");
  glEnableVertexAttribArray(_inTextureCoords);

  _inLightmapCoords = glGetAttribLocation(_sceneShader, "inLightmapCoords");
  glEnableVertexAttribArray(_inLightmapCoords);

  _inColor = glGetAttribLocation(_sceneShader, "inColor");
  glEnableVertexAttribArray(_inColor);

  _unifTexture = glGetUniformLocation(_sceneShader, "unifTexture");
  _unifLightmapTexture = glGetUniformLocation(_sceneShader, "unifLightmapTexture");
  _unifCameraTransform = glGetUniformLocation(_sceneShader, "unifCameraTransform");
  _unifProjTransform = glGetUniformLocation(_sceneShader, "unifProjTransform");

  if (hasErrors()) {
    return false;
  }

  return true;
}

glm::vec3 Camera::forward() {
  return glm::vec3(
    sin(pitch) * cos(-yaw),
    sin(pitch) * sin(-yaw),
    -cos(pitch)
  );
}

glm::vec3 Camera::right() {
  return glm::cross(forward(), upApproximate());
}

glm::vec3 Camera::upApproximate() {
  return glm::vec3(0,0,-1);
}

void BSPScenario::think(glm::vec2 dir, double pitch, double yaw) {
  _camera.pitch = pitch;
  _camera.yaw = yaw;
  _camera.location += _camera.forward() * (dir.y * 10.0f);
  _camera.location += _camera.right() * (dir.x * 10.0f);
}

void BSPScenario::render() {
  ResourcePtr<const BSPMap> mapResource = ResourceManager::getInstance()->getMap();
  const BSPMap* map = mapResource.get();

  //////////////////////////////////////////////////////////////////////////////

  optional<GLuint> sceneShaderID = ResourceManager::getInstance()->getShaderProgram(SCENE_SHADER_ID);
  if (!sceneShaderID) {
    cerr << "failed to load shader program\n";
    return;
  }

  if (!_renderableMap) {
    cerr << "failed to load map\n";
    return;
  }

  // Use the program...
  glUseProgram(_sceneShader);

  glClearColor(0.6, 0.2, 0.6, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);

  // Update camera transform
  glm::mat4 cameraTransform = glm::lookAt(
    _camera.location, // location of camera
    _camera.location + _camera.forward(), // look at
    glm::vec3(0,0,1)  // camera up vector
  );

  glUniformMatrix4fv(_unifCameraTransform, 1, GL_FALSE, glm::value_ptr(cameraTransform));

  // And projection transform
  glm::mat4 projectionTransform = glm::perspective(glm::radians(90.0f), 1200.0f / 800.0f, 0.5f, 10000.0f);
  glUniformMatrix4fv(_unifProjTransform, 1, GL_FALSE, glm::value_ptr(projectionTransform));

  // Render all the solid geometry in the map
  {
    ShaderParameters shaderInputs {
      _inPosition,
      _inColor,
      _inTextureCoords,
      _inLightmapCoords,
      _unifTexture,
      _unifLightmapTexture,
      RenderMode::SOLID
    };
    _renderableMap->render(shaderInputs);
  }

  // Render all the transparent geometry on top of the existing geometry
  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  {
    ShaderParameters shaderInputs {
      _inPosition,
      _inColor,
      _inTextureCoords,
      _inLightmapCoords,
      _unifTexture,
      _unifLightmapTexture,
      RenderMode::TRANSPARENCY
    };
    _renderableMap->render(shaderInputs);
  }
}
