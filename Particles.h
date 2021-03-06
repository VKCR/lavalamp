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
#include <set>
#include <queue>
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
  void render();
  void step(); // simulate one frame
 private:
  struct Particle
  {
    glm::dvec3 pos;
    glm::dvec3 last_pos;
    glm::dvec3 force;

    set<Particle*> linked;
    
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

  //system constants
  double sphere_radius = 0.04;
  double sphere_volume = 4.0 / 3.0 * 3.1417 * pow(sphere_radius, 3);
  double ext_density = 1.0;
  double du_density = 0.00005;
  double dd_density = 0.0000055;
  double delta_t = 0.4; //0.3 for rendering
  double damping = 2.0; //0.2
  double mass = 1.0;
  double friction = 0.01;
  double SURFACE_OFFSET = 0.00001;
  double ka = 0.0000001;
  double kr = 0.00000005;
  double force_thresh = 0.08;
  double blob_thresh = 0.3;
  
  glm::dvec3 gravity = glm::dvec3(0.0,-50,0.0);

  int rendering_step = 0;
  
  //forces and collisions
  double density(Particle &p);
  glm::dvec3 buoyancy(Particle &p);
  
  //data structures
  vector<Particle> particles;
  //vector<Blob> blobs;
  set<vector<Particle*>> blobs;
  unordered_map<float, vector<Particle *> *> map;
  
  //blob functions
  void update_blobs();
  
  //boundaries and collisions
  Plane lower_plane = Plane(glm::dvec3(0.0,-1.0,0.0), glm::dvec3(0.0,1.0,0.0));
  Plane upper_plane = Plane(glm::dvec3(0.0,1.0,0.0), glm::dvec3(0.0,-1.0,0.0));
  Cylinder cylinder = Cylinder(glm::dvec3(0.0,0.0,0.0), glm::dvec3(0.0,1.0,0.0), .4);
  void plane_collision(Plane &pl, Particle &p);
  void cylinder_collision(Cylinder &cy, Particle &p);
  void build_spatial_map();
  void self_collide(Particle &p, double simulation_steps);
  float hash_position(glm::dvec3 pos);
  
};

#endif /* PARTICLES_H */

