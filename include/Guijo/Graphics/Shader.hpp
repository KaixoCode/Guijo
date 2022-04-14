#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Utils/Color.hpp"

namespace Guijo {
#ifdef USE_OPENGL
    class Shader {
    public:
        static inline unsigned int current = static_cast<unsigned int>(-1);
        unsigned int ID;

        Shader(std::string_view vertex, std::string_view frag, std::string_view geo = "");
        
        inline GLint uniform(std::string_view c) const { return glGetUniformLocation(ID, c.data()); }
        inline void clean() const { glDeleteProgram(ID); };
        inline bool use() const { if (current != ID) { glUseProgram(ID), current = ID; return true; } else return false; };

        struct Assigner {
            const Shader& self;
            GLint n;

            inline void operator=(bool v) const { glUniform1i(n, (int)v); };
            inline void operator=(int v) const { glUniform1i(n, v); };
            inline void operator=(float v) const { glUniform1f(n, v); };
            inline void operator=(std::pair<const float*, const unsigned int> a) const { glUniform4fv(n, a.second, a.first); };
            inline void operator=(std::pair<const int*, const unsigned int> a) const { glUniform1iv(n, a.second, a.first); };
            inline void operator=(const glm::vec2& v) const { glUniform2fv(n, 1, &v.x); };
            inline void operator=(const glm::vec3& v) const { glUniform3fv(n, 1, &v.x); };
            inline void operator=(const glm::vec4& v) const { glUniform4fv(n, 1, &v.x); };
            inline void operator=(const glm::mat2& m) const { glUniformMatrix2fv(n, 1, GL_FALSE, &m[0][0]); };
            inline void operator=(const glm::mat3& m) const { glUniformMatrix3fv(n, 1, GL_FALSE, &m[0][0]); };
            inline void operator=(const glm::mat4& m) const { glUniformMatrix4fv(n, 1, GL_FALSE, &m[0][0]); };
        };

        inline Assigner operator[](GLint i) const {
            return { *this, i };
        }
    };
#endif
}