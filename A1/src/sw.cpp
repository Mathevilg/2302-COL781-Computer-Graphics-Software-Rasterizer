#include "sw.hpp"

#include <iostream>
#include <vector>

namespace COL781 {
	namespace Software {

		// Forward declarations

		template <> float Attribs::get(int index) const;
		template <> glm::vec2 Attribs::get(int index) const;
		template <> glm::vec3 Attribs::get(int index) const;
		template <> glm::vec4 Attribs::get(int index) const;

		template <> void Attribs::set(int index, float value);
		template <> void Attribs::set(int index, glm::vec2 value);
		template <> void Attribs::set(int index, glm::vec3 value);
		template <> void Attribs::set(int index, glm::vec4 value);

		// Built-in shaders

		VertexShader Rasterizer::vsIdentity() {
			return [](const Uniforms &uniforms, const Attribs &in, Attribs &out) {
				glm::vec4 vertex = in.get<glm::vec4>(0);
				// std::cout << "vs : " << vertex[0] << vertex[1] << vertex[2] << vertex[3] << "\n";
				return vertex;
			};
		}

		VertexShader Rasterizer::vsTransform() {
			return [](const Uniforms &uniforms, const Attribs &in, Attribs &out) {
				glm::vec4 vertex = in.get<glm::vec4>(0);
				glm::mat4 transform = uniforms.get<glm::mat4>("transform");
				return transform * vertex;
			};
		}

		VertexShader Rasterizer::vsColor() {
			return [](const Uniforms &uniforms, const Attribs &in, Attribs &out) {
				glm::vec4 vertex = in.get<glm::vec4>(0);
				glm::vec4 color = in.get<glm::vec4>(1);
				out.set<glm::vec4>(0, color);
				return vertex;
			};
		}

		// this vs was to be implemented
		VertexShader Rasterizer::vsColorTransform(){
			return [](const Uniforms &uniforms, const Attribs &in, Attribs &out) {
				glm::vec4 vertex = in.get<glm::vec4>(0);
				glm::vec4 color = in.get<glm::vec4>(1);
				out.set<glm::vec4>(0, color);
				glm::mat4 transform = uniforms.get<glm::mat4>("transform");
				return transform * vertex;
			};
		}

		FragmentShader Rasterizer::fsConstant() {
			return [](const Uniforms &uniforms, const Attribs &in) {
				glm::vec4 color = uniforms.get<glm::vec4>("color");
				return color;
			};
		}

		FragmentShader Rasterizer::fsIdentity() {
			return [](const Uniforms &uniforms, const Attribs &in) {
				glm::vec4 color = in.get<glm::vec4>(0);
				return color;
			};
		}

		// Implementation of Attribs and Uniforms classes

		void checkDimension(int index, int actual, int requested) {
			if (actual != requested) {
				// std::cout << "Warning: attribute " << index << " has dimension " << actual << " but accessed as dimension " << requested << std::endl;
			}
		}

		template <> float Attribs::get(int index) const {
			checkDimension(index, dims[index], 1);
			return values[index].x;
		}

		template <> glm::vec2 Attribs::get(int index) const {
			checkDimension(index, dims[index], 2);
			return glm::vec2(values[index].x, values[index].y);
		}

		template <> glm::vec3 Attribs::get(int index) const {
			checkDimension(index, dims[index], 3);
			return glm::vec3(values[index].x, values[index].y, values[index].z);
		}

		template <> glm::vec4 Attribs::get(int index) const {
			checkDimension(index, dims[index], 4);
			return values[index];
		}

		void expand(std::vector<int> &dims, std::vector<glm::vec4> &values, int index) {
			if (dims.size() < index+1)
				dims.resize(index+1);
			if (values.size() < index+1)
				values.resize(index+1);
		}

		template <> void Attribs::set(int index, float value) {
			expand(dims, values, index);
			dims[index] = 1;
			values[index].x = value;
		}

		template <> void Attribs::set(int index, glm::vec2 value) {
			expand(dims, values, index);
			dims[index] = 2;
			values[index].x = value.x;
			values[index].y = value.y;
		}

		template <> void Attribs::set(int index, glm::vec3 value) {
			expand(dims, values, index);
			dims[index] = 3;
			values[index].x = value.x;
			values[index].y = value.y;
			values[index].z = value.z;
		}

		template <> void Attribs::set(int index, glm::vec4 value) {
			expand(dims, values, index);
			dims[index] = 4;
			values[index] = value;
		}

		template <typename T> T Uniforms::get(const std::string &name) const {
			return *(T*)values.at(name);
		}

		template <typename T> void Uniforms::set(const std::string &name, T value) {
			auto it = values.find(name);
			if (it != values.end()) {
				delete it->second;
			}
			values[name] = (void*)(new T(value));
		}


		// Creates a window with the given title, size, and samples per pixel.
		bool Rasterizer::initialize(const std::string &title, int width, int height, int spp){
			int screenWidth = width;
			int screenHeight = height;
			supersampling_n = spp;
			window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
			if (window == NULL) {
				printf("Window could not be created! SDL_Error: %s", SDL_GetError());
				return false;
			}
			quit = false;
			zbuffering = false;
			return true;
		}
		
		// Creates a new shader program, i.e. a pair of a vertex shader and a fragment shader.
		ShaderProgram Rasterizer::createShaderProgram(const VertexShader &vs, const FragmentShader &fs){
			ShaderProgram new_shader_prog;
			new_shader_prog.vs = vs;
			new_shader_prog.fs = fs;
			return new_shader_prog;
		}

		// Creates an object, i.e. a collection of vertices and triangles.
		Object Rasterizer::createObject(){
			Object new_object;
			return new_object;
		}


		// Sets the data for the i'th vertex attribute. (T is vec4)
		template <> void Rasterizer::setVertexAttribs(Object &object, int attribIndex, int n, const glm::vec4* data){
			std::vector<float> v;
			for (int i=0; i<n; i++){
				v.push_back(data[i].x);
				v.push_back(data[i].y);
				v.push_back(data[i].z);
				v.push_back(data[i].w);
			}
			if (attribIndex>=object.attributeDims.size()) {
				object.attributeDims.push_back(4);
				object.attributeValues.push_back(v);
			}
			else {
				object.attributeDims[attribIndex] = 4;
				object.attributeValues[attribIndex] = v;
			}
		}

		// Sets the data for the i'th vertex attribute. (T is vec3)
		template <> void Rasterizer::setVertexAttribs(Object &object, int attribIndex, int n, const glm::vec3* data){
			std::vector<float> v;
			for (int i=0; i<n; i++){
				v.push_back(data[i].x);
				v.push_back(data[i].y);
				v.push_back(data[i].z);
				// v.push_back(data[i].w);
			}
			if (attribIndex>=object.attributeDims.size()) {
				object.attributeDims.push_back(3);
				object.attributeValues.push_back(v);
			}
			else {
				object.attributeDims[attribIndex] = 3;
				object.attributeValues[attribIndex] = v;
			}
		}

		// Sets the data for the i'th vertex attribute. (T is vec2)
		template <> void Rasterizer::setVertexAttribs(Object &object, int attribIndex, int n, const glm::vec2* data){
			std::vector<float> v;
			for (int i=0; i<n; i++){
				v.push_back(data[i].x);
				v.push_back(data[i].y);
				// v.push_back(data[i].z);
				// v.push_back(data[i].w);
			}
			if (attribIndex>=object.attributeDims.size()) {
				object.attributeDims.push_back(2);
				object.attributeValues.push_back(v);
			}
			else {
				object.attributeDims[attribIndex] = 2;
				object.attributeValues[attribIndex] = v;
			}
		}



		// Sets the indices of the triangles.
		void Rasterizer::setTriangleIndices(Object &object, int n, glm::ivec3* indices){
			for (int i=0; i<n; i++){
				object.indices.push_back(indices[i]);
			}
		};

		// Returns true if the user has requested to quit the program.
		bool Rasterizer::shouldQuit(){
			return quit;
		}

		// Clear the framebuffer, setting all pixels to the given color.
		void Rasterizer::clear(glm::vec4 color){
			int width, height;
    		SDL_GetWindowSize(window, &width, &height);
			// std::cout << width << " " << height << " \n";
			SDL_Surface* windowSurface = SDL_GetWindowSurface(window);
            framebuffer = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
			// std::cout << windowSurface << " " << framebuffer << "\n";
			Uint32 *pixels = (Uint32*)framebuffer->pixels;
            SDL_PixelFormat *format = framebuffer->format;
            for (int i = 0; i < width; i++) {
                for (int j = 0; j < height; j++) {
                    Uint32 colorij = SDL_MapRGBA(format, 255*color.r, 255*color.g, 255*color.b, 255*color.a);
                    pixels[i + width*j] = colorij;
                }
            }

			// clear the z buffer as well
			if (zbuffering){
				for (int i=1; i<width; i++){
					for (int j=0; j<height; j++){
						zbuffer[i][j]=FLT_MAX;				
					}
				}
			}
			// SDL_BlitScaled(framebuffer, NULL, windowSurface, NULL);
            // SDL_UpdateWindowSurface(window);
		}

		// Makes the given shader program active. Future draw calls will use its vertex and fragment shaders.
		void Rasterizer::useShaderProgram(const ShaderProgram &program){
			rasterizerProgram = program;
			return;
		}

		template <> void Rasterizer::setUniform(ShaderProgram &program, const std::string &name, glm::vec2 value){
			program.uniforms.set(name, value);
			rasterizerProgram = program;
		}
		template <> void Rasterizer::setUniform(ShaderProgram &program, const std::string &name, glm::vec3 value){
			program.uniforms.set(name, value);
			rasterizerProgram = program;
		}
		template <> void Rasterizer::setUniform(ShaderProgram &program, const std::string &name, glm::vec4 value){
			program.uniforms.set(name, value);
			rasterizerProgram = program;
		}
		template <> void Rasterizer::setUniform(ShaderProgram &program, const std::string &name, glm::mat2 value){
			program.uniforms.set(name, value);
			rasterizerProgram = program;
		}
		template <> void Rasterizer::setUniform(ShaderProgram &program, const std::string &name, glm::mat3 value){
			program.uniforms.set(name, value);
			rasterizerProgram = program;
		}
		template <> void Rasterizer::setUniform(ShaderProgram &program, const std::string &name, glm::mat4 value){
			program.uniforms.set(name, value);
			rasterizerProgram = program;
		}
		template <> void Rasterizer::setUniform(ShaderProgram &program, const std::string &name, float value){
			program.uniforms.set(name, value);
			rasterizerProgram = program;
		}
		template <> void Rasterizer::setUniform(ShaderProgram &program, const std::string &name, int value){
			program.uniforms.set(name, value);
			rasterizerProgram = program;
		}

		// template <typename T> void setUniform(ShaderProgram &program, const std::string &name, T value){
		// 	program.uniforms.set(name, value);
		// 	rasterizerProgram = program;
		// }


		// 
		void Rasterizer::drawObject(const Object &object){
			int width, height;
    		SDL_GetWindowSize(window, &width, &height);
			SDL_Surface* windowSurface = SDL_GetWindowSurface(window);
			// SDL_Surface* framebuffer = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
				Uint32 *pixels = (Uint32*)framebuffer->pixels;
				SDL_PixelFormat *format = framebuffer->format;
			for (auto triangle : object.indices){
				
				int v1_index, v2_index, v3_index;
				v1_index = triangle.x;
				v2_index = triangle.y;
				v3_index = triangle.z;
				// std::cout<<v1_index<<v2_index<<v3_index<<"\n";
				Attribs v1, v2, v3;
				for (int i = 0; i<object.attributeDims.size(); i++){
					int dim = object.attributeDims[i];
					std::vector<float> val = object.attributeValues[i];
					switch (dim) {
						case 1:
							v1.set(i, val[v1_index]);
							v2.set(i, val[v1_index]);
							v3.set(i, val[v1_index]);
						case 2:	
							v1.set(i, glm::vec2(val[2*v1_index], val[2*v1_index+1]));
							v2.set(i, glm::vec2(val[2*v2_index], val[2*v2_index+1]));
							v3.set(i, glm::vec2(val[2*v3_index], val[2*v3_index+1]));
						case 3:
							v1.set(i, glm::vec3(val[3*v1_index], val[3*v1_index+1], val[3*v1_index+2]));
							v2.set(i, glm::vec3(val[3*v2_index], val[3*v2_index+1], val[3*v2_index+2]));
							v3.set(i, glm::vec3(val[3*v3_index], val[3*v3_index+1], val[3*v3_index+2]));
						case 4:
							// std::cout << "case4\n";
							// std::cout << val[4*v1_index] << val[4*v1_index+1] << val[4*v1_index+2] << val[4*v1_index+3] << "\n";
							v1.set(i, glm::vec4(val[4*v1_index], val[4*v1_index+1], val[4*v1_index+2], val[4*v1_index+3]));
							v2.set(i, glm::vec4(val[4*v2_index], val[4*v2_index+1], val[4*v2_index+2], val[4*v2_index+3]));
							v3.set(i, glm::vec4(val[4*v3_index], val[4*v3_index+1], val[4*v3_index+2], val[4*v3_index+3]));
					}					
				} 
				Attribs v1_out, v2_out, v3_out;
				v1_out = v1; v2_out = v2; v3_out = v3;
				glm::vec4 v1_ndc, v2_ndc, v3_ndc;
				// std::cout << v1_ndc[0]  << v1_ndc[1] << v2_ndc[0] << "\n";
				v1_ndc = rasterizerProgram.vs(rasterizerProgram.uniforms, v1, v1_out);
				v2_ndc = rasterizerProgram.vs(rasterizerProgram.uniforms, v2, v2_out);
				v3_ndc = rasterizerProgram.vs(rasterizerProgram.uniforms, v3, v3_out);


				// To support 3D tringles, we include the perspective division stage after the vertex shader
				// v1_out = glm::vec4(v1_ndc[0]/v1_ndc[3], )
				float w1,w2,w3; w1=v1_ndc[3]; w1=v2_ndc[3]; w1=v3_ndc[3];
				// std::cout << w1 << " " << w2 << " " << w3 << "\n";
				float z_1,z_2,z_3; z_1 = v1_ndc[2]+0.000001; z_2=v2_ndc[2]+0.000001; z_3=v3_ndc[2]+0.000001;
				v1_ndc = glm::vec4(v1_ndc[0]/v1_ndc[3], v1_ndc[1]/v1_ndc[3], v1_ndc[2]/v1_ndc[3], 1.0);
				v2_ndc = glm::vec4(v2_ndc[0]/v2_ndc[3], v2_ndc[1]/v2_ndc[3], v2_ndc[2]/v2_ndc[3], 1.0);
				v3_ndc = glm::vec4(v3_ndc[0]/v3_ndc[3], v3_ndc[1]/v3_ndc[3], v3_ndc[2]/v3_ndc[3], 1.0);

				// for z buffering
				float z1, z2, z3;
				z1 = v1_ndc[2];
				z2 = v2_ndc[2];
				z3 = v3_ndc[2];

				// std::cout<<"here1\n";
				glm::vec4 v1_col, v2_col, v3_col;
				v1_col = rasterizerProgram.fs(rasterizerProgram.uniforms, v1_out);
				v2_col = rasterizerProgram.fs(rasterizerProgram.uniforms, v2_out);
				v3_col = rasterizerProgram.fs(rasterizerProgram.uniforms, v3_out);

				float x1,y1,x2,y2,x3,y3;
				x1 = v1_ndc[0];
				y1 = v1_ndc[1];
				x2 = v2_ndc[0];
				y2 = v2_ndc[1];
				x3 = v3_ndc[0];
				y3 = v3_ndc[1];
				// std::cout << x1 << " " << y1 << " " << x1/v1_ndc[2] << " " << x2/v2_ndc[2] << "\n";

				// std::cout << v1_ndc[0] <<" "<<v1_ndc[1]<<" " <<v2_ndc[0]<<" "<<v1_ndc[1]<<" "<<x3<<" "<<y3<<"\n";
				float c_left_or_right = (-(y2-y1)*(x3-x1)+(x2-x1)*(y3-y1));
				for (int i = 0; i < width; i++) {
					for (int j = 0; j < height; j++) {
						// glm::vec4 = glm::cross(v1_ndc-v2_ndc,v3_ndc-v2_ndc);
						// float total = 0;
						// if zbuffer[i][j]


						float x = (i + 0.5 )/width;
						float y = (j + 0.5  )/height;
						x = 2*x-1;
						y = 1 - 2*y;
						float bary_1 = 0.;
						float bary_2 = 0.;
						float bary_3 = 0.;
						if ((c_left_or_right>=0 && (-(y2-y1)*(x-x1)+(x2-x1)*(y-y1)>=0 && -(y3-y2)*(x-x2)+(x3-x2)*(y-y2)>=0 && -(y1-y3)*(x-x3)+(x1-x3)*(y-y3)>=0)) || (c_left_or_right<0 && (-(y2-y1)*(x-x1)+(x2-x1)*(y-y1)<=0 && -(y3-y2)*(x-x2)+(x3-x2)*(y-y2)<=0 && -(y1-y3)*(x-x3)+(x1-x3)*(y-y3)<=0)))
						{
							for (int k=0; k<sqrt(supersampling_n); k++){
								for (int l=0; l<sqrt(supersampling_n); l++) {
									float xi = (i + 0.5 + (float)k/(float)sqrt(supersampling_n))/width;
									float yi = (j + 0.5 + (float)l/(float)sqrt(supersampling_n))/height;
									xi = 2*xi-1;
									yi = 1 - 2*yi;
									if ((c_left_or_right>=0 && (-(y2-y1)*(xi-x1)+(x2-x1)*(yi-y1)>=0 && -(y3-y2)*(xi-x2)+(x3-x2)*(yi-y2)>=0 && -(y1-y3)*(xi-x3)+(x1-x3)*(yi-y3)>=0)) || 
									(c_left_or_right<0 && (-(y2-y1)*(xi-x1)+(x2-x1)*(yi-y1)<=0 && -(y3-y2)*(xi-x2)+(x3-x2)*(yi-y2)<=0 && -(y1-y3)*(xi-x3)+(x1-x3)*(yi-y3)<=0))){
									// std::cout << "X Y " << xi << " " << yi << "\n";
									// float total = glm::length(glm::cross(glm::vec3(v1_ndc[0]/v1_ndc[2]-v2_ndc[0]/v2_ndc[2], v1_ndc[1]/v1_ndc[2]-v2_ndc[1]/v2_ndc[2],0), glm::vec3(v3_ndc[0]/v3_ndc[2]-v2_ndc[0]/v2_ndc[2], v3_ndc[1]/v3_ndc[2]-v2_ndc[1]/v2_ndc[2],0)));
									float total = glm::length(glm::cross(glm::vec3(v1_ndc[0]-v2_ndc[0], v1_ndc[1]-v2_ndc[1],0), glm::vec3(v3_ndc[0]-v2_ndc[0], v3_ndc[1]-v2_ndc[1],0)));
									bary_1 += glm::length(glm::cross(glm::vec3(v2_ndc[0]-xi, v2_ndc[1]-yi,0), glm::vec3(v3_ndc[0]-xi, v3_ndc[1]-yi,0)))/total;
									bary_2 += glm::length(glm::cross(glm::vec3(v3_ndc[0]-xi, v3_ndc[1]-yi,0), glm::vec3(v1_ndc[0]-xi, v1_ndc[1]-yi,0)))/total;
									bary_3 += glm::length(glm::cross(glm::vec3(v1_ndc[0]-xi, v1_ndc[1]-yi,0), glm::vec3(v2_ndc[0]-xi, v2_ndc[1]-yi,0)))/total;
									// bary_1 += glm::length(glm::cross(glm::vec3(v2_ndc[0]/v2_ndc[2]-xi, v2_ndc[1]/v2_ndc[2]-yi,0), glm::vec3(v3_ndc[0]/v3_ndc[2]-xi, v3_ndc[1]/v3_ndc[2]-yi,0)))/total;
									// bary_2 += glm::length(glm::cross(glm::vec3(v3_ndc[0]/v3_ndc[2]-xi, v3_ndc[1]/v3_ndc[2]-yi,0), glm::vec3(v1_ndc[0]/v1_ndc[2]-xi, v1_ndc[1]/v1_ndc[2]-yi,0)))/total;
									// bary_3 += glm::length(glm::cross(glm::vec3(v1_ndc[0]/v1_ndc[2]-xi, v1_ndc[1]/v1_ndc[2]-yi,0), glm::vec3(v2_ndc[0]/v2_ndc[2]-xi, v2_ndc[1]/v2_ndc[2]-yi,0)))/total;
								// float total = glm::length(glm::cross(glm::vec3(v1_ndc[0]-v2_ndc[0], v1_ndc[1]-v2_ndc[1],0), glm::vec3(v3_ndc[0]-v2_ndc[0], v3_ndc[1]-v2_ndc[1],0)));
								// bary_1 = bary_1 + glm::length(glm::cross(glm::vec3(v2_ndc[0]-x, v2_ndc[1]-y,0), glm::vec3(v3_ndc[0]-x, v3_ndc[1]-y,0)))/total;
								// bary_2 = bary_2 + glm::length(glm::cross(glm::vec3(v3_ndc[0]-x, v3_ndc[1]-y,0), glm::vec3(v1_ndc[0]-x, v1_ndc[1]-y,0)))/total;
								// bary_3 = bary_3 + glm::length(glm::cross(glm::vec3(v1_ndc[0]-x, v1_ndc[1]-y,0), glm::vec3(v2_ndc[0]-x, v2_ndc[1]-y,0)))/total;
									}
								}
							}
							bary_1 = bary_1/(float)supersampling_n;
							bary_2 = bary_2/(float)supersampling_n;
							bary_3 = bary_3/(float)supersampling_n;
							if (bary_1==0.0 || bary_2==0.0 || bary_3==0.0){bary_1=1.0; bary_2=0.0; bary_3=0.0;}
							// std::cout << "Barry : " << bary_1 << " " << bary_2 << " "<< bary_3 << "\n";
							// float z_curr = 10000/(bary_1/z_1 + bary_2/z_2 + bary_3/z_3);
							float z_curr = (bary_1*z1 + bary_2*z2 + bary_3*z3);
							// std::cout << "z value " << z_1 << " " << z_2 << " " << z_3 << "\n";
							if (zbuffering){
								// std::cout << "zbuff\n";
								if (z_curr<=zbuffer[i][j]) {
									zbuffer[i][j] = z_curr;
									// std::cout<< ((bary_1*v1_col[0]/z1 + bary_2*v2_col[0]/z2 + bary_3*v3_col[0]/z3)/(bary_1/z1 + bary_2/z2 + bary_3/z3))*255<<" "<< ((bary_1*v1_col[1]/z1 + bary_2*v2_col[1]/z2 + bary_3*v3_col[1]/z3)/(bary_1/z1 + bary_2/z2 + bary_3/z3))*255<<" "<< ((bary_1*v1_col[2]/z1 + bary_2*v2_col[2]/z2 + bary_3*v3_col[2]/z3)/(bary_1/z1 + bary_2/z2 + bary_3/z3))*255<<" "<< ((bary_1*v1_col[3]/z1 + bary_2*v2_col[3]/z2 + bary_3*v3_col[3]/z3)/(bary_1/z1 + bary_2/z2 + bary_3/z3))*255 << "\n";
									// std::cout<< ((bary_1*v1_col[0] + bary_2*v2_col[0] + bary_3*v3_col[0])/(bary_1 + bary_2 + bary_3))*255 << " " <<  ((bary_1*v1_col[1] + bary_2*v2_col[1] + bary_3*v3_col[1])/(bary_1 + bary_2 + bary_3))*255 << " " << ((bary_1*v1_col[2] + bary_2*v2_col[2] + bary_3*v3_col[2])/(bary_1 + bary_2 + bary_3))*255 << " " << ((bary_1*v1_col[3] + bary_2*v2_col[3] + bary_3*v3_col[3])/(bary_1 + bary_2 + bary_3))*255 << "\n\n";
									// pixels[i + width*j] = SDL_MapRGBA(format, ((bary_1*v1_col[0] + bary_2*v2_col[0] + bary_3*v3_col[0])/(bary_1 + bary_2 + bary_3))*255, ((bary_1*v1_col[1] + bary_2*v2_col[1] + bary_3*v3_col[1])/(bary_1 + bary_2 + bary_3))*255, ((bary_1*v1_col[2] + bary_2*v2_col[2] + bary_3*v3_col[2])/(bary_1 + bary_2 + bary_3))*255, ((bary_1*v1_col[3] + bary_2*v2_col[3] + bary_3*v3_col[3])/(bary_1 + bary_2 + bary_3))*255);
									// pixels[i + width*j] = SDL_MapRGBA(format, ((bary_1*v1_col[0]/z1 + bary_2*v2_col[0]/z2 + bary_3*v3_col[0]/z3)/(bary_1/z1 + bary_2/z2 + bary_3/z3))*255, ((bary_1*v1_col[1]/z1 + bary_2*v2_col[1]/z2 + bary_3*v3_col[1]/z3)/(bary_1/z1 + bary_2/z2 + bary_3/z3))*255, ((bary_1*v1_col[2]/z1 + bary_2*v2_col[2]/z2 + bary_3*v3_col[2]/z3)/(bary_1/z1 + bary_2/z2 + bary_3/z3))*255, ((bary_1*v1_col[3]/z1 + bary_2*v2_col[3]/z2 + bary_3*v3_col[3]/z3)/(bary_1/z1 + bary_2/z2 + bary_3/z3))*255);
									pixels[i + width*j] = SDL_MapRGBA(format, ((bary_1*v1_col[0]/z_1 + bary_2*v2_col[0]/z_2 + bary_3*v3_col[0]/z_3)/(bary_1/z_1 + bary_2/z_2 + bary_3/z_3))*255, ((bary_1*v1_col[1]/z_1 + bary_2*v2_col[1]/z_2 + bary_3*v3_col[1]/z_3)/(bary_1/z_1 + bary_2/z_2 + bary_3/z_3))*255, ((bary_1*v1_col[2]/z_1 + bary_2*v2_col[2]/z_2 + bary_3*v3_col[2]/z_3)/(bary_1/z_1 + bary_2/z_2 + bary_3/z_3))*255, ((bary_1*v1_col[3]/z_1 + bary_2*v2_col[3]/z_2 + bary_3*v3_col[3]/z_3)/(bary_1/z_1 + bary_2/z_2 + bary_3/z_3))*255);
									// pixels[i + width*j] = SDL_MapRGBA(format, ((bary_1*v1_col[0]/w1 + bary_2*v2_col[0]/w2 + bary_3*v3_col[0]/w3)/(bary_1/w1 + bary_2/w2 + bary_3/w3))*255, ((bary_1*v1_col[1]/w1 + bary_2*v2_col[1]/w2 + bary_3*v3_col[1]/w3)/(bary_1/w1 + bary_2/w2 + bary_3/w3))*255, ((bary_1*v1_col[2]/w1 + bary_2*v2_col[2]/w2 + bary_3*v3_col[2]/w3)/(bary_1/w1 + bary_2/w2 + bary_3/w3))*255, ((bary_1*v1_col[3]/w1 + bary_2*v2_col[3]/w2 + bary_3*v3_col[3]/w3)/(bary_1/w1 + bary_2/w2 + bary_3/w3))*255);
									// pixels[i + width*j] = SDL_MapRGBA(format, ((bary_1*v1_col[0]*z1 + bary_2*v2_col[0]*z2 + bary_3*v3_col[0]*z3)/(bary_1*z1 + bary_2*z2 + bary_3*z3))*255, ((bary_1*v1_col[1]*z1 + bary_2*v2_col[1]*z2 + bary_3*v3_col[1]*z3)/(bary_1*z1 + bary_2*z2 + bary_3*z3))*255, ((bary_1*v1_col[2]*z1 + bary_2*v2_col[2]*z2 + bary_3*v3_col[2]*z3)/(bary_1*z1 + bary_2*z2 + bary_3*z3))*255, ((bary_1*v1_col[3]*z1 + bary_2*v2_col[3]*z2 + bary_3*v3_col[3]*z3)/(bary_1*z1 + bary_2*z2 + bary_3*z3))*255);

								}
								else {continue;}
							}
							else {
								// std::cout<< ((bary_1*v1_col[0]/z1 + bary_2*v2_col[0]/z2 + bary_3*v3_col[0]/z3)/(bary_1/z1 + bary_2/z2 + bary_3/z3))*255<<" "<< ((bary_1*v1_col[1]/z1 + bary_2*v2_col[1]/z2 + bary_3*v3_col[1]/z3)/(bary_1/z1 + bary_2/z2 + bary_3/z3))*255<<" "<< ((bary_1*v1_col[2]/z1 + bary_2*v2_col[2]/z2 + bary_3*v3_col[2]/z3)/(bary_1/z1 + bary_2/z2 + bary_3/z3))*255<<" "<< ((bary_1*v1_col[3]/z1 + bary_2*v2_col[3]/z2 + bary_3*v3_col[3]/z3)/(bary_1/z1 + bary_2/z2 + bary_3/z3))*255 << "\n";
								// pixels[i + width*j] = SDL_MapRGBA(format, ((bary_1*v1_col[0]/z1 + bary_2*v2_col[0]/z2 + bary_3*v3_col[0]/z3)/(bary_1/z1 + bary_2/z2 + bary_3/z3))*255, ((bary_1*v1_col[1]/z1 + bary_2*v2_col[1]/z2 + bary_3*v3_col[1]/z3)/(bary_1/z1 + bary_2/z2 + bary_3/z3))*255, ((bary_1*v1_col[2]/z1 + bary_2*v2_col[2]/z2 + bary_3*v3_col[2]/z3)/(bary_1/z1 + bary_2/z2 + bary_3/z3))*255, ((bary_1*v1_col[3]/z1 + bary_2*v2_col[3]/z2 + bary_3*v3_col[3]/z3)/(bary_1/z1 + bary_2/z2 + bary_3/z3))*255);
								pixels[i + width*j] = SDL_MapRGBA(format, ((bary_1*v1_col[0] + bary_2*v2_col[0] + bary_3*v3_col[0])/(bary_1 + bary_2 + bary_3))*255, ((bary_1*v1_col[1] + bary_2*v2_col[1] + bary_3*v3_col[1])/(bary_1 + bary_2 + bary_3))*255, ((bary_1*v1_col[2] + bary_2*v2_col[2] + bary_3*v3_col[2])/(bary_1 + bary_2 + bary_3))*255, ((bary_1*v1_col[3] + bary_2*v2_col[3] + bary_3*v3_col[3])/(bary_1 + bary_2 + bary_3))*255);
							}
							

						}
						
						// std::cout << "Barry : " << bary_1 << " " << bary_2 << " "<< bary_3 << "\n";
						// float x = (i + 0.5)/width;  // [0, w] -> [0, 1]
						// x = 2*x - 1;                     // [0, 1] -> [-1, 1]
						// float y = (j + 0.5)/height; // [0, h] -> [0, 1]
						// y = 1 - 2*y;                     // [0, 1] -> [1, -1]
						// // Uint32 colorij = SDL_MapRGBA(format, v1_col[0]*255, v1_col[1]*255, v1_col[2]*255, v1_col[3]*255); 
						// if (c_left_or_right>=0) {
						// 	if (-(y2-y1)*(x-x1)+(x2-x1)*(y-y1)>=0 && -(y3-y2)*(x-x2)+(x3-x2)*(y-y2)>=0 && -(y1-y3)*(x-x3)+(x1-x3)*(y-y3)>=0){
						// 		float total = glm::length(glm::cross(glm::vec3(v1_ndc[0]-v2_ndc[0], v1_ndc[1]-v2_ndc[1],0), glm::vec3(v3_ndc[0]-v2_ndc[0], v3_ndc[1]-v2_ndc[1],0)));
						// 		float bary_1 = glm::length(glm::cross(glm::vec3(v2_ndc[0]-x, v2_ndc[1]-y,0), glm::vec3(v3_ndc[0]-x, v3_ndc[1]-y,0)))/total;
						// 		float bary_2 = glm::length(glm::cross(glm::vec3(v3_ndc[0]-x, v3_ndc[1]-y,0), glm::vec3(v1_ndc[0]-x, v1_ndc[1]-y,0)))/total;
						// 		float bary_3 = glm::length(glm::cross(glm::vec3(v1_ndc[0]-x, v1_ndc[1]-y,0), glm::vec3(v2_ndc[0]-x, v2_ndc[1]-y,0)))/total;
						// 		// std::cout << "BARRRYY : " << bary_1+bary_2+bary_3 << "\n";	
						// 		pixels[i + width*j] = SDL_MapRGBA(format, (bary_1*v1_col[0] + bary_2*v2_col[0] + bary_3*v3_col[0])*255, (bary_1*v1_col[1] + bary_2*v2_col[1] + bary_3*v3_col[1])*255, (bary_1*v1_col[2] + bary_2*v2_col[2] + bary_3*v3_col[2])*255, (bary_1*v1_col[3] + bary_2*v2_col[3] + bary_3*v3_col[3])*255);
						// 	} 
						// }
						// else {
						// 	if (-(y2-y1)*(x-x1)+(x2-x1)*(y-y1)<=0 && -(y3-y2)*(x-x2)+(x3-x2)*(y-y2)<=0 && -(y1-y3)*(x-x3)+(x1-x3)*(y-y3)<=0){
						// 		float total = glm::length(glm::cross(glm::vec3(v1_ndc[0]-v2_ndc[0], v1_ndc[1]-v2_ndc[1],0), glm::vec3(v3_ndc[0]-v2_ndc[0], v3_ndc[1]-v2_ndc[1],0)));
						// 		float bary_1 = glm::length(glm::cross(glm::vec3(v2_ndc[0]-x, v2_ndc[1]-y,0), glm::vec3(v3_ndc[0]-x, v3_ndc[1]-y,0)))/total;
						// 		float bary_2 = glm::length(glm::cross(glm::vec3(v3_ndc[0]-x, v3_ndc[1]-y,0), glm::vec3(v1_ndc[0]-x, v1_ndc[1]-y,0)))/total;
						// 		float bary_3 = glm::length(glm::cross(glm::vec3(v1_ndc[0]-x, v1_ndc[1]-y,0), glm::vec3(v2_ndc[0]-x, v2_ndc[1]-y,0)))/total;
						// 		// std::cout << "BARRRYY : " << bary_1+bary_2+bary_3 << "\n";	
						// 		pixels[i + width*j] = SDL_MapRGBA(format, (bary_1*v1_col[0] + bary_2*v2_col[0] + bary_3*v3_col[0])*255, (bary_1*v1_col[1] + bary_2*v2_col[1] + bary_3*v3_col[1])*255, (bary_1*v1_col[2] + bary_2*v2_col[2] + bary_3*v3_col[2])*255, (bary_1*v1_col[3] + bary_2*v2_col[3] + bary_3*v3_col[3])*255);
						// 	} 
						// }
						
					}
				}
			}
			// SDL_BlitScaled(framebuffer, NULL, windowSurface, NULL);
			// SDL_UpdateWindowSurface(window);
			return;
		}

		void Rasterizer::show(){
			SDL_Surface* windowSurface = SDL_GetWindowSurface(window);
			SDL_BlitScaled(framebuffer, NULL, windowSurface, NULL);
            SDL_UpdateWindowSurface(window);

			SDL_Event e;
			// when we click (x) on the window, that is a poll event
    		while (SDL_PollEvent(&e) != 0) {
				if (e.type == SDL_QUIT) {
				quit = true;
				}
			}
		}

		// Deletes the given shader program.
		void Rasterizer::deleteShaderProgram(ShaderProgram &program) {
			// std::cout << "Deleted!\n";
			program.fs=NULL;
			program.vs=NULL;
			program.uniforms=Uniforms();
						// std::cout << "Deleted1!\n";
			return;
		}

		// Enable depth testing.
		void Rasterizer::enableDepthTest(){
			int width, height;
    		SDL_GetWindowSize(window, &width, &height);
			// std::cout << "width height" << width << " " << height << "\n";
			for (int i=0; i<width; i++){
				std::vector<float> v;
				for (int j=0; j<height; j++){
					v.push_back(FLT_MAX);
					// std::cout << v[j] << "\n";
				}
				zbuffer.push_back(v);
			}
			zbuffering = true;
			// std::cout << "enabled\n";
		}
	}
}
