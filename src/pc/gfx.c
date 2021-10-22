/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "../gfx.h"

#include "../main.h"
#include "../mem.h"

#define PSXF_GL_MODERN 0
#define PSXF_GL_LEGACY 1
#define PSXF_GL_ES 2

#if PSXF_GL == PSXF_GL_ES
 #include <GLES2/gl2.h>
#else
 #include "glad/glad.h"
#endif
#include <GLFW/glfw3.h>

#include "cglm/cglm.h"

//Gfx constants and shaders
#define WINDOW_SCALE 3
#define WINDOW_WIDTH  (SCREEN_WIDTH * WINDOW_SCALE)
#define WINDOW_HEIGHT (SCREEN_HEIGHT * WINDOW_SCALE)
//The PSX's VRAM can be thought of as a 1024x512 16bpp texture, but 4bpp
//allows cramming 4 times the pixels in that space.
//That said, 4096x512 is a little too large for some low-end platforms
//(particularly older Raspberry Pis), so let's do some trickery to fit
//it into 2048x1024 instead.
#define VRAM_WIDTH 2048
#define VRAM_HEIGHT 1024

//Window
GLFWwindow *window;

//Render state
static mat4 projection;

static u8 clear_r, clear_g, clear_b;
static boolean clear_e;

//Display list
typedef struct
{
	struct
	{
		float left, top, right, bottom;
	} src;
	struct
	{
		struct { float x, y; } tl, tr, bl, br;
	} dst;
	float r, g, b;
	GLuint texture_id;
	u8 blend_mode;
} Gfx_Cmd;

static Gfx_Cmd dlist[0x400];
static Gfx_Cmd *dlist_p;

typedef struct
{
	float x, y;
	float u, v;
	float r, g, b, a;
} Gfx_Vertex;

//Shader
#if PSXF_GL == PSXF_GL_MODERN
//GLSL Core 1.50, for OpenGL Core 3.2
static const char *generic_shader_vert = "\
#version 150 core\n\
in vec2 v_position;\
in vec2 v_uv;\
in vec4 v_colour;\
out vec2 f_uv;\
out vec4 f_colour;\
uniform mat4 u_projection;\
void main()\
{\
f_uv = v_uv;\
f_colour = v_colour;\
gl_Position = u_projection * vec4(v_position.xy, 0.0, 1.0);\
}";
static const char *generic_shader_frag = "\
#version 150 core\n\
uniform sampler2D u_texture;\
in vec2 f_uv;\
in vec4 f_colour;\
out vec4 o_colour;\
void main()\
{\
o_colour = texture(u_texture, f_uv) * f_colour;\
if (o_colour.a == 0.0)\
{\
discard;\
}\
}";
#elif PSXF_GL == PSXF_GL_LEGACY
//GLSL 1.20, for OpenGL 2.1
static const char *generic_shader_vert = "\
#version 120\n\
attribute vec2 v_position;\
attribute vec2 v_uv;\
attribute vec4 v_colour;\
varying vec2 f_uv;\
varying vec4 f_colour;\
uniform mat4 u_projection;\
void main()\
{\
f_uv = v_uv;\
f_colour = v_colour;\
gl_Position = u_projection * vec4(v_position.xy, 0.0, 1.0);\
}";
static const char *generic_shader_frag = "\
#version 120\n\
uniform sampler2D u_texture;\
varying vec2 f_uv;\
varying vec4 f_colour;\
void main()\
{\
gl_FragColor = texture2D(u_texture, f_uv) * f_colour;\
if (gl_FragColor.a == 0.0)\
{\
discard;\
}\
}";
#elif PSXF_GL == PSXF_GL_ES
//GLSL ES 1.0, for OpenGL ES 2.0
static const char *generic_shader_vert = "\
#version 100\n\
precision highp float;\
attribute vec2 v_position;\
attribute vec2 v_uv;\
attribute vec4 v_colour;\
varying vec2 f_uv;\
varying vec4 f_colour;\
uniform mat4 u_projection;\
void main()\
{\
f_uv = v_uv;\
f_colour = v_colour;\
gl_Position = u_projection * vec4(v_position.xy, 0.0, 1.0);\
}";
static const char *generic_shader_frag = "\
#version 100\n\
precision highp float;\
uniform sampler2D u_texture;\
varying vec2 f_uv;\
varying vec4 f_colour;\
void main()\
{\
gl_FragColor = texture2D(u_texture, f_uv) * f_colour;\
if (gl_FragColor.a == 0.0)\
{\
discard;\
}\
}";
#endif

typedef struct
{
	GLuint program, vertex, fragment;
} Gfx_Shader;

static Gfx_Shader generic_shader;

//Textures
static GLuint plain_texture;
static GLuint vram_texture;

//Batch
#if PSXF_GL == PSXF_GL_MODERN
static GLuint batch_vao;
#endif
static GLuint batch_vbo;

static GLuint batch_texture_id;

static Gfx_Vertex batch_buffer[COUNT_OF(dlist)][6]; //TODO: index buffer
static Gfx_Vertex *batch_buffer_p;

//Internal gfx functions
static void Gfx_FramebufferSizeCallback(GLFWwindow *window, int fb_width, int fb_height)
{
	(void)window;
	
	//Center the viewport within the window while maintaining the aspect ratio
	GLfloat viewport_width, viewport_height;
	if ((float)fb_width / (float)fb_height > (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)
	{
		viewport_width = fb_height * SCREEN_WIDTH / SCREEN_HEIGHT;
		viewport_height = fb_height;
	}
	else
	{
		viewport_width = fb_width;
		viewport_height = fb_width * SCREEN_HEIGHT / SCREEN_WIDTH;
	}

	glViewport((fb_width - viewport_width) / 2, (fb_height - viewport_height) / 2, viewport_width, viewport_height);
}

static void Gfx_CompileShader(Gfx_Shader *this, const char *src_vert, const char *src_frag)
{
	//Create shader
	GLint shader_status;
	this->program = glCreateProgram();
	
	//Compile vertex shader
	this->vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(this->vertex, 1, &src_vert, NULL);
	glCompileShader(this->vertex);
	
	glGetShaderiv(this->vertex, GL_COMPILE_STATUS, &shader_status);
	if (shader_status != GL_TRUE)
	{
		char buffer[0x200];
		glGetShaderInfoLog(this->vertex, sizeof(buffer), NULL, buffer);
		sprintf(error_msg, "[Gfx_CompileShader] Failed to compile vertex shader: %s", buffer);
		ErrorLock();
	}
	
	//Compile fragment shader
	this->fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(this->fragment, 1, &src_frag, NULL);
	glCompileShader(this->fragment);
	
	glGetShaderiv(this->fragment, GL_COMPILE_STATUS, &shader_status);
	if (shader_status != GL_TRUE)
	{
		char buffer[0x200];
		glGetShaderInfoLog(this->fragment, sizeof(buffer), NULL, buffer);
		sprintf(error_msg, "[Gfx_CompileShader] Failed to compile fragment shader: %s", buffer);
		ErrorLock();
	}
	
	//Attach and link
	glAttachShader(this->program, this->vertex);
	glAttachShader(this->program, this->fragment);
	
	glBindAttribLocation(this->program, 0, "v_position");
	glBindAttribLocation(this->program, 1, "v_uv");
	glBindAttribLocation(this->program, 2, "v_colour");
	
	glLinkProgram(this->program);
	
	glGetProgramiv(this->program, GL_LINK_STATUS, &shader_status);
	if (shader_status != GL_TRUE)
	{
		char buffer[0x200];
		glGetProgramInfoLog(this->program, sizeof(buffer), NULL, buffer);
		sprintf(error_msg, "[Gfx_CompileShader] Failed to link shader: %s", buffer);
		ErrorLock();
	}
	
	glDetachShader(this->program, this->vertex);
	glDetachShader(this->program, this->fragment);
}

static void Gfx_DeleteShader(Gfx_Shader *this)
{
	glDeleteProgram(this->program);
	glDeleteShader(this->vertex);
	glDeleteShader(this->fragment);
}

static GLuint Gfx_CreateTexture(GLint width, GLint height)
{
	//Create texture object
	GLuint texture_id;
	glGenTextures(1, &texture_id);
	
	//Set texture parameters
	glBindTexture(GL_TEXTURE_2D, texture_id);
#if PSXF_GL == PSXF_GL_ES
	//OpenGL ES 2.0 does not support RGBA5551, so settle for RGBA8888 instead.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, NULL);
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if PSXF_GL != PSXF_GL_ES
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
#endif
	
	return texture_id;
}

static void Gfx_UploadTexture(GLuint texture_id, GLint x, GLint y, const u8 *data, GLint width, GLint height)
{
	//Upload data to texture
	glBindTexture(GL_TEXTURE_2D, texture_id);
#if PSXF_GL == PSXF_GL_ES
	//OpenGL ES 2.0 does not support RGBA5551, so settle for RGBA8888 instead.
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (const void*)data);
#else
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, (const void*)data);
#endif
}

static void Gfx_PushBatch(void)
{
	//Drop if we haven't batched any data
	if (batch_buffer_p == &batch_buffer[0][0])
		return;
	
	//Bind VAO and VBO
#if PSXF_GL == PSXF_GL_MODERN
	//Only modern OpenGL has VAOs
	glBindVertexArray(batch_vao);
#endif
	glBindBuffer(GL_ARRAY_BUFFER, batch_vbo);
	
	//Set attribute pointers
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Gfx_Vertex), (GLvoid*)offsetof(Gfx_Vertex, x));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Gfx_Vertex), (GLvoid*)offsetof(Gfx_Vertex, u));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Gfx_Vertex), (GLvoid*)offsetof(Gfx_Vertex, r));
	
	//Send data to VBO
	glBufferData(GL_ARRAY_BUFFER, (batch_buffer_p - &batch_buffer[0][0]) * sizeof(Gfx_Vertex), (const void*)(&batch_buffer[0][0]), GL_STATIC_DRAW);
	
	//Display data
	glDrawArrays(GL_TRIANGLES, 0, batch_buffer_p - &batch_buffer[0][0]);

	batch_buffer_p = &batch_buffer[0][0];
}

static void Gfx_DisplayCmd(const Gfx_Cmd *cmd)
{
	static u8 previous_blend_mode = 0xFE; //A sane invalid value

	//Push batch if using a new blend mode
	float alpha = 1.0f;
	
	if (cmd->blend_mode != previous_blend_mode)
	{
		Gfx_PushBatch();
		previous_blend_mode = cmd->blend_mode;

		switch (cmd->blend_mode)
		{
			//0     - 50% background  +50% polygon
			//1     - 100% background +100% polygon
			//2     - 100% background - 100% polygon
			//3     - 100% background + 25% polygon

			case 0xFF:
				//No blending
				glDisable(GL_BLEND);
				break;

			case 0:
				//Mix blending
				glEnable(GL_BLEND);
				glBlendEquation(GL_FUNC_ADD);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				alpha = 0.5f;
				break;

			case 1:
				//Additive blending
				glEnable(GL_BLEND);
				glBlendEquation(GL_FUNC_ADD);
				glBlendFunc(GL_ONE, GL_ONE);
				break;

			case 2:
				//Subtractive blending
				glEnable(GL_BLEND);
				glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
				glBlendFunc(GL_ONE, GL_ONE);
				break;

			case 3:
				//Additive blending
				glEnable(GL_BLEND);
				glBlendEquation(GL_FUNC_ADD);
				glBlendFunc(GL_ONE, GL_ONE);
				alpha = 0.25f;
				break;

			default:
				sprintf(error_msg, "[Gfx_BlendRect] Blend mode %d is unimplemented", cmd->blend_mode);
				ErrorLock();
				break;
		}
	}

	//Push batch and bind if new texture
	if (cmd->texture_id != batch_texture_id)
	{
		Gfx_PushBatch();
		glBindTexture(GL_TEXTURE_2D, batch_texture_id = cmd->texture_id);
	}
	
	//Push data to buffer
	batch_buffer_p[0].x = cmd->dst.tl.x;
	batch_buffer_p[0].y = cmd->dst.tl.y;
	batch_buffer_p[0].u = cmd->src.left;
	batch_buffer_p[0].v = cmd->src.top;
	batch_buffer_p[0].r = cmd->r;
	batch_buffer_p[0].g = cmd->g;
	batch_buffer_p[0].b = cmd->b;
	batch_buffer_p[0].a = alpha;
	
	batch_buffer_p[1].x = cmd->dst.bl.x;
	batch_buffer_p[1].y = cmd->dst.bl.y;
	batch_buffer_p[1].u = cmd->src.left;
	batch_buffer_p[1].v = cmd->src.bottom;
	batch_buffer_p[1].r = cmd->r;
	batch_buffer_p[1].g = cmd->g;
	batch_buffer_p[1].b = cmd->b;
	batch_buffer_p[1].a = alpha;
	
	batch_buffer_p[2].x = cmd->dst.tr.x;
	batch_buffer_p[2].y = cmd->dst.tr.y;
	batch_buffer_p[2].u = cmd->src.right;
	batch_buffer_p[2].v = cmd->src.top;
	batch_buffer_p[2].r = cmd->r;
	batch_buffer_p[2].g = cmd->g;
	batch_buffer_p[2].b = cmd->b;
	batch_buffer_p[2].a = alpha;
	
	batch_buffer_p[3].x = cmd->dst.tr.x;
	batch_buffer_p[3].y = cmd->dst.tr.y;
	batch_buffer_p[3].u = cmd->src.right;
	batch_buffer_p[3].v = cmd->src.top;
	batch_buffer_p[3].r = cmd->r;
	batch_buffer_p[3].g = cmd->g;
	batch_buffer_p[3].b = cmd->b;
	batch_buffer_p[3].a = alpha;
	
	batch_buffer_p[4].x = cmd->dst.bl.x;
	batch_buffer_p[4].y = cmd->dst.bl.y;
	batch_buffer_p[4].u = cmd->src.left;
	batch_buffer_p[4].v = cmd->src.bottom;
	batch_buffer_p[4].r = cmd->r;
	batch_buffer_p[4].g = cmd->g;
	batch_buffer_p[4].b = cmd->b;
	batch_buffer_p[4].a = alpha;
	
	batch_buffer_p[5].x = cmd->dst.br.x;
	batch_buffer_p[5].y = cmd->dst.br.y;
	batch_buffer_p[5].u = cmd->src.right;
	batch_buffer_p[5].v = cmd->src.bottom;
	batch_buffer_p[5].r = cmd->r;
	batch_buffer_p[5].g = cmd->g;
	batch_buffer_p[5].b = cmd->b;
	batch_buffer_p[5].a = alpha;
	
	batch_buffer_p += 6;
}

static void Gfx_SubmitCommand(GLuint texture_id, const RECT *src, const POINT *p0, const POINT *p1, const POINT *p2, const POINT *p3, float r, float g, float b, u8 blend_mode)
{
	//Create new command
	Gfx_Cmd cmd;
	cmd.src.left =   src->x / (float)VRAM_WIDTH;
	cmd.src.top =    src->y / (float)VRAM_HEIGHT;
	cmd.src.right =  (src->x + src->w) / (float)VRAM_WIDTH;
	cmd.src.bottom = (src->y + src->h) / (float)VRAM_HEIGHT;
	cmd.dst.tl.x = (float)p0->x;
	cmd.dst.tl.y = (float)p0->y;
	cmd.dst.tr.x = (float)p1->x;
	cmd.dst.tr.y = (float)p1->y;
	cmd.dst.bl.x = (float)p2->x;
	cmd.dst.bl.y = (float)p2->y;
	cmd.dst.br.x = (float)p3->x;
	cmd.dst.br.y = (float)p3->y;
	cmd.r = r;
	cmd.g = g;
	cmd.b = b;
	cmd.texture_id = texture_id;
	cmd.blend_mode = blend_mode;

	//Push command
	*dlist_p++ = cmd;
}

//Gfx functions
void Gfx_Init(void)
{
	//Set window hints
#if PSXF_GL == PSXF_GL_MODERN
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
#elif PSXF_GL == PSXF_GL_LEGACY
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#elif PSXF_GL == PSXF_GL_ES
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	
	//Get monitor video mode
	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
	
	const GLFWvidmode *mode;
	if (monitor != NULL)
		mode = glfwGetVideoMode(monitor);
	else
		mode = NULL;
	
	//Create window
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "PSXFunkin", NULL, NULL);
	if (!window)
	{
		sprintf(error_msg, "[Gfx_Init] Failed to create GLFW window");
		ErrorLock();
	}
	glfwMakeContextCurrent(window);
	
	//Center window
	if (mode != NULL)
		glfwSetWindowPos(window, (mode->width - WINDOW_WIDTH) / 2, (mode->height - WINDOW_HEIGHT) / 2);
	glfwShowWindow(window);
	
	//Define callback for window resizing
	glfwSetFramebufferSizeCallback(window, Gfx_FramebufferSizeCallback);
	
	//Enable vsync
	if (glfwExtensionSupported("GLX_EXT_swap_control_tear") || glfwExtensionSupported("WGL_EXT_swap_control_tear"))
		glfwSwapInterval(-1);
	else
		glfwSwapInterval(1);

#if PSXF_GL != PSXF_GL_ES
	//Initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		sprintf(error_msg, "[Gfx_Init] Failed to initialize GLAD");
		ErrorLock();
	}
	
#if PSXF_GL == PSXF_GL_MODERN
	if (!GLAD_GL_VERSION_3_2)
	{
		sprintf(error_msg, "[Gfx_Init] OpenGL 3.2 is not supported");
		ErrorLock();
	}
#elif PSXF_GL == PSXF_GL_LEGACY
	if (!GLAD_GL_VERSION_2_1)
	{
		sprintf(error_msg, "[Gfx_Init] OpenGL 2.1 is not supported");
		ErrorLock();
	}
#endif
#endif
	
	//Initialize render state
	glm_ortho(0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f, -1.0f, 1.0f, projection);
	
	clear_r = 0;
	clear_g = 0;
	clear_b = 0;
	clear_e = true;
	
	//Initialize OpenGL state
	//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	
	//Create shaders
	Gfx_CompileShader(&generic_shader, generic_shader_vert, generic_shader_frag);
	glUseProgram(generic_shader.program);
	glUniformMatrix4fv(glGetUniformLocation(generic_shader.program, "u_projection"), 1, GL_FALSE, &projection[0][0]);
	
	//Create textures
#if PSXF_GL == PSXF_GL_ES
	static const u8 plain_texture_data[] = {0xFF, 0xFF, 0xFF, 0xFF}; //RGBA8888
#else
	static const u8 plain_texture_data[] = {0xFF, 0xFF}; //RGBA5551
#endif
	Gfx_UploadTexture(plain_texture = Gfx_CreateTexture(1, 1), 0, 0, plain_texture_data, 1, 1);
	
	vram_texture = Gfx_CreateTexture(VRAM_WIDTH, VRAM_HEIGHT);
	
	//Create batch VAO
#if PSXF_GL == PSXF_GL_MODERN
	//Only modern OpenGL has VAOs
	glGenVertexArrays(1, &batch_vao);
	glBindVertexArray(batch_vao);
#endif
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	//Create batch VBO
	glGenBuffers(1, &batch_vbo);
	
	//Initialize frame
	dlist_p = dlist;
	batch_buffer_p = &batch_buffer[0][0];
	batch_texture_id = 0;
}

void Gfx_Quit(void)
{
	//Delete GL objects
	glDeleteBuffers(1, &batch_vbo);
#if PSXF_GL == PSXF_GL_MODERN
	glDeleteVertexArrays(1, &batch_vao);
#endif
	glDeleteTextures(1, &plain_texture);
	glDeleteTextures(1, &vram_texture);
	Gfx_DeleteShader(&generic_shader);
	
	//Destroy window
	glfwDestroyWindow(window);
}

void Gfx_Flip(void)
{
	//FPS counter
	static int fps_i = 0;
	static double fps_t = 0.0;
	
	if (glfwGetTime() >= fps_t)
	{
		char buf[0x80];
		sprintf(buf, "PSXFunkin [%d fps]", fps_i);
		glfwSetWindowTitle(window, buf);
		fps_t = glfwGetTime() + 1.0;
		fps_i = 0;
	}
	fps_i++;
	
	//Clear screen
	if (clear_e)
	{
		glClear(GL_COLOR_BUFFER_BIT);

		//Draw the background color (we don't do this with glClearColor
		//or else we'd end up drawing in the black bars around the screen)
		RECT rect;
		rect.x = 0;
		rect.y = 0;
		rect.w = SCREEN_WIDTH;
		rect.h = SCREEN_HEIGHT;
		Gfx_DrawRect(&rect, clear_r, clear_g, clear_b);
	}
	
	//Traverse display list
	while (dlist_p > dlist)
	{
		//Step back
		dlist_p--;
		
		//Display command
		Gfx_DisplayCmd(dlist_p);
	}
	
	//Final batch push
	Gfx_PushBatch();
	
	//Swap window buffers
	glfwSwapBuffers(window);
	
	//Handle events
	glfwPollEvents();
	
	//Initialize frame
	dlist_p = dlist;
	batch_buffer_p = &batch_buffer[0][0];
	batch_texture_id = 0;
}

void Gfx_SetClear(u8 r, u8 g, u8 b)
{
	//Update clear colour
	clear_r = r;
	clear_g = g;
	clear_b = b;
}

void Gfx_EnableClear(void)
{
	//Enable clear
	clear_e = true;
}

void Gfx_DisableClear(void)
{
	//Disable clear
	clear_e = false;
}

void Gfx_LoadTex(Gfx_Tex *tex, IO_Data data, Gfx_LoadTex_Flag flag)
{
	//Read TIM header
	u8 tim_header = ((u8*)data)[4];
	u8 tim_bpp = tim_header & 3;
	
	switch (tim_bpp)
	{
		case 0: //4bpp
		{
			//Read CLUT header
			u8 *tim_clut = &((u8*)data)[8];
			u32 tim_clut_l = tim_clut[0] | (tim_clut[1] << 8) | (tim_clut[2] << 16) | (tim_clut[3] << 24);
			u16 tim_clut_w = tim_clut[8]  | (tim_clut[9] << 8);
			//u16 tim_clut_h = tim_clut[10] | (tim_clut[11] << 8);
			u8 *tim_clut_data = &tim_clut[12];
			
			//Convert palette
		#if PSXF_GL == PSXF_GL_ES
			static u8 tex_palette[16][4]; //RGBA8888
		#else
			static u8 tex_palette[16][2]; //RGBA5551
		#endif
			
			u8 *tex_palette_p = &tex_palette[0][0];
			const u8 *tim_clut_data_p = tim_clut_data;
		#if PSXF_GL == PSXF_GL_ES
			//Convert palette to RGBA8888
			for (u16 i = 0; i < tim_clut_w; i++, tex_palette_p += 4, tim_clut_data_p += 2)
			{
				u16 raw_pal = tim_clut_data_p[0] | (tim_clut_data_p[1] << 8);
				if (raw_pal == 0)
				{
					tex_palette_p[0] = 0;
					tex_palette_p[1] = 0;
					tex_palette_p[2] = 0;
					tex_palette_p[3] = 0;
				}
				else
				{
					u8 r = (u8)(raw_pal & 31);
					u8 g = (u8)((raw_pal >> 5) & 31);
					u8 b = (u8)((raw_pal >> 10) & 31);
					b = (b << 3) | (b & 7);
					g = (g << 3) | (g & 7);
					r = (r << 3) | (r & 7);
					
					tex_palette_p[0] = r;
					tex_palette_p[1] = g;
					tex_palette_p[2] = b;
					tex_palette_p[3] = 0xFF;
				}
			}
		#else
			//Copy the RGBA5551 palette as-is, and correct its alpha bit
			for (u16 i = 0; i < tim_clut_w; i++, tex_palette_p += 2, tim_clut_data_p += 2)
			{
				tex_palette_p[0] = tim_clut_data_p[0];
				tex_palette_p[1] = tim_clut_data_p[1];

				//Set the alpha bit if not transparent
				tex_palette_p[1] |= tim_clut_data_p[0] || tim_clut_data_p[1] ? 0x80 : 0;
			}
		#endif
			
			//Read texture header
			u8 *tim_tex = &((u8*)tim_clut)[tim_clut_l];
			//u32 tim_tex_l = tim_tex[0] | (tim_tex[1] << 8) | (tim_tex[2] << 16) | (tim_tex[3] << 24);
			u16 tim_tex_x = tim_tex[4] | (tim_tex[5] << 8);
			u16 tim_tex_y = tim_tex[6] | (tim_tex[7] << 8);
			u16 tim_tex_w = tim_tex[8]  | (tim_tex[9] << 8);
			u16 tim_tex_h = tim_tex[10] | (tim_tex[11] << 8);
			u8 *tim_tex_data = &tim_tex[12];
			
			//Convert tpage coordinate from 16bpp 1024x512 to 4bpp 2048x1024
			tex->tpage_x = (tim_tex_x * 4) % VRAM_WIDTH;
			tex->tpage_y = tim_tex_y + ((tim_tex_x * 4) / VRAM_WIDTH) * (VRAM_HEIGHT / 2);
			
			//Convert art
		#if PSXF_GL == PSXF_GL_ES
			static u8 tex_data[256*256][4]; //RGBA8888
		#else
			static u8 tex_data[256*256][2]; //RGBA5551
		#endif
			
			u8 *tex_data_p = &tex_data[0][0];
			const u8 *tim_tex_data_p = tim_tex_data;
		#if PSXF_GL == PSXF_GL_ES
			//Output RGBA8888 bitmap
			for (size_t i = (tim_tex_w << 1) * tim_tex_h; i > 0; i--, tex_data_p += 8, tim_tex_data_p++)
			{
				u8 *mapp;
				mapp = &tex_palette[*tim_tex_data_p & 0xF][0];
				tex_data_p[0] = mapp[0];
				tex_data_p[1] = mapp[1];
				tex_data_p[2] = mapp[2];
				tex_data_p[3] = mapp[3];
				mapp = &tex_palette[*tim_tex_data_p >> 4][0];
				tex_data_p[4] = mapp[0];
				tex_data_p[5] = mapp[1];
				tex_data_p[6] = mapp[2];
				tex_data_p[7] = mapp[3];
			}
		#else
			//Output RGBA5551 bitmap
			for (size_t i = (tim_tex_w << 1) * tim_tex_h; i > 0; i--, tex_data_p += 4, tim_tex_data_p++)
			{
				u8 *mapp;
				mapp = &tex_palette[*tim_tex_data_p & 0xF][0];
				tex_data_p[0] = mapp[0];
				tex_data_p[1] = mapp[1];
				mapp = &tex_palette[*tim_tex_data_p >> 4][0];
				tex_data_p[2] = mapp[0];
				tex_data_p[3] = mapp[1];
			}
		#endif
			
			//Upload to texture
			Gfx_UploadTexture(vram_texture, tex->tpage_x, tex->tpage_y, &tex_data[0][0], tim_tex_w << 2, tim_tex_h);
			break;
		}
		case 1: //8bpp
		{
			//Read CLUT header
			u8 *tim_clut = &((u8*)data)[8];
			u32 tim_clut_l = tim_clut[0] | (tim_clut[1] << 8) | (tim_clut[2] << 16) | (tim_clut[3] << 24);
			u16 tim_clut_w = tim_clut[8]  | (tim_clut[9] << 8);
			//u16 tim_clut_h = tim_clut[10] | (tim_clut[11] << 8);
			u8 *tim_clut_data = &tim_clut[12];
			
			//Convert palette
		#if PSXF_GL == PSXF_GL_ES
			u8 tex_palette[256][4]; //RGBA8888
		#else
			u8 tex_palette[256][2]; //RGBA5551
		#endif
			
			u8 *tex_palette_p = &tex_palette[0][0];
			const u8 *tim_clut_data_p = tim_clut_data;
		#if PSXF_GL == PSXF_GL_ES
			//Convert palette to RGBA8888
			for (u16 i = 0; i < tim_clut_w; i++, tex_palette_p += 4, tim_clut_data_p += 2)
			{
				u16 raw_pal = tim_clut_data_p[0] | (tim_clut_data_p[1] << 8);
				if (raw_pal == 0)
				{
					tex_palette_p[0] = 0;
					tex_palette_p[1] = 0;
					tex_palette_p[2] = 0;
					tex_palette_p[3] = 0;
				}
				else
				{
					u8 r = (u8)(raw_pal & 31);
					u8 g = (u8)((raw_pal >> 5) & 31);
					u8 b = (u8)((raw_pal >> 10) & 31);
					b = (b << 3) | (b & 7);
					g = (g << 3) | (g & 7);
					r = (r << 3) | (r & 7);
					
					tex_palette_p[0] = r;
					tex_palette_p[1] = g;
					tex_palette_p[2] = b;
					tex_palette_p[3] = 0xFF;
				}
			}
		#else
			//Copy the RGBA5551 palette as-is, and correct its alpha bit
			for (u16 i = 0; i < tim_clut_w; i++, tex_palette_p += 2, tim_clut_data_p += 2)
			{
				tex_palette_p[0] = tim_clut_data_p[0];
				tex_palette_p[1] = tim_clut_data_p[1];

				//Set the alpha bit if not transparent
				tex_palette_p[1] |= tim_clut_data_p[0] || tim_clut_data_p[1] ? 0x80 : 0;
			}
		#endif
			
			//Read texture header
			u8 *tim_tex = &((u8*)tim_clut)[tim_clut_l];
			//u32 tim_tex_l = tim_tex[0] | (tim_tex[1] << 8) | (tim_tex[2] << 16) | (tim_tex[3] << 24);
			u16 tim_tex_x = tim_tex[4] | (tim_tex[5] << 8);
			u16 tim_tex_y = tim_tex[6] | (tim_tex[7] << 8);
			u16 tim_tex_w = tim_tex[8]  | (tim_tex[9] << 8);
			u16 tim_tex_h = tim_tex[10] | (tim_tex[11] << 8);
			u8 *tim_tex_data = &tim_tex[12];
			
			//Convert tpage coordinate from 16bpp 1024x512 to 4bpp 2048x1024
			tex->tpage_x = (tim_tex_x * 4) % VRAM_WIDTH;
			tex->tpage_y = tim_tex_y + ((tim_tex_x * 4) / VRAM_WIDTH) * (VRAM_HEIGHT / 2);
			
			//Convert art
		#if PSXF_GL == PSXF_GL_ES
			u8 tex_data[256*256][4]; //RGBA8888
		#else
			u8 tex_data[256*256][2]; //RGBA5551
		#endif
			
			u8 *tex_data_p = &tex_data[0][0];
			const u8 *tim_tex_data_p = tim_tex_data;
		#if PSXF_GL == PSXF_GL_ES
			//Output RGBA8888 bitmap
			for (size_t i = (tim_tex_w << 1) * tim_tex_h; i > 0; i--, tex_data_p += 4, tim_tex_data_p++)
			{
				u8 *mapp = &tex_palette[*tim_tex_data_p][0];
				tex_data_p[0] = mapp[0];
				tex_data_p[1] = mapp[1];
				tex_data_p[2] = mapp[2];
				tex_data_p[3] = mapp[3];
			}
		#else
			//Output RGBA5551 bitmap
			for (size_t i = (tim_tex_w << 1) * tim_tex_h; i > 0; i--, tex_data_p += 2, tim_tex_data_p++)
			{
				u8 *mapp = &tex_palette[*tim_tex_data_p][0];
				tex_data_p[0] = mapp[0];
				tex_data_p[1] = mapp[1];
			}
		#endif
			
			//Upload to texture
			Gfx_UploadTexture(vram_texture, tex->tpage_x, tex->tpage_y, &tex_data[0][0], tim_tex_w << 1, tim_tex_h);
			break;
		}
		case 2: //16bpp
		{
			sprintf(error_msg, "[Gfx_LoadTex] 16bpp unsupported");
			ErrorLock(); //Doesn't actually return
			break;
		}
		case 3: //24bpp
		{
			sprintf(error_msg, "[Gfx_LoadTex] 24bpp unsupported");
			ErrorLock(); //Doesn't actually return
			break;
		}
	}
	
	if (flag & GFX_LOADTEX_FREE)
		Mem_Free(data);
}

void Gfx_DrawRect(const RECT *rect, u8 r, u8 g, u8 b)
{
	Gfx_BlendRect(rect, r, g, b, 0xFF);
}

void Gfx_BlendRect(const RECT *rect, u8 r, u8 g, u8 b, u8 mode)
{
	RECT src;
	src.x = 0.0f;
	src.y = 0.0f;
	src.w = 0.0f;
	src.h = 0.0f;

	POINT tl, tr, bl, br;
	tl.x = bl.x = (float)rect->x;
	tl.y = tr.y = (float)rect->y;
	tr.x = br.x = (float)rect->x + (float)rect->w;
	bl.y = br.y = (float)rect->y + (float)rect->h;

	Gfx_SubmitCommand(plain_texture, &src, &tl, &tr, &bl, &br, r / 255.0f, g / 255.0f, b / 255.0f, mode);
}

void Gfx_BlitTexCol(Gfx_Tex *tex, const RECT *src, s32 x, s32 y, u8 r, u8 g, u8 b)
{
	POINT tl, tr, bl, br;
	tl.x = bl.x = (float)x;
	tl.y = tr.y = (float)y;
	tr.x = br.x = (float)x + (float)src->w;
	bl.y = br.y = (float)y + (float)src->h;

	Gfx_DrawTexArbCol(tex, src, &tl, &tr, &bl, &br, r, g, b);
}

void Gfx_BlitTex(Gfx_Tex *tex, const RECT *src, s32 x, s32 y)
{
	Gfx_BlitTexCol(tex, src, x, y, 0x80, 0x80, 0x80);
}

void Gfx_DrawTexCol(Gfx_Tex *tex, const RECT *src, const RECT *dst, u8 r, u8 g, u8 b)
{
	POINT tl, tr, bl, br;
	tl.x = bl.x = (float)dst->x;
	tl.y = tr.y = (float)dst->y;
	tr.x = br.x = (float)dst->x + (float)dst->w;
	bl.y = br.y = (float)dst->y + (float)dst->h;

	Gfx_DrawTexArbCol(tex, src, &tl, &tr, &bl, &br, r, g, b);
}

void Gfx_DrawTex(Gfx_Tex *tex, const RECT *src, const RECT *dst)
{
	Gfx_DrawTexCol(tex, src, dst, 0x80, 0x80, 0x80);
}

void Gfx_DrawTexArbCol(Gfx_Tex *tex, const RECT *src, const POINT *p0, const POINT *p1, const POINT *p2, const POINT *p3, u8 r, u8 g, u8 b)
{
	RECT vram_src;
	vram_src.x = tex->tpage_x + src->x;
	vram_src.y = tex->tpage_y + src->y;
	vram_src.w = src->w;
	vram_src.h = src->h;

	Gfx_SubmitCommand(vram_texture, &vram_src, p0, p1, p2, p3, r / 128.0f, g / 128.0f, b / 128.0f, 0xFF);
}

void Gfx_DrawTexArb(Gfx_Tex *tex, const RECT *src, const POINT *p0, const POINT *p1, const POINT *p2, const POINT *p3)
{
	Gfx_DrawTexArbCol(tex, src, p0, p1, p2, p3, 0x80, 0x80, 0x80);
}

void Gfx_BlendTexArb(Gfx_Tex *tex, const RECT *src, const POINT *p0, const POINT *p1, const POINT *p2, const POINT *p3, u8 mode)
{
	RECT vram_src;
	vram_src.x = tex->tpage_x + src->x;
	vram_src.y = tex->tpage_y + src->y;
	vram_src.w = src->w;
	vram_src.h = src->h;

	Gfx_SubmitCommand(vram_texture, &vram_src, p0, p1, p2, p3, 1.0f, 1.0f, 1.0f, mode);
}
