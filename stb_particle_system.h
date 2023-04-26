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
    unsigned int transform_uniform_loc, color_uniform_loc;
    // shader and transformations

public:
    ParticleSystem();

    void onUpdate(float time_step);
    void onRender(unsigned int shader_id);

    void Emit(const ParticleProps& props);
};

#endif // Header

#define STB_PARTICLE_SYSTEM_IMPLEMENTATION
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

void ParticleSystem::onRender(unsigned int shader_id){

    if (!quad_VA){

		float vertices[] = {
			 -0.5f, -0.5f, 0.0f,
			  0.5f, -0.5f, 0.0f,
			  0.5f,  0.5f, 0.0f,
			 -0.5f,  0.5f, 0.0f
		};

		glCreateVertexArrays(1, &quad_VA);
		glBindVertexArray(quad_VA);

		GLuint quadVB, quadIB;
		glCreateBuffers(1, &quadVB);
		glBindBuffer(GL_ARRAY_BUFFER, quadVB);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glEnableVertexArrayAttrib(quadVB, 0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

		uint32_t indices[] = {
			0, 1, 2, 2, 3, 0
		};

		glCreateBuffers(1, &quadIB);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIB);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		// m_ParticleShader = std::unique_ptr<GLCore::Utils::Shader>(GLCore::Utils::Shader::FromGLSLTextFiles("assets/shader.glsl.vert", "assets/shader.glsl.frag"));
		// m_ParticleShaderViewProj = glGetUniformLocation(m_ParticleShader->GetRendererID(), "u_ViewProj");
		// m_ParticleShaderTransform = glGetUniformLocation(m_ParticleShader->GetRendererID(), "u_Transform");
		// m_ParticleShaderColor = glGetUniformLocation(m_ParticleShader->GetRendererID(), "u_Color");
        transform_uniform_loc = glGetUniformLocation(shader_id, "u_Transform");
        color_uniform_loc = glGetUniformLocation(shader_id, "u_Color");
	}

    // glUseProgram(m_ParticleShader->GetRendererID());
	// glUniformMatrix4fv(m_ParticleShaderViewProj, 1, GL_FALSE, glm::value_ptr(camera.GetViewProjectionMatrix()));

    for (Particle& particle : particle_pool){
		if (!particle.active)
			continue;

		// Fade away particles
		float life = particle.life_remaining / particle.life_time;
		glm::vec4 color = glm::lerp(particle.color_end, particle.color_begin, life);
		//color.a = color.a * life;

		float size = glm::lerp(particle.size_end, particle.size_begin, life);
		
		// Render
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), { particle.position.x, particle.position.y, 0.0f })
			* glm::rotate(glm::mat4(1.0f), particle.rotation, { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { size, size, 1.0f });

		glUniformMatrix4fv(transform_uniform_loc, 1, GL_FALSE, glm::value_ptr(transform));
		glUniform4fv(color_uniform_loc, 1, glm::value_ptr(color));
		
        glBindVertexArray(quad_VA);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}

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