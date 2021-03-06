#ifndef BSP_H
#define BSP_H

#include "support.h"

namespace BSP {
  enum class FaceType : int {
    POLYGON = 1,
    PATCH = 2,
    MESH = 3,
    BILLBOARD = 4
  };

  struct direntry_t {
    int offset;
    int length;
  };

  struct texture_t {
    char name[64];
    int flags; // Surface flags?
    int contents; // Content flags?
  };

  struct vertex_t {
    float position[3];
    float texcoord[2]; // Vertex texture coordinates.
    float lmcoord[2]; // Lightmap texture coordinates.
    float normal[3]; //	Vertex normal.
    int8_t color[4]; //	Vertex color. RGBA.

    vertex_t operator+(const vertex_t& rhs) const;
    vertex_t operator*(double rhs) const;
  };

  struct face_t {
    int texture; //	Texture index.
    int effect; //	Index into lump 12 (Effects), or -1.
    int type; //	Face type. 1=polygon, 2=patch, 3=mesh, 4=billboard
    int vertex; //	Index of first vertex.
    int n_vertices; //	Number of vertices.
    int meshvert; //	Index of first meshvert.
    int n_meshverts; //	Number of meshverts.
    int lm_index; //	Lightmap index.
    int lm_start[2]; //	Corner of this face's lightmap image in lightmap.
    int lm_size[2]; //	Size of this face's lightmap image in lightmap.
    float lm_origin[3]; //	World space origin of lightmap.
    float lm_vecs[2][3]; //	World space lightmap s and t unit vectors.
    float normal[3]; //	Surface normal.
    int size[2]; //	Patch dimensions.
  };

  struct effect_t {
    char name[64];
    int brush;
    int unknown; // always 5?
  };

  struct meshvert_t {
    int offset; // Vertex index offset, relative to first vertex of corresponding face.
  };

  struct lightmap_t {
    unsigned char map[128 * 128 * 3];
  };

  struct header_t {
    char magic[4];
    int version;
    direntry_t direntries[17];

    // Surface descriptions (assume these have been converted to OpenGL textures).
    const direntry_t* texturesEntry() const { return direntries + 1; }
    int numTextures() const {
      return texturesEntry()->length / sizeof(texture_t);
    }
    const texture_t* textures() const {
      return (const texture_t*) ((char*) this + texturesEntry()->offset);
    }

    // Planes	Planes used by map geometry.
    const direntry_t* planesEntry() const { return direntries + 2; }

    // Nodes	BSP tree nodes.
    const direntry_t* nodesEntry() const { return direntries + 3; }

    // Leaves	BSP tree leaves.
    const direntry_t* leavesEntry() const { return direntries + 4; }

    // Leaffaces	Lists of face indices, one list per leaf.
    const direntry_t* leaffacesEntry() const { return direntries + 5; }

    // Models	Descriptions of rigid world geometry in map (we only use model[0]).
    const direntry_t* modelsEntry() const { return direntries + 7; }

    // Vertices	Vertices used to describe faces.
    const direntry_t* verticesEntry() const { return direntries + 10; }
    int numVertices() const {
      return verticesEntry()->length / sizeof(vertex_t);
    }
    const vertex_t* vertices() const {
      return (const vertex_t*) ((char*) this + verticesEntry()->offset);
    }

    // Meshverts	Lists of offsets, one list per mesh.
    const direntry_t* meshvertsEntry() const { return direntries + 11; }
    int numMeshverts() const {
      return meshvertsEntry()->length / sizeof(meshvert_t);
    }
    const meshvert_t* meshverts() const {
      return (const meshvert_t*) ((char*) this + meshvertsEntry()->offset);
    }

    // Shaders per face
    const direntry_t* effectsEntry() const { return direntries + 12; }
    int numEffects() const {
      return effectsEntry()->length / sizeof(effect_t);
    }
    const effect_t* effects() const {
      return (const effect_t*) ((char*) this + effectsEntry()->offset);
    }

    // Faces	Surface geometry.
    const direntry_t* facesEntry() const { return direntries + 13; }
    int numFaces() const {
      return facesEntry()->length / sizeof(face_t);
    }
    const face_t* faces() const {
      return (const face_t*) ((char*) this + facesEntry()->offset);
    }

    // Lightmaps	Packed lightmap data (assume these have been converted to an OpenGL texture)
    const direntry_t* lightmapsEntry() const { return direntries + 14; }
    int numLightmaps() const {
      return lightmapsEntry()->length / sizeof(lightmap_t);
    }
    const lightmap_t* lightmaps() const {
      return (const lightmap_t*) ((char*) this + lightmapsEntry()->offset);
    }

    const direntry_t* lightvolsEntry() const { return direntries + 15; }

    // Visdata	Cluster-cluster visibility data.
    const direntry_t* visdataEntry() const { return direntries + 16; }

    void print() const;
    void printEffects() const;
    void printTextures() const;
    void printVertices() const;
    void printFaces() const;
    void printMeshverts() const;
  };
};

std::ostream& operator<<(std::ostream& os, const BSP::texture_t& texture);
std::ostream& operator<<(std::ostream& os, const BSP::face_t& vertex);
std::ostream& operator<<(std::ostream& os, const BSP::vertex_t& vertex);
std::ostream& operator<<(std::ostream& os, const BSP::meshvert_t& meshvert);
std::ostream& operator<<(std::ostream& os, const BSP::effect_t& effect);

using BSPMap = BSP::header_t;

#endif