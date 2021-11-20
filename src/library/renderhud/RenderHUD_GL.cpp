/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "RenderHUD_GL.h"
#ifdef LIBTAS_ENABLE_HUD

#include "../logging.h"
#include "../hook.h"
#include "../ScreenCapture.h"

namespace libtas {

DECLARE_ORIG_POINTER(glGetIntegerv)
DECLARE_ORIG_POINTER(glGenTextures)
DECLARE_ORIG_POINTER(glDeleteTextures)
DECLARE_ORIG_POINTER(glBindTexture)
DECLARE_ORIG_POINTER(glBindSampler)
DECLARE_ORIG_POINTER(glTexParameteri)
DECLARE_ORIG_POINTER(glTexImage2D)
DECLARE_ORIG_POINTER(glActiveTexture)
DECLARE_ORIG_POINTER(glUseProgram)
DECLARE_ORIG_POINTER(glPixelStorei)
DECLARE_ORIG_POINTER(glGetError)
DECLARE_ORIG_POINTER(glGenBuffers)
DECLARE_ORIG_POINTER(glGenVertexArrays)
DECLARE_ORIG_POINTER(glBindVertexArray)
DECLARE_ORIG_POINTER(glBindBuffer)
DECLARE_ORIG_POINTER(glBufferData)
DECLARE_ORIG_POINTER(glVertexAttribPointer)
DECLARE_ORIG_POINTER(glEnableVertexAttribArray)
DECLARE_ORIG_POINTER(glCreateShader)
DECLARE_ORIG_POINTER(glShaderSource)
DECLARE_ORIG_POINTER(glCompileShader)
DECLARE_ORIG_POINTER(glGetShaderiv)
DECLARE_ORIG_POINTER(glGetShaderInfoLog)
DECLARE_ORIG_POINTER(glCreateProgram)
DECLARE_ORIG_POINTER(glAttachShader)
DECLARE_ORIG_POINTER(glLinkProgram)
DECLARE_ORIG_POINTER(glGetProgramiv)
DECLARE_ORIG_POINTER(glGetProgramInfoLog)
DECLARE_ORIG_POINTER(glDetachShader)
DECLARE_ORIG_POINTER(glDeleteShader)
DECLARE_ORIG_POINTER(glDrawElements)
DECLARE_ORIG_POINTER(glDeleteBuffers)
DECLARE_ORIG_POINTER(glDeleteVertexArrays)
DECLARE_ORIG_POINTER(glDeleteProgram)
DECLARE_ORIG_POINTER(glEnable)
DECLARE_ORIG_POINTER(glDisable)
DECLARE_ORIG_POINTER(glIsEnabled)
DECLARE_ORIG_POINTER(glBlendFunc)

GLuint RenderHUD_GL::texture = 0;
GLuint RenderHUD_GL::vao = 0;
GLuint RenderHUD_GL::vbo = 0;
GLuint RenderHUD_GL::ebo = 0;
GLuint RenderHUD_GL::programID = 0;

float RenderHUD_GL::vertices[20] = {
    // positions          // texture coords
     0.5f,  0.5f, 0.0f,   1.0f, 1.0f,   // top right
     0.5f, -0.5f, 0.0f,   1.0f, 0.0f,   // bottom right
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,   // bottom left
    -0.5f,  0.5f, 0.0f,   0.0f, 1.0f    // top left 
};

GLuint RenderHUD_GL::indices[6] = {
    0, 1, 3,
    1, 2, 3
};

RenderHUD_GL::~RenderHUD_GL() {
    fini();
}

void RenderHUD_GL::init(bool stateGLES)
{
    if (texture == 0) {
        LINK_NAMESPACE(glGenTextures, "GL");
        LINK_NAMESPACE(glGetIntegerv, "GL");
        LINK_NAMESPACE(glActiveTexture, "GL");
        LINK_NAMESPACE(glDeleteTextures, "GL");
        LINK_NAMESPACE(glBindTexture, "GL");
        LINK_NAMESPACE(glGetError, "GL");
        LINK_NAMESPACE(glGenBuffers, "GL");
        LINK_NAMESPACE(glGenVertexArrays, "GL");
        LINK_NAMESPACE(glBindVertexArray, "GL");
        LINK_NAMESPACE(glBindBuffer, "GL");
        LINK_NAMESPACE(glBufferData, "GL");
        LINK_NAMESPACE(glVertexAttribPointer, "GL");
        LINK_NAMESPACE(glEnableVertexAttribArray, "GL");
        LINK_NAMESPACE(glCreateShader, "GL");
        LINK_NAMESPACE(glShaderSource, "GL");
        LINK_NAMESPACE(glCompileShader, "GL");
        LINK_NAMESPACE(glGetShaderiv, "GL");
        LINK_NAMESPACE(glGetShaderInfoLog, "GL");
        LINK_NAMESPACE(glCreateProgram, "GL");
        LINK_NAMESPACE(glAttachShader, "GL");
        LINK_NAMESPACE(glLinkProgram, "GL");
        LINK_NAMESPACE(glGetProgramiv, "GL");
        LINK_NAMESPACE(glGetProgramInfoLog, "GL");
        LINK_NAMESPACE(glDetachShader, "GL");
        LINK_NAMESPACE(glDeleteShader, "GL");
        LINK_NAMESPACE(glDeleteBuffers, "GL");
        LINK_NAMESPACE(glDeleteVertexArrays, "GL");
        LINK_NAMESPACE(glDeleteProgram, "GL");

        GlobalNative gn;
        GLint res;

        /* Get previous active texture */
        GLint oldActiveTex;
        orig::glGetIntegerv(GL_ACTIVE_TEXTURE, &oldActiveTex);

        /* Create a new texture */
        GLenum error;
        orig::glGetError();
        orig::glActiveTexture(GL_TEXTURE0);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glActiveTexture failed with error %d", error);

        orig::glGenTextures(1, &texture);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glGenTextures failed with error %d", error);

        if (oldActiveTex != 0) {
            orig::glActiveTexture(oldActiveTex);
            if ((error = orig::glGetError()) != GL_NO_ERROR)
                debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glActiveTexture failed with error %d", error);
        }

        /* Create buffers */
        orig::glGenBuffers(1, &vbo);        
        orig::glGenBuffers(1, &ebo);
        orig::glGenVertexArrays(1, &vao);

        /* Get previous bound buffers */
        GLint oldArrayBuffer;
        orig::glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &oldArrayBuffer);
        GLint oldElementArrayBuffer;
        orig::glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &oldElementArrayBuffer);
        GLint oldVertexArray;
        orig::glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &oldVertexArray);
        
        /* Set up the vertex array */
        orig::glBindVertexArray(vao);    
        orig::glBindBuffer(GL_ARRAY_BUFFER, vbo);
        orig::glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        orig::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        orig::glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        
        orig::glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        orig::glEnableVertexAttribArray(0);
        orig::glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        orig::glEnableVertexAttribArray(1);
        
        orig::glBindVertexArray(oldVertexArray);
        orig::glBindBuffer(GL_ARRAY_BUFFER, oldArrayBuffer);
        orig::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oldElementArrayBuffer);
        
        /* Create the shaders */
    	GLuint vertexShaderID = orig::glCreateShader(GL_VERTEX_SHADER);
    	GLuint fragmentShaderID = orig::glCreateShader(GL_FRAGMENT_SHADER);
        
    	/* Compile the vertex shader */
        const char *vertexShaderSource = R"(
            #version 120
            attribute vec3 aPos;
            attribute vec2 aTexCoord;
            varying vec2 TexCoord;
        
            void main()
            {
                gl_Position = vec4(aPos, 1.0);
                TexCoord = aTexCoord;
            }
        )";

        const char *vertexShaderSourceES = R"(
            #version 100
            attribute vec3 aPos;
            attribute vec2 aTexCoord;
            varying vec2 TexCoord;
        
            void main()
            {
                gl_Position = vec4(aPos, 1.0);
                TexCoord = aTexCoord;
            }
        )";

        if (stateGLES) {
            orig::glShaderSource(vertexShaderID, 1, &vertexShaderSourceES , NULL);
        }
        else {
            orig::glShaderSource(vertexShaderID, 1, &vertexShaderSource , NULL);            
        }
    	orig::glCompileShader(vertexShaderID);
        
        
        /* Check the vertex shader */
    	orig::glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &res);
        if (!res) {
            char buf[512];
            orig::glGetShaderInfoLog(vertexShaderID, 512, NULL, buf);
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "Vertex shader compilation failed with error %s", buf);
        }
        
        /* Compile the fragment shader */
        const char *fragmentShaderSource = R"(
            #version 120
            uniform sampler2D ourTexture;
            varying vec2 TexCoord;
        
            void main()
            {
                gl_FragColor = texture2D(ourTexture, TexCoord);
            }
        )";

        const char *fragmentShaderSourceES = R"(
            #version 100
            precision mediump float;
            uniform sampler2D ourTexture;
            varying vec2 TexCoord;
        
            void main()
            {
                gl_FragColor = texture2D(ourTexture, TexCoord);
            }
        )";
        
        if (stateGLES) {
            orig::glShaderSource(fragmentShaderID, 1, &fragmentShaderSourceES , NULL);
        }
        else {
            orig::glShaderSource(fragmentShaderID, 1, &fragmentShaderSource , NULL);
        }

    	orig::glCompileShader(fragmentShaderID);
        
        /* Check the fragment shader */
    	orig::glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &res);
        if (!res) {
            char buf[512];
            orig::glGetShaderInfoLog(fragmentShaderID, 512, NULL, buf);
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "Fragment shader compilation failed with error %s", buf);
        }
        
    	/* Link the program */
    	programID = orig::glCreateProgram();
    	orig::glAttachShader(programID, vertexShaderID);
    	orig::glAttachShader(programID, fragmentShaderID);
    	orig::glLinkProgram(programID);
        
    	/* Check the program */
    	orig::glGetProgramiv(programID, GL_LINK_STATUS, &res);
        if (!res) {
            char buf[512];
            orig::glGetProgramInfoLog(programID, 512, NULL, buf);
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "Program link failed with error %s", buf);
        }
        
    	orig::glDetachShader(programID, vertexShaderID);
    	orig::glDetachShader(programID, fragmentShaderID);
        
    	orig::glDeleteShader(vertexShaderID);
    	orig::glDeleteShader(fragmentShaderID);
    }
}

void RenderHUD_GL::fini()
{
    GlobalNative gn;

    if (texture != 0) {
        orig::glDeleteTextures(1, &texture);
        texture = 0;
    }
    if (vao != 0) {
        orig::glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
    if (vbo != 0) {
        orig::glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
    if (ebo != 0) {
        orig::glDeleteBuffers(1, &ebo);
        ebo = 0;
    }
    if (programID != 0) {
        orig::glDeleteProgram(programID);
        programID = 0;
    }
}

void RenderHUD_GL::renderSurface(std::unique_ptr<SurfaceARGB> surf, int x, int y)
{
    RenderHUD_GL::init(isGLES);

    LINK_NAMESPACE(glEnable, "GL");
    LINK_NAMESPACE(glDisable, "GL");
    LINK_NAMESPACE(glIsEnabled, "GL");
    LINK_NAMESPACE(glBlendFunc, "GL");
    LINK_NAMESPACE(glBindTexture, "GL");
    LINK_NAMESPACE(glBindSampler, "GL");
    LINK_NAMESPACE(glTexImage2D, "GL");
    LINK_NAMESPACE(glTexParameteri, "GL");
    LINK_NAMESPACE(glUseProgram, "GL");
    LINK_NAMESPACE(glGetIntegerv, "GL");
    LINK_NAMESPACE(glPixelStorei, "GL");
    LINK_NAMESPACE(glActiveTexture, "GL");
    LINK_NAMESPACE(glDrawElements, "GL");

    GlobalNative gn;

    GLenum error;
    orig::glGetError();

    GLboolean isBlend = orig::glIsEnabled(GL_BLEND);
    orig::glEnable(GL_BLEND);
    orig::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
    /* Save the previous program */
    GLint oldProgram;
    orig::glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);

    /* Save previous unpack row length */
    GLint oldUnpackRow;
    orig::glGetIntegerv(GL_UNPACK_ROW_LENGTH, &oldUnpackRow);
    if (oldUnpackRow != 0) {
        orig::glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glPixelStorei failed with error %d", error);
    }

    /* Save previous binded texture and active texture unit */
    GLint oldTex;
    orig::glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTex);
    GLint oldActiveTex;
    orig::glGetIntegerv(GL_ACTIVE_TEXTURE, &oldActiveTex);

    /* Create our text as a texture */
    orig::glActiveTexture(GL_TEXTURE0);
    if ((error = orig::glGetError()) != GL_NO_ERROR)
        debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glActiveTexture failed with error %d", error);

    orig::glBindTexture(GL_TEXTURE_2D, texture);
    if ((error = orig::glGetError()) != GL_NO_ERROR)
        debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glBindTexture failed with error %d", error);

    orig::glBindSampler(0, 0);

    orig::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    orig::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    orig::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, surf->pixels.data());
    if ((error = orig::glGetError()) != GL_NO_ERROR)
        debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glTexImage2D failed with error %d", error);

    /* Update the triangle coordinates */
    int width, height;
    ScreenCapture::getDimensions(width, height);

    /* Flip y coord */
    y = height - y - surf->h;

    float left = -1.0 + (float) 2.0 * x / (float) width;
    float right = -1.0 + (float) 2.0 * (x + surf->w) / (float) width;
    float top = -1.0 + (float) 2.0 * y / (float) height;
    float bottom = -1.0 + (float) 2.0 * (y + surf->h) / (float) height;

    vertices[0] = vertices[5] = right;
    vertices[10] = vertices[15] = left;
    vertices[1] = vertices[16] = top;
    vertices[6] = vertices[11] = bottom;

    /* Send modified vertices */
    GLint oldVertexArray;
    orig::glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &oldVertexArray);
    GLint oldArrayBuffer;
    orig::glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &oldArrayBuffer);

    orig::glBindBuffer(GL_ARRAY_BUFFER, vbo);
    orig::glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    orig::glUseProgram(programID);

    orig::glBindVertexArray(vao);
    orig::glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    orig::glBindVertexArray(oldVertexArray);
    orig::glBindBuffer(GL_ARRAY_BUFFER, oldArrayBuffer);
    
    /* Restore previous binded texture and active texture unit */
    if (oldTex != 0) {
        orig::glBindTexture(GL_TEXTURE_2D, oldTex);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glBindTexture failed with error %d", error);
    }
    if (oldActiveTex != 0) {
        orig::glActiveTexture(oldActiveTex);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glActiveTexture failed with error %d", error);
    }

    /* Restore unpack row length */
    if (oldUnpackRow != 0) {
        orig::glPixelStorei(GL_UNPACK_ROW_LENGTH, oldUnpackRow);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glPixelStorei failed with error %d", error);
    }

    /* Restore the previous program */
    orig::glUseProgram(oldProgram);
    if ((error = orig::glGetError()) != GL_NO_ERROR)
        debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glUseProgram failed with error %d", error);
        
    if (!isBlend)
        orig::glDisable(GL_BLEND);
}

}

#endif
