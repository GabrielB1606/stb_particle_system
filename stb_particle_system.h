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
//      stdio.h     FILE, fopen, fclose, fseek, ftell
//      stdlib.h    malloc, realloc, free
//      string.h    strcpy, strncmp, memcpy
//
// External libraries:
// 
//      glm 0.9.6.4
// 
// Credits:
//
// Written by Gabriel Belisario

#include "headers.h"

#ifndef STB_PARTICLE_SYSTEM_H
#define STB_PARTICLE_SYSTEM_H

#include <vector>
#include <cstdlib>
#include <ctime>

struct ParticleProps{
    glm::vec2 position;
    glm::vec2 velocity, velocity_variation;
    glm::vec4 color_begin, color_end;
    float size_begin, size_end, size_variation;
    float life_time = 1.f;
};

class ParticleSystem{

    struct Particle{
        glm::vec2 position;
        glm::vec2 velocity;
        glm::vec4 color_begin, color_end;
        float rotation = 0.f;
        float size_begin, size_end;

        float life_time = 1.f;
        float life_remaining = 0.f;

        bool active = false;
    };

    std::vector<Particle> particle_pool;
    uint32_t pool_index = 999;

    unsigned int quad_VA = 0;
    // shader and transformations

public:
    ParticleSystem();

    void onUpdate(float time_step);
    void onRender();

    void Emit(const ParticleProps& props);
};

#endif // Header

#ifdef STB_PARTICLE_SYSTEM_IMPLEMENTATION

ParticleSystem::ParticleSystem(){
    particle_pool.resize(1000);
}

void ParticleSystem::onUpdate(float time_step){

    for(Particle& part : particle_pool){
        
        if( part.life_remaining <= 0.f )
            part.active = false;
        
        if( !part.active )
            continue;        

        part.life_remaining -= time_step;
        part.position += part.velocity * time_step;
        part.rotation += 0.01f * time_step;

    }

}

void ParticleSystem::onRender(){
}

void ParticleSystem::Emit(const ParticleProps &props){

    Particle& part = this->particle_pool[pool_index];
    part.active = true;
    part.position = props.position;
    part.rotation = ((float) rand() / RAND_MAX) * 2.f * glm::pi<float>();

    // velocity
    part.velocity = props.velocity;
    part.velocity.x += props.velocity_variation.x * ( ((float) rand() / RAND_MAX) - 0.5f );
    part.velocity.y += props.velocity_variation.y * ( ((float) rand() / RAND_MAX) - 0.5f );

    // color
    part.color_begin = props.color_begin;
    part.color_end = props.color_end;

    part.life_time = props.life_time;
    part.life_remaining = props.life_time;
    part.size_begin = props.size_begin + props.size_variation* ( ((float) rand() / RAND_MAX) - 0.5f );
    part.size_end = props.size_end;

    pool_index = --pool_index % particle_pool.size();

}

#endif // Implementation