#include "../src/a1.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <ctime>
// Program with perspective correct interpolation of vertex attributes.

namespace R = COL781::Software;
// namespace R = COL781::Hardware;
using namespace glm;

void createMinuteSymbol(int minute, R::Rasterizer &r, R::ShaderProgram &program, R::Object &shape) {
        mat4 model = mat4(0.02f, 0.0f,  0.0f, 0.0f,
                          0.0f,  0.05f, 0.0f, 0.0f,
                          0.0f,  0.0f,  1.0f, 0.0f,
                          0.0f,  0.0f,  0.0f, 1.0f );
        mat4 view = translate(mat4(1.0f), vec3(0.0f, 0.475f, 0.0f));
        float angle = minute*6.0f ; // degrees per second
        mat4 rotation = rotate(mat4(1.0f), -radians(angle), vec3(0.0f,0.0f,1.0f));
        r.setUniform(program, "transform", rotation * view * model);
        r.setUniform(program, "color", vec4(0.0, 0.0, 0.0, 1.0));
		r.drawObject(shape);
}

void createHourSymbol(int hour, R::Rasterizer &r, R::ShaderProgram &program, R::Object &shape) {
        mat4 model = mat4(0.05f, 0.0f, 0.0f, 0.0f,
                          0.0f,  0.1f, 0.0f, 0.0f,
                          0.0f,  0.0f, 1.0f, 0.0f,
                          0.0f,  0.0f, 0.0f, 1.0f );
        mat4 view = translate(mat4(1.0f), vec3(0.0f, 0.45f, 0.0f));
        float angle = hour*30.0f ; // degrees per second
        mat4 rotation = rotate(mat4(1.0f), -radians(angle), vec3(0.0f,0.0f,1.0f));
        r.setUniform(program, "transform", rotation * view * model);
        r.setUniform(program, "color", vec4(0.0, 0.0, 0.0, 1.0));
		r.drawObject(shape);
}

int main() {
	R::Rasterizer r;
	int width = 480, height = 480;
    if (!r.initialize("Example 6", width, height))
        return EXIT_FAILURE;

    R::ShaderProgram program = r.createShaderProgram(
        r.vsColorTransform(),
        r.fsConstant()
    );
    vec4 vertices[] = {
        vec4( -0.5,  -0.5, 0.0, 1.0),
        vec4(  0.5,  -0.5, 0.0, 1.0),
		vec4( -0.5,   0.5, 0.0, 1.0),
        vec4(  0.5,   0.5, 0.0, 1.0)
    };
	ivec3 triangles[] = {
		ivec3(0, 1, 2),
		ivec3(1, 2, 3)
	};

	R::Object shape = r.createObject();
	r.setVertexAttribs(shape, 0, 4, vertices);
	r.setTriangleIndices(shape, 2, triangles);
    r.enableDepthTest();
    
    while (!r.shouldQuit()) {
        std::time_t currentTime = std::time(nullptr);

        // Convert the current time to a local time structure
        std::tm* localTime = std::localtime(&currentTime);
    
        // Extract hour, minute, and second from the local time structure
        int hour = localTime->tm_hour;
        hour = hour%12;
        int minute = localTime->tm_min;
        int second = localTime->tm_sec;
        float time = SDL_GetTicks()*1e-3;
        r.clear(vec4(1.0, 1.0, 1.0, 1.0));
        r.useShaderProgram(program);

        mat4 hour_hand = mat4(0.05f, 0.0f, 0.0f, 0.0f,
                              0.0f,  0.3f, 0.0f, 0.0f,
                              0.0f,  0.0f, 1.0f, 0.0f,
                              0.0f,  0.0f, 0.0f, 1.0f );
        mat4 hour_view = translate(mat4(1.0f), vec3(0.0f, 0.15f, 0.0f));
        float hour_angle = (hour*3600.0f + minute*60.0f + second*1.0f)/(120.0f) ; // degrees per second
        mat4 hour_model = rotate(mat4(1.0f), -radians(hour_angle), vec3(0.0f,0.0f,1.0f));
        r.setUniform(program, "transform", hour_model * hour_view *hour_hand);
        r.setUniform(program, "color", vec4(0.0, 0.0, 0.0, 1.0));
		r.drawObject(shape);

        mat4 minute_hand = mat4(0.03f, 0.0f,  0.0f, 0.0f,
                                0.0f,  0.40f, 0.0f, 0.0f,
                                0.0f,  0.0f,  1.0f, 0.0f,
                                0.0f,  0.0f,  0.0f, 1.0f );
        mat4 minute_view = translate(mat4(1.0f), vec3(0.0f, 0.2f, -0.05f));
        float minute_angle = (minute*360.0f + second*1.0)/(60.0f); // degrees per second
        mat4 minute_model = rotate(mat4(1.0f), -radians(minute_angle), vec3(0.0f,0.0f,1.0f));
        r.setUniform(program, "transform", minute_model * minute_view * minute_hand);
        r.setUniform(program, "color", vec4(0.0, 0.0, 0.0, 1.0));
		r.drawObject(shape);

        mat4 second_hand = mat4(0.01f, 0.0f,  0.0f, 0.0f,
                                0.0f,  0.40f, 0.0f, 0.0f,
                                0.0f,  0.0f,  1.0f, 0.0f,
                                0.0f,  0.0f,  0.0f, 1.0f );
        mat4 second_view = translate(mat4(1.0f), vec3(0.0f, 0.2f, -0.1f));
        float second_angle = (second*6.0f); // degrees per second
        mat4 second_model = rotate(mat4(1.0f), -radians(second_angle), vec3(0.0f,0.0f,1.0f));
        r.setUniform(program, "transform", second_model * second_view * second_hand);
        r.setUniform(program, "color", vec4(1.0, 0.0, 0.0, 1.0));
		r.drawObject(shape);

        for(int i = 0; i < 60; i++) {
            if(i%5 == 0){
                createHourSymbol(i/5, r, program, shape);
            }
            else{
                createMinuteSymbol(i, r, program, shape);
            }
        }

        r.show();

    }
    r.deleteShaderProgram(program);
    return EXIT_SUCCESS;
}