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

#define RANDOM_VAL ((float) rand() / RAND_MAX) // random float between 0 and 1

// All atributes that can be asigned to the particle system from software
struct ParticleProps{
    glm::vec3 position = glm::vec3(0.f); // where the particles will be generated
    glm::vec3 boundaries[2] = {glm::vec3(0.f), glm::vec3(0.f)};
    glm::vec3 velocity = glm::vec3(0.f), velocity_variation = glm::vec3(0.f), acceleration = glm::vec3(0.f);
    float acceleration_sensitivity = 1.f;
    glm::vec4 color_begin = glm::vec4(1.f), color_end = glm::vec4(1.f), color_variation = glm::vec4(0.f); // color of the particle
    float size_begin = 1.f, size_end = 1.f, size_variation = 0.f; // size of the particle
    float life_time = 2.f, life_time_variation = 0.f; // how long should a particle be render
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
        float acceleration_sensitivity = 1.f;
        glm::vec4 color_begin, color_end;
        float rotation = 0.f;
        float size_begin, size_end;

        float life_time = 1.f;
        float life_remaining = 0.f;

        float distance_from_camera = 0.f;

        bool active = false;
    };

private:

    GLenum blendFactors[10] = {
        GL_ZERO,
        GL_ONE,
        GL_SRC_COLOR,
        GL_ONE_MINUS_SRC_COLOR,
        GL_DST_COLOR,
        GL_ONE_MINUS_DST_COLOR,
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA,
        GL_DST_ALPHA,
        GL_ONE_MINUS_DST_ALPHA
    };
    int curr_sfactor = 6, curr_dfactor = 7;

    GLenum renderModes[3] = {
        GL_POINTS,
        GL_LINES,
        GL_TRIANGLES
    };
    int curr_render_mode_index = 0;

    std::vector<Particle> particle_pool;
    uint32_t pool_index = 9999;
    float spawn_rate = 3.f, curr_spawn_rate = -1.f, spawn_rate_variation = 0.f;
    ParticleProps props;
    bool playing = true;
    bool acceleration_active = true;
    float reproduction_speed = 1.f;

    // particle model
    unsigned int VAO = 0, VBO = 0;
    GLenum mode = GL_TRIANGLES;
    GLsizei indices_count = 0;
    GLenum indices_type = GL_UNSIGNED_INT;
    void* indices = nullptr;
    GLint basevertex = 0;

    bool point_mode = false;
    float point_size = 1.f;

    unsigned int billboard_texture = -1;
    bool use_texture = true;

    GLenum blending_dfactor = GL_ONE_MINUS_SRC_ALPHA;
    bool use_blending = true;

    unsigned int transform_uniform_loc, color_uniform_loc, projview_uniform_loc, size_uniform_loc;

    void psDrawElementsBaseVertex(unsigned int shader_id);
    void psDrawPoint(unsigned int shader_id);
    void cleanVAO();
    // void psDrawElements(glm::mat4 projection_view_matrix);

public:
    ParticleSystem();
    ~ParticleSystem();

    void pointMode();
    void attatchVAO(unsigned int VAO, GLsizei count, GLenum type, void* indices, GLint basevertex = 0);
    void attatchProps(const ParticleProps& props);
    void attatchTexture(unsigned int texture_id);
    void useTexture(bool use);
    bool getUseTexture();

    void onUpdate(float time_step, glm::vec3 camera_position);
    void onRender(unsigned int shader_id, glm::mat4 projection_view_matrix);

    void pause();
    void play();
    void togglePlay();
    bool isPlaying();
    void setReproductionSpeed(float speed);

    void setRenderMode(GLenum mode);
    int getRenderModeInd();
    void setRenderModeInd(int index);
    void setPointSize(float point_size);

    void setBlendFunc(GLenum sfactor, GLenum dfactor);
    void setBlending(bool activate);
    void enableBlending();
    void disableBlending();
    int getSFactor();
    int getDFactor();
    void setSFactor(int factor_index);
    void setDFactor(int factor_index);

    void toggleAcceleration(bool active);
    bool isAccelerationActive();
    void toggleTexture(bool active);

    void emit(const ParticleProps& props);
    void setSpawnRateVariation(float var);

    ParticleProps* getPropsReference();
    float* getSpawnRateReference();
    float* getSpawnRateVarReference();
    float* getReproductionSpeedReference();
    float* getPointSizeReference();
    bool useBlending();
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

void ParticleSystem::psDrawPoint(unsigned int shader_id){
    GLenum error;

    error = glGetError();
    if (error != GL_NO_ERROR) 
        std::cerr << "stb_particle_system psUseProgram\nOpenGL error: " << error << std::endl;

    glUseProgram(shader_id);

    error = glGetError();
    if (error != GL_NO_ERROR) 
        std::cerr << "stb_particle_system psUseProgram\nOpenGL error: " << error << std::endl;
    // Render the point
    glBindVertexArray(this->VAO);
    
    error = glGetError();
    if (error != GL_NO_ERROR) 
        std::cerr << "stb_particle_system psUseProgram\nOpenGL error: " << error << std::endl;
    
    glDrawArrays(GL_POINTS, 0, 1);
    
    error = glGetError();
    if (error != GL_NO_ERROR) 
        std::cerr << "stb_particle_system psUseProgram\nOpenGL error: " << error << std::endl;
    
    // glBindVertexArray(0);
}

void ParticleSystem::cleanVAO(){
    if( point_mode && this->VBO != 0 ){
        glDeleteBuffers(1, &this->VBO);
        glDeleteVertexArrays(1, &this->VAO);
        this->VAO = 0;
        this->VBO = 0;
    }
}

ParticleSystem::ParticleSystem(){
    particle_pool.resize(pool_index+1);
    curr_spawn_rate = 0.f;
}

ParticleSystem::~ParticleSystem(){

    this->cleanVAO();

}

void ParticleSystem::pointMode(){
    point_mode = true;
    
    // Create a vertex array object (VAO) and vertex buffer object (VBO)
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);

    // Bind the VAO and VBO
    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

    // Specify the position of the point
    GLfloat pointPosition[] = { 0.0f, 0.0f, 0.0f };

    // Upload the point position data to the VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(pointPosition), pointPosition, GL_STATIC_DRAW);

    // Set the vertex attribute pointer for the position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Unbind the VAO and VBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void ParticleSystem::attatchVAO(unsigned int VAO, GLsizei count, GLenum type, void *indices, GLint basevertex){

    this->VAO = VAO;
    this->indices_count = count;
    this->indices_type = type;
    this->indices = indices;
    this->basevertex = basevertex;
}

void ParticleSystem::attatchProps(const ParticleProps &props){
    this->props = props; 
}

void ParticleSystem::attatchTexture(unsigned int texture_id){
    this->billboard_texture = texture_id;
}

void ParticleSystem::useTexture(bool use){
    this->use_texture = use;
}

bool ParticleSystem::getUseTexture(){
    return this->use_texture;
}

void ParticleSystem::onUpdate(float time_step, glm::vec3 camera_position){
    
    if(!playing)
        return;
    
    time_step *= reproduction_speed;

    curr_spawn_rate -= time_step;
    if(curr_spawn_rate < 0){
        this->emit(this->props);
        curr_spawn_rate = spawn_rate + (spawn_rate_variation*RANDOM_VAL - 0.5f);
    }

    for(Particle& part : particle_pool){
        
        if( part.life_remaining <= 0.f)
            part.active = false;
        else
            part.active = true;
        
        if( !part.active )
            continue;        

        part.life_remaining -= time_step;
        part.position += part.velocity * time_step;

        if(acceleration_active)
            part.velocity += part.acceleration_sensitivity * part.acceleration * time_step;
        
        part.distance_from_camera = glm::distance(part.position, camera_position);
        // part.rotation += 0.01f * time_step;

    }

    if( this->blending_dfactor == GL_SRC_ALPHA || this->blending_dfactor == GL_ONE_MINUS_SRC_ALPHA)
        std::sort(particle_pool.begin(), particle_pool.end(), compareParticles);

}

void ParticleSystem::onRender(unsigned int shader_id, glm::mat4 projection_view_matrix){
    // GLenum error;

    glUseProgram(shader_id);

    projview_uniform_loc = glGetUniformLocation(shader_id, "u_ProjView");
    transform_uniform_loc = glGetUniformLocation(shader_id, "u_Transform");
    color_uniform_loc = glGetUniformLocation(shader_id, "u_Color");
    size_uniform_loc = glGetUniformLocation(shader_id, "u_Size");

	glUniformMatrix4fv(projview_uniform_loc, 1, GL_FALSE, glm::value_ptr(projection_view_matrix));

    if( use_texture && billboard_texture != -1 ){
        glActiveTexture(GL_TEXTURE0);
        glBindTexture( GL_TEXTURE_2D, billboard_texture );
    }

    GLfloat currPointSize;
    glGetFloatv(GL_POINT_SIZE, &currPointSize); 
    
    glPointSize(point_size);

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
        glUniform1f(size_uniform_loc, size);
		
        if( point_mode )
            psDrawPoint(shader_id);
        else
            psDrawElementsBaseVertex(shader_id);
	
    }

    if( use_texture && billboard_texture != -1){
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glPointSize(currPointSize);

}

void ParticleSystem::pause(){
    this->playing = false;
}

void ParticleSystem::play(){
    this->playing = true;
}

void ParticleSystem::togglePlay(){
    this->playing = !this->playing;
}

bool ParticleSystem::isPlaying(){
    return this->playing;
}

void ParticleSystem::setReproductionSpeed(float speed){
    this->reproduction_speed = speed;
}

void ParticleSystem::setRenderMode(GLenum mode){
    this->mode = mode;
}

int ParticleSystem::getRenderModeInd(){
    return this->curr_render_mode_index;
}

void ParticleSystem::setRenderModeInd(int index){
    curr_render_mode_index = index;
    setRenderMode(renderModes[index]);
    if(index == 0)
        point_mode = true;
    else
        point_mode = false;

}

void ParticleSystem::setPointSize(float point_size){
    glPointSize(point_size);
}

void ParticleSystem::setBlendFunc(GLenum sfactor, GLenum dfactor){
    glBlendFunc(sfactor, dfactor);
}

void ParticleSystem::setBlending(bool activate)
{
    if(activate)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);
}

void ParticleSystem::enableBlending(){
    use_blending = true;
    glEnable(GL_BLEND);
}

void ParticleSystem::disableBlending(){
    use_blending = false;
    glDisable(GL_BLEND);
}

int ParticleSystem::getSFactor(){
    return this->curr_sfactor;
}

int ParticleSystem::getDFactor(){
    return this->curr_dfactor;
}

void ParticleSystem::setSFactor(int factor_index){
    curr_sfactor = factor_index;
    glBlendFunc(blendFactors[curr_sfactor], blendFactors[curr_dfactor]);
}

void ParticleSystem::setDFactor(int factor_index){
    curr_dfactor = factor_index;
    glBlendFunc(blendFactors[curr_sfactor], blendFactors[curr_dfactor]);
}

void ParticleSystem::toggleAcceleration(bool active){
    acceleration_active = active;
}

bool ParticleSystem::isAccelerationActive(){
    return this->acceleration_active;
}

void ParticleSystem::toggleTexture(bool active){
    use_texture = active;
}

void ParticleSystem::emit(const ParticleProps &props){

    Particle& part = this->particle_pool[pool_index];
    part.active = true;

    part.position = props.position;
    part.position.x += glm::lerp(props.boundaries[0].x, props.boundaries[1].x, RANDOM_VAL);
    part.position.y += glm::lerp(props.boundaries[0].y, props.boundaries[1].y, RANDOM_VAL);
    part.position.z += glm::lerp(props.boundaries[0].z, props.boundaries[1].z, RANDOM_VAL);

    part.rotation = RANDOM_VAL * 2.f * glm::pi<float>();

    // velocity
    part.velocity = props.velocity;
    part.velocity.x += props.velocity_variation.x * ( RANDOM_VAL - 0.5f );
    part.velocity.y += props.velocity_variation.y * ( RANDOM_VAL - 0.5f );
    part.velocity.z += props.velocity_variation.z * ( RANDOM_VAL - 0.5f );

    part.acceleration = props.acceleration;

    // color
    part.color_begin = glm::vec4(
        props.color_begin.r + ((RANDOM_VAL-0.5f)*props.color_variation.r),
        props.color_begin.g + ((RANDOM_VAL-0.5f)*props.color_variation.g),
        props.color_begin.b + ((RANDOM_VAL-0.5f)*props.color_variation.b),
        props.color_begin.a + ((RANDOM_VAL-0.5f)*props.color_variation.a)
    );

    part.color_end = glm::vec4(
        props.color_end.r + ((RANDOM_VAL-0.5f)*props.color_variation.r),
        props.color_end.g + ((RANDOM_VAL-0.5f)*props.color_variation.g),
        props.color_end.b + ((RANDOM_VAL-0.5f)*props.color_variation.b),
        props.color_end.a + ((RANDOM_VAL-0.5f)*props.color_variation.a)
    );

    part.life_time = props.life_time + ((RANDOM_VAL-0.5f)*props.life_time_variation);
    part.life_remaining = props.life_time;
    part.size_begin = props.size_begin + props.size_variation* ( RANDOM_VAL - 0.5f );
    part.size_end = props.size_end;
    part.acceleration_sensitivity = props.acceleration_sensitivity;

    pool_index = --pool_index % particle_pool.size();
}

void ParticleSystem::setSpawnRateVariation(float var){
    this->spawn_rate_variation = var;
}

ParticleProps *ParticleSystem::getPropsReference(){
    return &this->props;
}

float *ParticleSystem::getSpawnRateReference(){
    return &this->spawn_rate;
}

float *ParticleSystem::getSpawnRateVarReference(){
    return &this->spawn_rate_variation;
}

float *ParticleSystem::getReproductionSpeedReference(){
    return &this->reproduction_speed;
}

float *ParticleSystem::getPointSizeReference(){
    return &this->point_size;
}

bool ParticleSystem::useBlending(){
    return this->use_blending;
}

// Define a custom comparison function based on your sorting criterion
bool compareParticles(const ParticleSystem::Particle& obj1, const ParticleSystem::Particle& obj2) {
    return obj1.distance_from_camera > obj2.distance_from_camera;
}

#endif // Implementation