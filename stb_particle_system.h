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

#ifndef STB_PARTICLE_SYSTEM_H
#define STB_PARTICLE_SYSTEM_H

struct ParticleProps{
    glm::vec2 position;
    glm::vec2 velocity, velocity_variation;
    glm::vec4 color_begin, color_end;
    float size_begin, size_end, size_variation;
    float life_time = 1.f;
};

#endif // Header

#ifdef STB_PARTICLE_SYSTEM_IMPLEMENTATION
#endif // Implementation