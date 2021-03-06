#ifndef CELLARWORKBENCH_GLPROGRAM_H
#define CELLARWORKBENCH_GLPROGRAM_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include <stack>
#include <memory>

#include <GLM/glm.hpp>

#include <GL3/gl3w.h>

#include "GlInputsOutputs.h"
#include "GlShader.h"


namespace cellar
{
    struct GlProgramBinary
    {
        GlProgramBinary() : length(0), binary(nullptr) {}
        ~GlProgramBinary() {delete [] binary;}

        GLint length;
        GLenum format;
        char* binary;
    };


    class CELLAR_EXPORT GlProgram
    {
    public:
        GlProgram();
        virtual ~GlProgram();

        GLuint id() const;
        const GlInputsOutputs& InOutLocations() const;

        // Build process
        bool setInAndOutLocations(const GlInputsOutputs& inout);
        bool addShader(const std::shared_ptr<GlShader>& shader);
        bool addShader(GLenum shaderType, const std::string& shaderName);
        bool addShader(GLenum shaderType, const std::vector<std::string>& shaderNames);
        void reset();
        bool link();

        // Specific program states
        void setTexture(
            GLenum activeTexUnit,
            GLenum target,
            GLuint id);
        void setTexture(
            GLenum activeTexUnit,
            GLenum target,
            const std::string& imgName);
        void setSubroutine(
            GLenum shaderType,
            const std::string& uniformName,
            const std::string& subroutineName);


        // Shader programs management
        static void pushProgram(GLuint id);
        static void pushFixedPipelineProgram();
        static unsigned int popProgram();
        static unsigned int currentProgramId();

        void pushProgram() const;
        bool isCurrentProgram() const;

        // Attributes
        int getAttributeLocation(const std::string& name) const;

        // Uniforms
        int getUniformLocation(const std::string& name) const;

        bool setInt(const std::string& var, int val) const;
        bool setVec2i(const std::string& var, const glm::ivec2& vec) const;
        bool setVec3i(const std::string& var, const glm::ivec3& vec) const;
        bool setVec4i(const std::string& var, const glm::ivec4& vec) const;

        bool setUnsigned(const std::string& var, unsigned int val) const;
        bool setVec2u(const std::string& var, const glm::uvec2& vec) const;
        bool setVec3u(const std::string& var, const glm::uvec3& vec) const;
        bool setVec4u(const std::string& var, const glm::uvec4& vec) const;

        bool setFloat(const std::string& var, float val) const;
        bool setVec2f(const std::string& var, const glm::vec2& vec) const;
        bool setVec3f(const std::string& var, const glm::vec3& vec) const;
        bool setVec4f(const std::string& var, const glm::vec4& vec) const;

        bool setMat3f(const std::string& var, const glm::mat3& mat) const;
        bool setMat4f(const std::string& var, const glm::mat4& mat) const;

        // Program Binary
        const GlProgramBinary& getBinary();


    private:
        void applyState() const;

        static std::stack<std::pair<GLuint, const GlProgram*> > _progStack;

        GLuint _id;
        int _linked;
        GlInputsOutputs _inAndOutLocations;
        std::set<std::shared_ptr<GlShader>> _shaders;
        std::map<GLenum, std::pair<GLenum, GLuint>> _textures;
        std::map<GLenum, std::vector<GLuint>> _subroutines;
        std::shared_ptr<GlProgramBinary> _binary;

        mutable std::map<std::string, int> _cachedLocations;
    };



    // IMPLEMENTATION //
    inline GLuint GlProgram::id() const
    {
        return _id;
    }

    inline const GlInputsOutputs& GlProgram::InOutLocations() const
    {
        return _inAndOutLocations;
    }
}

#endif // CELLARWORKBENCH_GLPROGRAM_H
