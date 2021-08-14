#include "../gfx.h"

#include "../main.h"
#include "../mem.h"

#ifdef PSXF_GLES
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

//Window
GLFWwindow *window;

//Render state
static mat4 projection;

static float clear_r, clear_g, clear_b;
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
	float r, g, b, a;
	GLuint texture_id;
} Gfx_Cmd;

static Gfx_Cmd dlist[0x200];
static Gfx_Cmd *dlist_p;

typedef struct
{
	float x, y;
	float u, v;
	float r, g, b, a;
} Gfx_Vertex;

//Shader
#ifdef PSXF_GLES
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
#else
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
#endif

static GLuint generic_shader_id;

//Textures
#define TPAGE_X 16
#define TPAGE_Y 2

GLuint plain_texture;
GLuint tpage_texture[TPAGE_Y * TPAGE_X];

//Batch
GLuint batch_vao;
GLuint batch_vbo;

GLuint batch_texture_id;

static Gfx_Vertex batch_buffer[COUNT_OF(dlist)][6]; //TODO: index buffer
static Gfx_Vertex *batch_buffer_p;

//Internal gfx functions
static void Gfx_FramebufferSizeCallback(GLFWwindow *window, int fb_width, int fb_height)
{
	// Center the viewport within the window while maintaining the aspect ratio
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

static GLuint Gfx_CompileShader(const char *src_vert, const char *src_frag)
{
	//Create shader
	GLint shader_status;
	GLuint shader_id = glCreateProgram();
	
	//Compile vertex shader
	GLuint vertex_id = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_id, 1, &src_vert, NULL);
	glCompileShader(vertex_id);
	
	glGetShaderiv(vertex_id, GL_COMPILE_STATUS, &shader_status);
	if (shader_status != GL_TRUE)
	{
		char buffer[0x200];
		glGetShaderInfoLog(vertex_id, sizeof(buffer), NULL, buffer);
		sprintf(error_msg, "[Gfx_CompileShader] Failed to compile vertex shader: %s", buffer);
		ErrorLock();
	}
	
	//Compile fragment shader
	GLuint fragment_id = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_id, 1, &src_frag, NULL);
	glCompileShader(fragment_id);
	
	glGetShaderiv(fragment_id, GL_COMPILE_STATUS, &shader_status);
	if (shader_status != GL_TRUE)
	{
		char buffer[0x200];
		glGetShaderInfoLog(fragment_id, sizeof(buffer), NULL, buffer);
		sprintf(error_msg, "[Gfx_CompileShader] Failed to compile fragment shader: %s", buffer);
		ErrorLock();
	}
	
	//Attach and link
	glAttachShader(shader_id, vertex_id);
	glAttachShader(shader_id, fragment_id);
	
	glBindAttribLocation(shader_id, 0, "v_position");
	glBindAttribLocation(shader_id, 1, "v_uv");
	glBindAttribLocation(shader_id, 2, "v_colour");
	
	glLinkProgram(shader_id);
	
	glGetProgramiv(shader_id, GL_LINK_STATUS, &shader_status);
	if (shader_status != GL_TRUE)
	{
		char buffer[0x200];
		glGetProgramInfoLog(shader_id, sizeof(buffer), NULL, buffer);
		sprintf(error_msg, "[Gfx_CompileShader] Failed to link shader: %s", buffer);
		ErrorLock();
	}
	
	glDetachShader(shader_id, vertex_id);
	glDetachShader(shader_id, fragment_id);
	
	return shader_id;
}

static GLuint Gfx_CreateTexture(GLint width, GLint height)
{
	//Create texture object
	GLuint texture_id;
	glGenTextures(1, &texture_id);
	
	//Set texture parameters
	glBindTexture(GL_TEXTURE_2D, texture_id);
#ifdef PSXF_GLES
	//OpenGL ES 2.0 does not support RGBA5551, so settle for RGBA8888 instead.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, NULL);
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	
	return texture_id;
}

static void Gfx_UploadTexture(GLuint texture_id, const u8 *data, GLint width, GLint height)
{
	//Upload data to texture
	glBindTexture(GL_TEXTURE_2D, texture_id);
#ifdef PSXF_GLES
	//OpenGL ES 2.0 does not support RGBA5551, so settle for RGBA8888 instead.
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (const void*)data);
#else
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, (const void*)data);
#endif
}

static void Gfx_PushBatch(void)
{
	//Drop if we haven't batched any data
	if (batch_buffer_p == &batch_buffer[0][0])
		return;
	
	//Bind VAO and VBO
#ifndef PSXF_GLES
	//OpenGL ES 2.0 doesn't have VAOs
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
}

static void Gfx_DisplayCmd(const Gfx_Cmd *cmd)
{
	//Push batch and bind if new texture
	if (cmd->texture_id != batch_texture_id)
	{
		Gfx_PushBatch();
		glBindTexture(GL_TEXTURE_2D, batch_texture_id = cmd->texture_id);
		batch_buffer_p = &batch_buffer[0][0];
	}
	
	//Push data to buffer
	batch_buffer_p[0].x = cmd->dst.tl.x;
	batch_buffer_p[0].y = cmd->dst.tl.y;
	batch_buffer_p[0].u = cmd->src.left;
	batch_buffer_p[0].v = cmd->src.top;
	batch_buffer_p[0].r = cmd->r;
	batch_buffer_p[0].g = cmd->g;
	batch_buffer_p[0].b = cmd->b;
	batch_buffer_p[0].a = cmd->a;
	
	batch_buffer_p[1].x = cmd->dst.bl.x;
	batch_buffer_p[1].y = cmd->dst.bl.y;
	batch_buffer_p[1].u = cmd->src.left;
	batch_buffer_p[1].v = cmd->src.bottom;
	batch_buffer_p[1].r = cmd->r;
	batch_buffer_p[1].g = cmd->g;
	batch_buffer_p[1].b = cmd->b;
	batch_buffer_p[1].a = cmd->a;
	
	batch_buffer_p[2].x = cmd->dst.tr.x;
	batch_buffer_p[2].y = cmd->dst.tr.y;
	batch_buffer_p[2].u = cmd->src.right;
	batch_buffer_p[2].v = cmd->src.top;
	batch_buffer_p[2].r = cmd->r;
	batch_buffer_p[2].g = cmd->g;
	batch_buffer_p[2].b = cmd->b;
	batch_buffer_p[2].a = cmd->a;
	
	batch_buffer_p[3].x = cmd->dst.tr.x;
	batch_buffer_p[3].y = cmd->dst.tr.y;
	batch_buffer_p[3].u = cmd->src.right;
	batch_buffer_p[3].v = cmd->src.top;
	batch_buffer_p[3].r = cmd->r;
	batch_buffer_p[3].g = cmd->g;
	batch_buffer_p[3].b = cmd->b;
	batch_buffer_p[3].a = cmd->a;
	
	batch_buffer_p[4].x = cmd->dst.bl.x;
	batch_buffer_p[4].y = cmd->dst.bl.y;
	batch_buffer_p[4].u = cmd->src.left;
	batch_buffer_p[4].v = cmd->src.bottom;
	batch_buffer_p[4].r = cmd->r;
	batch_buffer_p[4].g = cmd->g;
	batch_buffer_p[4].b = cmd->b;
	batch_buffer_p[4].a = cmd->a;
	
	batch_buffer_p[5].x = cmd->dst.br.x;
	batch_buffer_p[5].y = cmd->dst.br.y;
	batch_buffer_p[5].u = cmd->src.right;
	batch_buffer_p[5].v = cmd->src.bottom;
	batch_buffer_p[5].r = cmd->r;
	batch_buffer_p[5].g = cmd->g;
	batch_buffer_p[5].b = cmd->b;
	batch_buffer_p[5].a = cmd->a;
	
	batch_buffer_p += 6;
}

//Gfx functions
void Gfx_Init(void)
{
	//Set window hints
#ifdef PSXF_GLES
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#else
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
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

#ifndef PSXF_GLES	
	//Initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		sprintf(error_msg, "[Gfx_Init] Failed to initialize GLAD");
		ErrorLock();
	}
	
	if (!GLAD_GL_VERSION_3_2)
	{
		sprintf(error_msg, "[Gfx_Init] OpenGL 3.2 is not supported");
		ErrorLock();
	}
#endif
	
	//Initialize render state
	glm_ortho(0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f, -1.0f, 1.0f, projection);
	
	clear_r = 0.0f;
	clear_g = 0.0f;
	clear_b = 0.0f;
	clear_e = true;
	
	//Initialize OpenGL state
	//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	
	//Create shaders
	generic_shader_id = Gfx_CompileShader(generic_shader_vert, generic_shader_frag);
	glUseProgram(generic_shader_id);
	glUniformMatrix4fv(glGetUniformLocation(generic_shader_id, "u_projection"), 1, GL_FALSE, &projection[0][0]);
	
	//Create textures
	static const u8 plain_texture_data[] = {0xFF, 0xFF};
	Gfx_UploadTexture(plain_texture = Gfx_CreateTexture(1, 1), plain_texture_data, 1, 1);
	
	for (size_t i = 0; i < (TPAGE_X * TPAGE_Y); i++)
		tpage_texture[i] = Gfx_CreateTexture(256, 256);
	
	//Create batch VAO
#ifndef PSXF_GLES
	//OpenGL ES 2.0 doesn't have VAOs
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
		glClearColor(clear_r, clear_g, clear_b, 1.0f);
	#ifdef PSXF_GLES
		glClearDepthf(1.0f);
	#else
		glClearDepth(1.0f);
	#endif
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
	if (glfwWindowShouldClose(window))
		exit(0);
	
	//Initialize frame
	dlist_p = dlist;
	batch_buffer_p = &batch_buffer[0][0];
	batch_texture_id = 0;
}

void Gfx_SetClear(u8 r, u8 g, u8 b)
{
	//Update clear colour
	clear_r = (float)r / 255.0f;
	clear_g = (float)g / 255.0f;
	clear_b = (float)b / 255.0f;
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
		#ifdef PSXF_GLES
			u8 tex_palette[16][4]; //RGBA8888
		#else
			u8 tex_palette[16][2]; //RGBA5551
		#endif
			
			u8 *tex_palette_p = &tex_palette[0][0];
			const u8 *tim_clut_data_p = tim_clut_data;
		#ifdef PSXF_GLES
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
			
			tex->tpage = ((tim_tex_y >> 8) % TPAGE_Y) * TPAGE_X + ((tim_tex_x >> 6) % TPAGE_X);
			
			//Convert art
		#ifdef PSXF_GLES
			u8 tex_data[256*256][4]; //RGBA8888
		#else
			u8 tex_data[256*256][2]; //RGBA5551
		#endif
			
			u8 *tex_data_p = &tex_data[0][0];
			const u8 *tim_tex_data_p = tim_tex_data;
		#ifdef PSXF_GLES
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
			Gfx_UploadTexture(tpage_texture[tex->tpage], &tex_data[0][0], tim_tex_w << 2, tim_tex_h);
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
		#ifdef PSXF_GLES
			u8 tex_palette[256][4]; //RGBA8888
		#else
			u8 tex_palette[256][2]; //RGBA5551
		#endif
			
			u8 *tex_palette_p = &tex_palette[0][0];
			const u8 *tim_clut_data_p = tim_clut_data;
		#ifdef PSXF_GLES
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
			
			tex->tpage = ((tim_tex_y >> 8) % TPAGE_Y) * TPAGE_X + ((tim_tex_x >> 6) % TPAGE_X);
			
			//Convert art
		#ifdef PSXF_GLES
			u8 tex_data[256*256][4]; //RGBA8888
		#else
			u8 tex_data[256*256][2]; //RGBA5551
		#endif
			
			u8 *tex_data_p = &tex_data[0][0];
			const u8 *tim_tex_data_p = tim_tex_data;
		#ifdef PSXF_GLES
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
			Gfx_UploadTexture(tpage_texture[tex->tpage], &tex_data[0][0], tim_tex_w << 1, tim_tex_h);
			break;
		}
		case 2: //16bpp
		{
			break;
		}
		case 3: //24bpp
		{
			sprintf(error_msg, "[Gfx_LoadTex] 24bpp unsupported");
			ErrorLock();
		}
	}
	
	if (flag & GFX_LOADTEX_FREE)
		Mem_Free(data);
}

void Gfx_DrawRect(const RECT *rect, u8 r, u8 g, u8 b)
{
	//Create new command
	Gfx_Cmd cmd;
	cmd.src.left = cmd.src.top = cmd.src.right = cmd.src.bottom = 0.0f;
	cmd.dst.tl.x = cmd.dst.bl.x = (float)rect->x;
	cmd.dst.tl.y = cmd.dst.tr.y = (float)rect->y;
	cmd.dst.tr.x = cmd.dst.br.x = (float)rect->x + (float)rect->w;
	cmd.dst.bl.y = cmd.dst.br.y = (float)rect->y + (float)rect->h;
	cmd.r = (float)r / 255.0f;
	cmd.g = (float)g / 255.0f;
	cmd.b = (float)b / 255.0f;
	cmd.a = 1.0f;
	cmd.texture_id = plain_texture;
	
	//Push command
	*dlist_p++ = cmd;
}

void Gfx_BlitTexCol(Gfx_Tex *tex, const RECT *src, s32 x, s32 y, u8 r, u8 g, u8 b)
{
	//Create new command
	Gfx_Cmd cmd;
	cmd.src.left =   (float)src->x / 256.0f;
	cmd.src.top =    (float)src->y / 256.0f;
	cmd.src.right =  ((float)src->x + (float)src->w) / 256.0f;
	cmd.src.bottom = ((float)src->y + (float)src->h) / 256.0f;
	cmd.dst.tl.x = cmd.dst.bl.x = (float)x;
	cmd.dst.tl.y = cmd.dst.tr.y = (float)y;
	cmd.dst.tr.x = cmd.dst.br.x = (float)x + (float)src->w;
	cmd.dst.bl.y = cmd.dst.br.y = (float)y + (float)src->h;
	cmd.r = (float)r / 128.0f;
	cmd.g = (float)g / 128.0f;
	cmd.b = (float)b / 128.0f;
	cmd.a = 1.0f;
	cmd.texture_id = tpage_texture[tex->tpage];
	
	//Push command
	*dlist_p++ = cmd;
}

void Gfx_BlitTex(Gfx_Tex *tex, const RECT *src, s32 x, s32 y)
{
	Gfx_BlitTexCol(tex, src, x, y, 0x80, 0x80, 0x80);
}

void Gfx_DrawTexCol(Gfx_Tex *tex, const RECT *src, const RECT *dst, u8 r, u8 g, u8 b)
{
	//Create new command
	Gfx_Cmd cmd;
	cmd.src.left =   (float)src->x / 256.0f;
	cmd.src.top =    (float)src->y / 256.0f;
	cmd.src.right =  ((float)src->x + (float)src->w) / 256.0f;
	cmd.src.bottom = ((float)src->y + (float)src->h) / 256.0f;
	cmd.dst.tl.x = cmd.dst.bl.x = (float)dst->x;
	cmd.dst.tl.y = cmd.dst.tr.y = (float)dst->y;
	cmd.dst.tr.x = cmd.dst.br.x = (float)dst->x + (float)dst->w;
	cmd.dst.bl.y = cmd.dst.br.y = (float)dst->y + (float)dst->h;
	cmd.r = (float)r / 128.0f;
	cmd.g = (float)g / 128.0f;
	cmd.b = (float)b / 128.0f;
	cmd.a = 1.0f;
	cmd.texture_id = tpage_texture[tex->tpage];
	
	//Push command
	*dlist_p++ = cmd;
}

void Gfx_DrawTex(Gfx_Tex *tex, const RECT *src, const RECT *dst)
{
	Gfx_DrawTexCol(tex, src, dst, 0x80, 0x80, 0x80);
}

void Gfx_DrawTexArb(Gfx_Tex *tex, const RECT *src, const POINT *p0, const POINT *p1, const POINT *p2, const POINT *p3)
{
	
}
