#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Utils/Color.hpp"

namespace Guijo {
#ifdef USE_OPENGL
    class Shader {
    public:
        unsigned int ID;

        Shader(std::string_view vertex, std::string_view frag, std::string_view geo = "");

        inline void Clean() const { glDeleteProgram(ID); };
        inline void Use() const { glUseProgram(ID); };
        inline void SetBool(GLint n, bool v) const { glUniform1i(n, (int)v); };
        inline void SetInt(GLint n, int v) const { glUniform1i(n, v); };
        inline void SetFloat(GLint n, float v) const { glUniform1f(n, v); };
        inline void SetFloatA(GLint n, const float* v, const unsigned int a) const { glUniform4fv(n, a, v); };
        inline void SetIntA(GLint n, const int* v, const unsigned int a) const { glUniform1iv(n, a, v); };
        inline void SetVec2(GLint n, const glm::vec2& v) const { glUniform2fv(n, 1, &v.x); };
        inline void SetVec3(GLint n, const glm::vec3& v) const { glUniform3fv(n, 1, &v.x); };
        inline void SetVec4(GLint n, const glm::vec4& v) const { glUniform4fv(n, 1, &v.x); };
        inline void SetMat2(GLint n, const glm::mat2& m) const { glUniformMatrix2fv(n, 1, GL_FALSE, &m[0][0]); };
        inline void SetMat3(GLint n, const glm::mat3& m) const { glUniformMatrix3fv(n, 1, GL_FALSE, &m[0][0]); };
        inline void SetMat4(GLint n, const glm::mat4& m) const { glUniformMatrix4fv(n, 1, GL_FALSE, &m[0][0]); };
    };
#endif
}