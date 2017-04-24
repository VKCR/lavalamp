/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Particles.h
 * Author: swl
 *
 * Created on April 15, 2016, 12:16 PM
 */

#ifndef PARTICLES_H
#define PARTICLES_H

#include <glm/glm.hpp>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <random>
#if defined(__APPLE_CC__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include <math.h>
#endif

using namespace std;

class Particles {
 public:
  Particles(int, int, int, float);
  void render() const;
  void step(); // simulate one frame
 private:
  struct Particle
  {
    glm::dvec3 pos;
    glm::dvec3 last_pos;
    glm::dvec3 force;
    double mass;
  };

  struct Plane
  {
    Plane(glm::dvec3 o, glm::dvec3 n){
      origin = o;
      normal = n;
    }
    glm::dvec3 origin;
    glm::dvec3 normal;
  };

  struct Cylinder
  {
    Cylinder(glm::dvec3 o, glm::dvec3 n, double r){
      origin = o;
      normal = n;
      radius = r;
    }
    glm::dvec3 origin;
    glm::dvec3 normal;
    double radius;
  };

  struct Link
  {
    Particle &p1, &p2;
  };

  //system constants
  double sphere_radius = 0.15;
  double sphere_volume = 4.0 / 3.0 * 3.1417 * pow(sphere_radius, 3);
  double ext_density = 1.0;
  double du_density = 0.000002;
  double dd_density = 0.000002;
  double delta_t = 0.1;
  double damping = 0.1;
  double mass = 1.0;
  double friction = 0.1;
  double SURFACE_OFFSET = 0.00001;
  double ks = 0.000003;
  double dist_thresh = 0.3;
  
  glm::dvec3 gravity = glm::dvec3(0.0,-50,0.0);
  
  //forces and collisions
  double density(Particle &p);
  glm::dvec3 buoyancy(Particle &p);
  void plane_collision(Plane &pl, Particle &p);
  void cylinder_collision(Cylinder &cy, Particle &p);
  void build_spatial_map();
  void self_collide(Particle &p, double simulation_steps);
  float hash_position(glm::dvec3 pos);
  //data structures
  std::vector<Particle> particles;
  std::vector<Link> links;
  unordered_map<float, vector<Particle *> *> map;
  //boundaries
  Plane lower_plane = Plane(glm::dvec3(0.0,-5.0,0.0), glm::dvec3(0.0,1.0,0.0));
  Plane upper_plane = Plane(glm::dvec3(0.0,5.0,0.0), glm::dvec3(0.0,-1.0,0.0));
  Cylinder cylinder = Cylinder(glm::dvec3(0.0,0.0,0.0), glm::dvec3(0.0,1.0,0.0), 1.5);
  
};

#endif /* PARTICLES_H */

