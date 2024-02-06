#include "../src/a1.hpp"
#include "../src/sw.cpp"
#include <glm/gtc/matrix_transform.hpp>
#include <ctime>
// Program with perspective correct interpolation of vertex attributes.

// namespace R = COL781::Software;
namespace R = COL781::Hardware;
using namespace glm;


class Camera{
    public:

        Camera(){
            Eye = vec3(0.0f, 0.0f, 0.0f);
            ViewDir = vec3(0.0f, 0.0f, -1.0f);
            UpVec = vec3(0.0f, 1.0f, 0.0f);
        }

        Camera(vec3 eye, vec3 viewDir, vec3 upVec){
            Eye = eye;
            ViewDir = viewDir;
            UpVec = upVec;
        }

        mat4 getViewMatrix() const{
            return lookAt(Eye, ViewDir, UpVec);
        }

        void moveForward(float delta){
            Eye += delta * ViewDir;
        }

        void moveRight(float delta){
            Eye += delta * cross(ViewDir, UpVec);
        }

        void moveUp(float delta){
            Eye += delta * UpVec;
        }

        void rotateRight(float angle){
            ViewDir = rotate(mat4(1.0f), -radians(angle), UpVec) * vec4(ViewDir, 0.0f);
        }

        void rotateUp(float angle){
            vec3 right = cross(ViewDir, UpVec);
            ViewDir = rotate(mat4(1.0f), -radians(angle), right) * vec4(ViewDir, 0.0f);
            UpVec = rotate(mat4(1.0f), -radians(angle), right) * vec4(UpVec, 0.0f);
        }


    private:
        vec3 Eye;
        vec3 ViewDir;
        vec3 UpVec;
};

void createGround (R::Rasterizer &r, R::ShaderProgram &program, R::Object &shape, mat4 view, mat4 projection, float Scale) {
    
    mat4 groundScale = scale(mat4(1.0f), vec3(Scale, Scale, Scale));
    mat4 groundModel = translate(mat4(1.0f), vec3(0.0f, -3.0f, -0.5f));
    r.setUniform(program, "transform", projection * view * groundModel * groundScale);
    r.drawObject(shape);

    mat4 groundModel2 = translate(mat4(1.0f), vec3(0.0f , -3.0f, 0.5f));
    r.setUniform(program, "transform", projection * view * groundModel2 * groundScale);
    r.drawObject(shape);
}

void createPerson (R::Rasterizer &r, R::ShaderProgram &program, R::Object &shape, mat4 view, mat4 projection) {
    
        mat4 headScale = translate(mat4(1.0f), vec3(1.7f, 0.5f, 0.0f));
        r.setUniform(program, "transform", projection * view * headScale);
        r.drawObject(shape);

        mat4 bodyScale = scale(mat4(1.0f), vec3(0.6f, 1.7f, 0.9f)); 
        mat4 bodyModel = translate(mat4(1.0f), vec3(1.7f, -0.85f, 0.0f));
        r.setUniform(program, "transform", projection * view * bodyModel * bodyScale);
        r.drawObject(shape);
 
        mat4 ArmScale = scale(mat4(1.0f), vec3(0.3f, 1.6f, 0.3f));
        mat4 ArmTranslateDown = translate(mat4(1.0f), vec3(0.0f, -0.8f, 0.0f));

        mat4 leftArmRotation = rotate(mat4(1.0f), -radians(45.0f), vec3(0.0f,0.0f,1.0f));
        mat4 leftArmModel = translate(mat4(1.0f), vec3(1.9f, 0.0f, 0.6f));
        r.setUniform(program, "transform", projection * view * leftArmModel * leftArmRotation * ArmTranslateDown * ArmScale);
        r.drawObject(shape);
 
        mat4 rightArmRotation = rotate(mat4(1.0f), radians(45.0f), vec3(0.0f,0.0f,1.0f));
        mat4 rightArmModel = translate(mat4(1.0f), vec3(1.9f, 0.0f, -0.6f));
        r.setUniform(program, "transform", projection * view * rightArmModel * rightArmRotation * ArmTranslateDown * ArmScale);
        r.drawObject(shape);
 
        mat4 LegScale = scale(mat4(1.0f), vec3(0.4f, 1.7f, 0.4f));
        mat4 LegTranslateDown = translate(mat4(1.0f), vec3(0.0f, -0.85f, 0.0f));

        mat4 LeftLegRotatation = rotate(mat4(1.0f), radians(45.0f), vec3(0.0f,0.0f,1.0f));
        mat4 LeftLegModel = translate(mat4(1.0f), vec3(1.7f, -1.7f, 0.25f));
        r.setUniform(program, "transform", projection * view * LeftLegModel * LeftLegRotatation * LegTranslateDown * LegScale);
        
        r.drawObject(shape);
        
        mat4 RightLegRotatation = rotate(mat4(1.0f), -radians(45.0f), vec3(0.0f,0.0f,1.0f));
        mat4 RightLegModel = translate(mat4(1.0f), vec3(1.7f, -1.7f, -0.25f));
        r.setUniform(program, "transform", projection * view * RightLegModel * RightLegRotatation * LegTranslateDown * LegScale);
        r.drawObject(shape);
}

void createFlag(R::Rasterizer &r, R::ShaderProgram &program, R::Object &flagPole, R::Object &Flag, mat4 view, mat4 projection){
        
        float time = SDL_GetTicks()*1e-3;
        mat4 flagPoleScale = scale(mat4(1.0f), vec3(0.3f, 5.5f, 0.3f));
        mat4 flagPoleModel = translate(mat4(1.0f), vec3(0.0f, -3.0f, 0.0f));
        r.setUniform(program, "transform", projection * view * flagPoleModel * flagPoleScale);
        r.drawObject(flagPole);

        mat4 flagScale = scale(mat4(1.0f), vec3(1.4f, 1.0f, 1.0f));
        mat4 flagModel = translate(mat4(1.0f), vec3(-2.3f, 6.3f, 0.0f));
        mat4 flagRotate = rotate(mat4(1.0f), radians(time*40.0f), vec3(0.0f,1.0f,0.0f));
        r.setUniform(program, "transform", projection * view * flagRotate * flagModel * flagScale);
        r.drawObject(Flag);
}

int main() {
	R::Rasterizer r;
	int width = 480, height = 480;
    if (!r.initialize("Example 6", width, height))
        return EXIT_FAILURE;

    R::ShaderProgram program = r.createShaderProgram(
        r.vsColorTransform(),
        r.fsIdentity()
    );

    R::Object cube = r.createObject();
	{
		vec4 vertices[] = {
            vec4( -0.5,  -0.5, 0.5, 1.0),
            vec4(  0.5,  -0.5, 0.5, 1.0),
            vec4( -0.5,   0.5, 0.5, 1.0),
            vec4(  0.5,   0.5, 0.5, 1.0),
            vec4( -0.5,  -0.5, -0.5, 1.0),
            vec4(  0.5,  -0.5, -0.5, 1.0),
            vec4( -0.5,   0.5, -0.5, 1.0),
            vec4(  0.5,   0.5, -0.5, 1.0)
		};
		ivec3 triangles[] = {
			ivec3(0, 1, 2),
            ivec3(1, 2, 3),
            ivec3(4, 5, 6),
            ivec3(5, 6, 7),
            ivec3(0, 1, 4),
            ivec3(1, 4, 5),
            ivec3(2, 3, 6),
            ivec3(3, 6, 7),
            ivec3(0, 2, 4),
            ivec3(2, 4, 6),
            ivec3(1, 3, 5),
            ivec3(3, 5, 7)
		};
        vec4 colors[] = {
		vec4(0.7421875, 0.4453125, 0.234375, 1.0),
        vec4(0.7421875, 0.4453125, 0.234375, 1.0),
        vec4(0.0, 0.0, 0.0, 1.0),
        vec4(0.0, 0.0, 0.0, 1.0),
        vec4(0.7421875, 0.4453125, 0.234375, 1.0),
        vec4(0.7421875, 0.4453125, 0.234375, 1.0),
        vec4(0.0, 0.0, 0.0, 1.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    };
		r.setVertexAttribs(cube, 0, 8, vertices);
        r.setVertexAttribs(cube, 1, 8, colors);
		r.setTriangleIndices(cube, 12, triangles);
	}
	
    R::Object Ground = r.createObject();
    {
        vec4 vertices[] = {
            vec4( -5.0,  0.0,  5.0, 1.0),
            vec4(  5.0,  0.0,  5.0, 1.0),
            vec4( -5.0,  0.0, -5.0, 1.0),
            vec4(  5.0,  0.0, -5.0, 1.0)
        };
        ivec3 triangles[] = {
            ivec3(0, 1, 2),
            ivec3(1, 2, 3)
        };
        vec4 colors[] = {
            vec4(0.0, 1.0, 0.0, 1.0),
            vec4(0.0, 1.0, 0.0, 1.0),
            vec4(0.0, 1.0, 0.0, 1.0),
            vec4(0.0, 1.0, 0.0, 1.0)
        };

        r.setVertexAttribs(Ground, 0, 4, vertices);   
        r.setVertexAttribs(Ground, 1, 4, colors);
        r.setTriangleIndices(Ground, 2, triangles);
    }

    R::Object flagPole = r.createObject();
    {
        vec4 vertices[] = {
            vec4( 0.5, 0.0, sin(60.0), 1.0),
            vec4(-0.5, 0.0, sin(60.0), 1.0),
            vec4(-0.5, 0.0,-sin(60.0), 1.0),
            vec4( 0.5, 0.0,-sin(60.0), 1.0),
            vec4( 1.0, 0.0, 0.0, 1.0),
            vec4(-1.0, 0.0, 0.0, 1.0),
            vec4( 0.5, 2.0, sin(60.0), 1.0),
            vec4(-0.5, 2.0, sin(60.0), 1.0),
            vec4(-0.5, 2.0,-sin(60.0), 1.0),
            vec4( 0.5, 2.0,-sin(60.0), 1.0),
            vec4( 1.0, 2.0, 0.0, 1.0),
            vec4(-1.0, 2.0, 0.0, 1.0)
        };
        ivec3 triangles[] = {
            ivec3(0,1,4),
            ivec3(2,3,4),
            ivec3(5,1,4),
            ivec3(5,3,4),
            ivec3(6,7,10),
            ivec3(8,9,10),
            ivec3(11,7,10),
            ivec3(11,9,10),
            ivec3(0,1,6),
            ivec3(1,6,7),
            ivec3(1,5,7),
            ivec3(5,7,10),
            ivec3(5,2,11),
            ivec3(2,11,8),
            ivec3(2,3,8),
            ivec3(3,8,9),
            ivec3(3,4,9),
            ivec3(4,9,10),
            ivec3(4,0,10),
            ivec3(0,10,6)
        };
        vec4 colors[] = {
            vec4(0.0, 0.0, 0.0, 1.0), vec4(0.0, 0.0, 0.0, 1.0), vec4(0.0, 0.0, 0.0, 1.0), vec4(0.0, 0.0, 0.0, 1.0),
            vec4(0.0, 0.0, 0.0, 1.0), vec4(0.0, 0.0, 0.0, 1.0), vec4(0.3765, 0.5098, 0.7137, 1.0),vec4(0.3765, 0.5098, 0.7137, 1.0),
            vec4(0.3765, 0.5098, 0.7137, 1.0),vec4(0.3765, 0.5098, 0.7137, 1.0), vec4(0.3765, 0.5098, 0.7137, 1.0),vec4(0.3765, 0.5098, 0.7137, 1.0)
        };
        r.setVertexAttribs(flagPole, 0, 12, vertices);
        r.setVertexAttribs(flagPole, 1, 12, colors);
        r.setTriangleIndices(flagPole, 20, triangles);
    }

    
    R::Object Flag = r.createObject();{
        vec4 vertices[] = {
            vec4( 1.5,  0.2, 0.0, 1.0),
            vec4(-1.5,  0.2, 0.0, 1.0),
            vec4( 1.5,  1.5, 0.0, 1.0),
            vec4(-1.5,  1.5, 0.0, 1.0),
            vec4( 1.5, -0.2, 0.0, 1.0),
            vec4(-1.5, -0.2, 0.0, 1.0),
            vec4( 1.5, -1.5, 0.0, 1.0),
            vec4(-1.5, -1.5, 0.0, 1.0),
        };
        ivec3 triangles[] = {
            ivec3(0, 1, 2),
            ivec3(1, 2, 3),
            ivec3(0, 1, 5), 
            ivec3(0, 4, 5),
            ivec3(4, 5, 6),
            ivec3(5, 6, 7)
        };
        vec4 colors[] = {
            vec4(1.0, 1.0, 1.0, 1.0),
            vec4(1.0, 1.0, 1.0, 1.0),
            vec4(0.9568, 0.7686, 0.1882, 1.0),
            vec4(0.9568, 0.7686, 0.1882, 1.0),
            vec4(1.0, 1.0, 1.0, 1.0),
            vec4(1.0, 1.0, 1.0, 1.0),
            vec4(0.0, 1.0, 0.0, 1.0),
            vec4(0.0, 1.0, 0.0, 1.0)
        };
        r.setVertexAttribs(Flag, 0, 8, vertices);
        r.setVertexAttribs(Flag, 1, 8, colors);
        r.setTriangleIndices(Flag, 6, triangles);
    }

    r.enableDepthTest();
    Camera myCamera(vec3(-10.0f, 3.0f, -7.0f), vec3(1.0f, 2.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f));
    myCamera.moveRight(-1.5f);
    
    mat4 projection = perspective(radians(60.0f), (float)width/(float)height, 0.1f, 100.0f);

    while (!r.shouldQuit()) {
        r.clear(vec4(0.0, 0.0, 1.0, 1.0));
        r.useShaderProgram(program);
        

        mat4 view = myCamera.getViewMatrix();
        
        createPerson(r, program, cube, view, projection);

        createGround(r, program, Ground, view, projection,1.5);

        createFlag(r, program, flagPole, Flag, view, projection);

        std::time_t currentTime = std::time(nullptr);
        std::tm* localTime = std::localtime(&currentTime);
        int second = localTime->tm_sec;
        second %= 6;
        if(second <3){
            myCamera.moveForward(0.01f);
            myCamera.moveRight(0.3f);
            myCamera.rotateRight(3.0f);
        }
        else{
            myCamera.moveForward(-0.01f);
            myCamera.moveRight(-0.3f);
            myCamera.rotateRight(-3.0f);
        }

        r.show();

    }
    r.deleteShaderProgram(program);
    return EXIT_SUCCESS;
}
