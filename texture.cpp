#include "texture.h"

Texture::Texture(GLenum slot) {
    glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &tex);
    glActiveTexture(slot);
}

void Texture::ParameterTex(GLuint width, GLuint height, unsigned char* bytes) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
    glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Texture::UnbindTex() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::BindTex() {
    glBindTexture(GL_TEXTURE_2D, tex);
}

void Texture::DeleteTex() {
	glDeleteTextures(1, &tex);
}
