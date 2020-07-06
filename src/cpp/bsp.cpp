#include "bsp.h"
#include "pprint.hpp"

using namespace BSP;

void header_t::print() const {
  printf("map {\n");
  printf(" magic: %s\n", this->magic);
  printf(" version: %d\n", this->version);
  for (const direntry_t& entry : this->direntries) {
    printf(" entry offset: %d, size: %d\n", entry.offset, entry.length);
  }
  printf("}\n");
}

void header_t::printVertices() const {
  const direntry_t* verticesEntry = this->vertices();

  const auto printVertex = [](const vertex_t* v) {
    cout << "  position: " << v->position << "\n";
    cout << "  texcoord: " << v->texcoord << "\n";
    cout << "  normal: " << v->normal << "\n";
    cout << "  color: " << v->color << "\n";
  };

  cout << "vertices {\n";

  const int offset = verticesEntry->offset;
  const int length = verticesEntry->length;
  cout << "  offset: " << offset << " length: " << length << "\n";

  cout << "  num: " << numVertices() << "\n";

  {
    const vertex_t* vertex = getVertex(0);
    printVertex(vertex);
  }

  {
    const vertex_t* vertex = getVertex(numVertices() - 1);
    printVertex(vertex);
  }

  cout << "}\n";
}


void header_t::printFaces() const {
  const direntry_t* facesEntry = this->faces();

  const auto printFace = [](const face_t* f) {
    cout << "  texture: " << f->texture << "\n";
    cout << "  effect: " << f->effect << "\n";
    cout << "  type: " << f->type << "\n";
    cout << "  vertex: " << f->vertex << "\n";
    cout << "  n_vertices: " << f->n_vertices << "\n";
    cout << "  meshvert: " << f->meshvert << "\n";
    cout << "  n_meshverts: " << f->n_meshverts << "\n";
    cout << "  lm_index: " << f->lm_index << "\n";
    cout << "  lm_start: " << f->lm_start << "\n";
    cout << "  lm_size: " << f->lm_size << "\n";
    cout << "  lm_origin: " << f->lm_origin << "\n";
    cout << "  lm_vecs: " << f->lm_vecs << "\n";
    cout << "  normal: " << f->normal << "\n";
    cout << "  size: " << f->size << "\n";
  };

  cout << "faces {\n";

  const int offset = facesEntry->offset;
  const int length = facesEntry->length;
  cout << "  offset: " << offset << " length: " << length << "\n";

  cout << "  num: " << numFaces() << "\n";

  {
    const face_t* face = getFace(0);
    printFace(face);
  }

  {
    const face_t* face = getFace(numFaces() - 1);
    printFace(face);
  }

  cout << "}\n";
}
