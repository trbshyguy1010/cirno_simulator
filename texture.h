#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>

class Texture {
public:
    unsigned int tex;
    Texture(GLenum slot);
    void ParameterTex(GLuint width, GLuint height, unsigned char* bytes);
    void BindTex();
    void UnbindTex();
    void DeleteTex();
};

#endif // TEXTURE_H
