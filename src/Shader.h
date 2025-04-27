#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include <GL/glew.h>

class Shader {
public:
    [[nodiscard]] static std::shared_ptr<Shader> createShader(
        const std::string_view& vertSrc,
        const std::string_view& fragSrc
    );

    Shader(GLuint program);
    ~Shader();

    void use();
    void setInt(const std::string& name, int value);

private:
    GLint getUniformLocation(const std::string& name);

private:
    GLuint m_program;
    std::unordered_map<std::string, GLint> m_uniforms;
};

