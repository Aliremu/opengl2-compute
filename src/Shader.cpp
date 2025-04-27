#include "Shader.h"

#include <memory>
#include <optional>
#include <string_view>

#include <spdlog/spdlog.h>

namespace {
std::optional<GLuint> compileShader(GLenum type, const std::string_view& src) {
    GLuint shader = glCreateShader(type);

    const char* glSrc = src.data();
    glShaderSource(shader, 1, &glSrc, NULL);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        auto log = std::vector<char>(length);
        glGetShaderInfoLog(shader, length, NULL, log.data());
        spdlog::error("Shader compilation for {} failed: {}", type, log.data());
        glDeleteShader(shader);
        return std::nullopt;
    }

    return shader;
}
} // namespace

Shader::Shader(GLuint program) : m_program{program} {}

Shader::~Shader() { glDeleteProgram(m_program); }

std::shared_ptr<Shader> Shader::createShader(const std::string_view& vertSrc, const std::string_view& fragSrc) {
    auto vertShader = compileShader(GL_VERTEX_SHADER, vertSrc);
    auto fragShader = compileShader(GL_FRAGMENT_SHADER, fragSrc);

    if (!vertShader.has_value() || !fragShader.has_value()) {
        return nullptr;
    }

    auto program = glCreateProgram();
    glAttachShader(program, *vertShader);
    glAttachShader(program, *fragShader);
    glLinkProgram(program);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        spdlog::error("Program linking failed!");
        return nullptr;
    }

    glDeleteShader(*vertShader);
    glDeleteShader(*fragShader);

    return std::make_shared<Shader>(program);
}

void Shader::use() { glUseProgram(m_program); }

GLint Shader::getUniformLocation(const std::string& name) {
    if (m_uniforms.find(name) != m_uniforms.end()) {
        return m_uniforms.at(name);
    }

    auto location = glGetUniformLocation(m_program, name.c_str());
    m_uniforms[name] = location;

    return location;
}

void Shader::setInt(const std::string& name, int value) { glUniform1i(getUniformLocation(name), value); }

