// stb_particle_system.h - v0.0 - OpenGL 4.6+ Simple Particle System - public domain
//
// To build this, in one source file that includes this file do
//      #define STB_INCLUDE_IMPLEMENTATION
//
// DESCRIPTION [to do]
//
// Options:
//
//      [to do]
//
// Standard libraries:
//
//      vector
//      cstdlib     rand, srand
//      ctime       time
//
// External libraries:
// 
//      glm 0.9.6.4
//      openGL (glad or glew)
// 
// Credits:
//
// Written by Gabriel Belisario

#define STB_PARTICLE_SYSTEM_DEV

#ifdef STB_PARTICLE_SYSTEM_DEV
// #define STB_PARTICLE_SYSTEM_IMPLEMENTATION
#include "headers.h"
#endif

#ifndef STB_PARTICLE_SYSTEM_H
#define STB_PARTICLE_SYSTEM_H

#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

// All atributes that can be asigned to the particle system from software
struct ParticleProps{
    glm::vec3 position = glm::vec3(0.f); // where the particles will be generated
    glm::vec3 velocity = glm::vec3(0.f), velocity_variation = glm::vec3(0.f), acceleration = glm::vec3(0.f);
    glm::vec4 color_begin = glm::vec4(1.f), color_end = glm::vec4(1.f); // color of the particle
    float size_begin = 1.f, size_end = 1.f, size_variation = 0.f; // size of the particle
    float life_time = 2.f; // how long should a particle be render
};

enum PSenum{
    PS_DRAW_ELEMENTS,
    PS_DRAW_ELEMENTS_BASE_VERTEX
};

class ParticleSystem{

public:

    struct Particle{
        glm::vec3 position;
        glm::vec3 velocity, acceleration;
        glm::vec4 color_begin, color_end;
        float rotation = 0.f;
        float size_begin, size_end;

        float life_time = 1.f;
        float life_remaining = 0.f;

        float distance_from_camera = 0.f;

        bool active = false;
    };

private:

    std::vector<Particle> particle_pool;
    uint32_t pool_index = 999;
    float spawn_rate = 3.f, curr_spawn_rate = -1.f;
    ParticleProps props;

    // particle model
    unsigned int VAO = 0;
    GLenum mode = GL_TRIANGLES;
    GLsizei indices_count = 0;
    GLenum indices_type = GL_UNSIGNED_INT;
    void* indices = nullptr;
    GLint basevertex = 0;

    unsigned int transform_uniform_loc, color_uniform_loc, projview_uniform_loc;

    void psDrawElementsBaseVertex(unsigned int shader_id);
    // void psDrawElements(glm::mat4 projection_view_matrix);

public:
    ParticleSystem();

    void attatchVAO(unsigned int VAO, GLsizei count, GLenum type, void* indices, GLint basevertex = 0);
    void attatchProps(const ParticleProps& props);

    void onUpdate(float time_step, glm::vec3 camera_position);
    void onRender(unsigned int shader_id, glm::mat4 projection_view_matrix);

    void emit(const ParticleProps& props);
};

bool compareParticles(const ParticleSystem::Particle& obj1, const ParticleSystem::Particle& obj2);

#endif // Header

#ifdef STB_PARTICLE_SYSTEM_IMPLEMENTATION

inline void ParticleSystem::psDrawElementsBaseVertex(unsigned int shader_id){
    GLenum error;

    glUseProgram(shader_id);
//  for OpenGL errors
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "stb_particle_system psUseProgram\nOpenGL error: " << error << std::endl;
    }
    glBindVertexArray(this->VAO);
//  for OpenGL errors
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "stb_particle_system psBindVAO\nOpenGL error: " << error << std::endl;
    }
    glDrawElementsBaseVertex(
        this->mode,
        this->indices_count,
        this->indices_type,
        this->indices,
        this->basevertex
    );
    
    //  for OpenGL errors
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "stb_particle_system psDrawElementsBaseVertex\nOpenGL error: " << error << std::endl;
    }

    // glBindVertexArray(0);
    // glUseProgram(0);

}

ParticleSystem::ParticleSystem(){
    particle_pool.resize(1000);
    curr_spawn_rate = 0.f;

}

void ParticleSystem::attatchVAO(unsigned int VAO, GLsizei count, GLenum type, void* indices, GLint basevertex){

    this->VAO = VAO;
    this->indices_count = count;
    this->indices_type = type;
    this->indices = indices;
    this->basevertex = basevertex;

}

void ParticleSystem::attatchProps(const ParticleProps &props){
    this->props = props; 
}

void ParticleSystem::onUpdate(float time_step, glm::vec3 camera_position){

    curr_spawn_rate += time_step;
    if(curr_spawn_rate > spawn_rate){
        this->emit(this->props);
        curr_spawn_rate = 0.f;
        std::cout << "gen\n";   
    }

    for(Particle& part : particle_pool){
        
        if( part.life_remaining <= 0.f )
            part.active = false;
        
        if( !part.active )
            continue;        

        part.life_remaining -= time_step;
        part.position += part.velocity * time_step;
        part.velocity += part.acceleration * time_step;
        part.distance_from_camera = glm::distance(part.position, camera_position);
        // part.rotation += 0.01f * time_step;

    }

    std::sort(particle_pool.begin(), particle_pool.end(), compareParticles);

}

void ParticleSystem::onRender(unsigned int shader_id, glm::mat4 projection_view_matrix){
    // GLenum error;

    glUseProgram(shader_id);

    projview_uniform_loc = glGetUniformLocation(shader_id, "u_ProjView");
    transform_uniform_loc = glGetUniformLocation(shader_id, "u_Transform");
    color_uniform_loc = glGetUniformLocation(shader_id, "u_Color");

	glUniformMatrix4fv(projview_uniform_loc, 1, GL_FALSE, glm::value_ptr(projection_view_matrix));

    for (Particle& particle : particle_pool){
		if (!particle.active)
			continue;

		// Fade away particles
		float life = particle.life_remaining / particle.life_time;
		glm::vec4 color = glm::lerp(particle.color_end, particle.color_begin, life);
		//color.a = color.a * life;

		float size = glm::lerp(particle.size_end, particle.size_begin, life);
		
		// Render
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), particle.position)
			* glm::rotate(glm::mat4(1.0f), particle.rotation, { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { size, size, 1.0f });

		glUniformMatrix4fv(transform_uniform_loc, 1, GL_FALSE, glm::value_ptr(transform));
		glUniform4fv(color_uniform_loc, 1, glm::value_ptr(color));
		
        psDrawElementsBaseVertex(shader_id);
	}

}

void ParticleSystem::emit(const ParticleProps &props){

    Particle& part = this->particle_pool[pool_index];
    part.active = true;
    part.position = props.position;
    part.rotation = ((float) rand() / RAND_MAX) * 2.f * glm::pi<float>();

    // velocity
    part.velocity = props.velocity;
    part.velocity.x += props.velocity_variation.x * ( ((float) rand() / RAND_MAX) - 0.5f );
    part.velocity.y += props.velocity_variation.y * ( ((float) rand() / RAND_MAX) - 0.5f );
    part.velocity.z += props.velocity_variation.z * ( ((float) rand() / RAND_MAX) - 0.5f );

    // color
    part.color_begin = props.color_begin;
    part.color_end = props.color_end;

    part.life_time = props.life_time;
    part.life_remaining = props.life_time;
    part.size_begin = props.size_begin + props.size_variation* ( ((float) rand() / RAND_MAX) - 0.5f );
    part.size_end = props.size_end;

    pool_index = --pool_index % particle_pool.size();

}

// Define a custom comparison function based on your sorting criterion
bool compareParticles(const ParticleSystem::Particle& obj1, const ParticleSystem::Particle& obj2) {
    return obj1.distance_from_camera < obj2.distance_from_camera;
}

#endif // Implementation