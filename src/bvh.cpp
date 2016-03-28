#include "bvh.h"

#include "CGL/CGL.h"
#include "static_scene/triangle.h"

#include <iostream>
#include <stack>

using namespace std;

namespace CGL { namespace StaticScene {

BVHAccel::BVHAccel(const std::vector<Primitive *> &_primitives,
                   size_t max_leaf_size) {

  root = construct_bvh(_primitives, max_leaf_size);

}

BVHAccel::~BVHAccel() {
  if (root) delete root;
}

BBox BVHAccel::get_bbox() const {
  return root->bb;
}

void BVHAccel::draw(BVHNode *node, const Color& c) const {
  if (node->isLeaf()) {
    for (Primitive *p : *(node->prims))
      p->draw(c);
  } else {
    draw(node->l, c);
    draw(node->r, c);
  }
}

void BVHAccel::drawOutline(BVHNode *node, const Color& c) const {
  if (node->isLeaf()) {
    for (Primitive *p : *(node->prims))
      p->drawOutline(c);
  } else {
    drawOutline(node->l, c);
    drawOutline(node->r, c);
  }
}

BVHNode *BVHAccel::construct_bvh(const std::vector<Primitive*>& prims, size_t max_leaf_size) {
  
  // TODO Part 2, task 1:
  // Construct a BVH from the given vector of primitives and maximum leaf
  // size configuration. The starter code build a BVH aggregate with a
  // single leaf node (which is also the root) that encloses all the
  // primitives.


  BBox centroid_box, bbox;

  for (Primitive *p : prims) {
    BBox bb = p->get_bbox();
    bbox.expand(bb);
    Vector3D c = bb.centroid();
    centroid_box.expand(c);
  }

  BVHNode *node = new BVHNode(bbox);
  node->prims = new vector<Primitive *>(prims);
  if (prims.size() > max_leaf_size) {
    double xVal = bbox.extent[0];
    double yVal = bbox.extent[1];
    double zVal = bbox.extent[2];
    double axisVal = max(xVal, max(yVal, zVal));
    int axis;
    if (axisVal == xVal) {
      axis = 0;
    } else if (axisVal == yVal) {
      axis = 1;
    } else {
      axis = 2;
    }
    vector<Primitive *> leftPrims = vector<Primitive *>();
    vector<Primitive *> rightPrims = vector<Primitive *>();
    double split = (centroid_box.max[axis] + centroid_box.min[axis])/2;
    for (Primitive *p : prims) {
      Vector3D c = p->get_bbox().centroid();
      if (c[axis] < split) {
        leftPrims.push_back(p);
      } else {
        rightPrims.push_back(p);
      }
    }
    if (leftPrims.size() != 0) {
      node->l = construct_bvh(leftPrims, max_leaf_size);
    }
    if (rightPrims.size() != 0) {
      node->r = construct_bvh(rightPrims, max_leaf_size);
    }
  } else {
    node->l = NULL;
    node->r = NULL;
  }
  return node;
}


bool BVHAccel::intersect(const Ray& ray, BVHNode *node) const {

  // TODO Part 2, task 3:
  // Implement BVH intersection.
  // Currently, we just naively loop over every primitive.
  double start = ray.min_t;
  double end = ray.max_t;
  if (!node->bb.intersect(ray, start, end)) {
    return false;
  }
  if (node->isLeaf()) {
    for (Primitive *p : *(node->prims)) {
      total_isects++;
      if (p->intersect(ray)) {
        return true;
      }
    }
    return false;
  }
  bool leftVal = intersect(ray, node->l);
  bool rightVal = intersect(ray, node->r);
  return leftVal || rightVal;
}

bool BVHAccel::intersect(const Ray& ray, Intersection* i, BVHNode *node) const {

  // TODO Part 2, task 3:
  // Implement BVH intersection.
  // Currently, we just naively loop over every primitive.
  double start = ray.min_t;
  double end = ray.max_t;
  if (!node->bb.intersect(ray, start, end)) {
    return false;
  }
  if (node->isLeaf()) {
    bool r = false;
    for (Primitive *p : *(node->prims)) {
      total_isects++;
      if (p->intersect(ray, i)) {
        r = true;
      }
    }
    return r;
  }
  bool leftVal = intersect(ray, i, node->l);
  bool rightVal = intersect(ray, i, node->r);
  return leftVal || rightVal;
}

}  // namespace StaticScene
}  // namespace CGL
