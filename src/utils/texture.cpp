#include <utils.h>
#include <stb_image.h>

int load_texture(std::string const& texture_file_path)
{
	unsigned texture_id;
	glGenTextures(1, &texture_id);

	glBindTexture(GL_TEXTURE_2D, texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nr_channels;
	unsigned char* data = stbi_load(texture_file_path.c_str(), &width, &height, &nr_channels, 0);

	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, nr_channels == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
    {
        std::cout << "Could not load texture at location: " << texture_file_path <<"\n";
        return -1;
    }
		
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(data);
	return texture_id;
}