/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Particles.cpp
 * Author: swl
 * 
 * Created on April 15, 2016, 12:16 PM
 */

#include "Particles.h"

Particles::Particles(int nx, int ny, int nz, float d) 
{
  for(int x=0; x<nx; x++)
    {
      for(int y=0; y<ny; y++)
        {
	  for(int z=0; z<nz; z++)
            {
	      Particle p;
	      //p.pos = glm::dvec3((x+0.5-nx*0.5)*d, (y+0.5)*d-1.0, (z+0.5-nz*0.5)*d);
	      p.pos = glm::dvec3((double)rand() / RAND_MAX - nx*0.5*d, (double)rand() / RAND_MAX * 1.5  - ny*0.5*d + lower_plane.origin.y * 0.5 , (double)rand() / RAND_MAX  - nz*0.5*d );
	      p.last_pos = p.pos;
	      p.mass = sphere_volume * ext_density;//density(p);
	      particles.push_back(p);
            }
        }
    }
}

void Particles::render() const
{
  GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat mat_shininess[] = { 50.0 };
  GLfloat light_position[] = { 10.0, 10.0, 10.0, 0.0 };
  glShadeModel (GL_SMOOTH);
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT, GL_DIFFUSE);
  glColor3f(0.2, 0.5, 0.8);
  glColorMaterial(GL_FRONT, GL_SPECULAR);
  glColor3f(0.9, 0.9, 0.9);
  glColorMaterial(GL_FRONT, GL_AMBIENT);
  glColor3f(0.2, 0.5, 0.8);

  /*draw the two disk extremities*/
  glPushMatrix();
  glTranslatef(lower_plane.origin.x, lower_plane.origin.y, lower_plane.origin.z);
  glRotatef(90.0, 1.0, 0.0, 0.0);
  GLUquadricObj* quad_bot = gluNewQuadric();
  gluDisk(quad_bot, 0.0, 1.5  + sphere_radius, 64, 1);
  gluDeleteQuadric(quad_bot);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(upper_plane.origin.x, upper_plane.origin.y, upper_plane.origin.z);
  glRotatef(90.0, 1.0, 0.0, 0.0);
  GLUquadricObj* quad_top = gluNewQuadric();
  gluDisk(quad_top, 0.0, 1.5 + sphere_radius,64, 1);
  gluDeleteQuadric(quad_top);
  glPopMatrix();
  
  
  
  for(const Particle &par : particles)
    {    
        
      glPushMatrix();
      glTranslatef(par.pos.x, par.pos.y, par.pos.z);
      glutSolidSphere(sphere_radius, 10, 10);
      glPopMatrix();
      }

    
  glPopAttrib();
}

void Particles::step()
{
  //TODO
  //-compute inter-particle forces: make it better
  //-inter-particle collisions
  //-fix wall collisions so that the particles keep their momentum. V i think
  //-buoyancy: more realistic model
  //-blobs

  //calculate force
  //external forces
  for (auto &p : particles){
    p.force = buoyancy(p);
    //cout<<p.force.y<<endl;
  }


  //inter particle force
  for (Particle &p1 : particles){ //O(n**2), to change
    for (Particle &p2 : particles){
      if (&p1 == &p2)
	continue;
      double d = glm::length(p1.pos - p2.pos);
      //if (d <= dist_thresh){
      //p1.force += ks * glm::normalize(p2.pos - p1.pos);// * pow(1.0 / d, 2);
      if (d < 0.03)
	p1.force += ks * glm::normalize(p2.pos - p1.pos) * pow(1.0 / 0.03, 2);
      else
	p1.force += ks * glm::normalize(p2.pos - p1.pos) * pow(1.0 / d, 2);
	//}
    }
  }

  //verlet integration

  for(Particle &p : particles){    
    glm::dvec3 curpos = p.pos;
    glm::dvec3 lastpos = p.last_pos;
      
    p.pos = curpos + (1 - damping / 100.0) * (curpos - lastpos) + p.force * (1.0 / mass) * pow(delta_t,2);
    p.last_pos = curpos;
  }


  //self collisions
  build_spatial_map();
  
  for (Particle &p : particles) {
    self_collide(p, 300);
    }
  
  //collisions with cylinder and bottom/top plane
  for (Particle &p : particles){
    cylinder_collision(cylinder, p);
    plane_collision(lower_plane, p);
    plane_collision(upper_plane, p);
    
  }

  

  

  

  //update the mass of the particles

  for (Particle &p : particles){
    p.mass += sphere_volume * density(p);
  }
  
}


void Particles::build_spatial_map() {
  map.clear();
  float hash_val;
  int count = 0;
  for (auto &p : particles){
    hash_val = hash_position(p.pos);
    vector<Particle *> *points = map[hash_val];
    if (points == nullptr){ //new key
      points = new vector<Particle *>();
      points->push_back(&p);
      map[hash_val] = points;
    }
    else{
      points->push_back(&p);
    }
  }


}

void Particles::self_collide(Particle &p, double simulation_steps) {
  //cout<<"what"<<endl;
  float hash_val = hash_position(p.pos);
  
  vector<Particle *> *points = map[hash_val];

  if (points == nullptr){
    //cout<<"why though?"<<endl;
    return;
  }
  
  glm::dvec3 corrs;
  int ncorrs = 0;
  for (auto ps : *points){
    if (&p == ps){
      continue;
    }
    glm::dvec3 delta_p = ps->pos - p.pos;
    if (glm::length(delta_p) < 2.0 * sphere_radius){
      ncorrs++;
      corrs += -glm::normalize(delta_p) * (2.0 * sphere_radius - glm::length(delta_p));
    }
    
  }
  if (ncorrs > 0){
    
    corrs = corrs * (1.0 / ncorrs / simulation_steps);
    p.pos += corrs;
    //p.last_pos = p.pos - corrs * 2.0;
  }
  //cout<<"what"<<endl;
  //cout<<"whats"<<endl;  

}

float Particles::hash_position(glm::dvec3 pos) {
 
  int width = 5.0;
  int height = 5.0;
  int num_width_points = 100;
  int num_height_points = 100;
  
  double w = 3.0 * width / num_width_points;
  double h = 3.0 * height / num_height_points;
  double t = max(w,t);

  //cout<<w<<endl<<h<<endl<<t<<endl;
  //cout<<pos.x<<endl<<pos.y<<endl<<pos.z<<endl;

  //Vector3D bbox_index = Vector3D(fmod(pos.x,w) + 1, fmod(pos.y, h) + 1, fmod(pos.z, t) + 1);
  glm::dvec3 bbox_index = glm::dvec3(floor(pos.x/w) , floor(pos.y/h) , floor(pos.z/t) );

  //cout<<bbox_index<<endl<<endl;
  //return 0;
  
  return abs(bbox_index.x)*1.0  + abs(bbox_index.y)*0.00001 + abs(bbox_index.z)*0.00000001;
}


//collision functions
void Particles::plane_collision(Plane &pl, Particle &p){
  if (glm::dot((p.pos - pl.origin),pl.normal) <= 0){
    glm::dvec3 tang_p = p.pos - pl.normal * glm::dot((p.pos - pl.origin), pl.normal);
    glm::dvec3 corr = tang_p - p.last_pos + pl.normal * SURFACE_OFFSET;
    p.pos = p.last_pos + corr * (1 - friction);
    //p.last_pos = p.pos;
  }
}

void Particles::cylinder_collision(Cylinder &cy, Particle &p){
  glm::dvec3 dpos = p.pos - cy.origin;
  glm::dvec3 delta = dpos - glm::dot(dpos,cy.normal) * cy.normal;
  //cout<<delta.x<<" "<<delta.y<<" "<<delta.z<<" "<<endl;
  
  if (glm::length(delta) > cy.radius){
    glm::dvec3 tang_p =  glm::normalize(delta) * cy.radius;
    glm::dvec3 corr = tang_p - delta;
    p.pos = p.last_pos + corr * (1 - friction);
    //p.last_pos = p.pos;
  }
}


//force functions
double Particles::density(Particle &p){
  if (p.pos.y - lower_plane.origin.y <= 1.0 / 8.0 * (upper_plane.origin.y - lower_plane.origin.y)){
    return - du_density;
  }else{
    return  dd_density;
  }
  //return 1.0 + p.pos.y / (upper_plane.origin.y - lower_plane.origin.y); //2.0 = distance of the planes from origin in y direction
  //thus varies between 0.5 at lower end of the plane and 1.5 at top end of the plane
}

glm::dvec3 Particles::buoyancy(Particle &p){
  return gravity * (p.mass - sphere_volume * ext_density);
}


