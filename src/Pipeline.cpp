#include "Pipeline.hpp"

#include <format>
#include <fstream>
#include <print>
#include <sstream>

//------------------------------------------------------------------------

namespace Zhade
{

//------------------------------------------------------------------------

Pipeline::Pipeline(PipelineDescriptor desc)
    : m_headers{std::move(desc.headers)},
      m_managed{desc.managed}
{
    glCreateProgramPipelines(1, &m_name);

    setupHeaders();

    setupStageProgram(PipelineStage::VERTEX, desc.vertPath);
    setupStageProgram(PipelineStage::FRAGMENT, desc.fragPath);

    if (!desc.geomPath.empty())
        setupStageProgram(PipelineStage::GEOMETRY, desc.geomPath);

    validate();
}

//------------------------------------------------------------------------

Pipeline::~Pipeline()
{
    if (m_managed) [[likely]] return;
    freeResources();
}

//------------------------------------------------------------------------

void Pipeline::freeResources() const noexcept
{
    glDeleteProgramPipelines(1, &m_name);
    glDeleteProgram(m_stages[PipelineStage::VERTEX]);
    glDeleteProgram(m_stages[PipelineStage::FRAGMENT]);
    glDeleteProgram(m_stages[PipelineStage::GEOMETRY]);
    for (const auto& header : m_headers)
        glDeleteNamedStringARB(header.size(), header.c_str());
}

//------------------------------------------------------------------------

std::string Pipeline::readFileContents(const fs::path& path) const noexcept
{
    std::ifstream file{path};
    if (file.bad()) [[unlikely]]
    {
        std::println("Error reading shader from '{}'", path.string());
        return "";
    }
    std::ostringstream osstream;
    osstream << file.rdbuf();
    return osstream.str();
}

//------------------------------------------------------------------------

GLuint Pipeline::createShaderProgram(PipelineStage::Type stage, const std::string& shaderSource) const noexcept
{
    const GLuint shader = glCreateShader(PipelineStage2GLShader[stage]);

    const char* shaderSourceRaw = shaderSource.c_str();
    glShaderSource(shader, 1, &shaderSourceRaw, nullptr);
    static const GLchar* shaderVirtualPath[] = { "/" };
    glCompileShaderIncludeARB(shader, 1, shaderVirtualPath, nullptr);

    const GLuint program = glCreateProgram();

    GLint compileStatus = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    glProgramParameteri(program, GL_PROGRAM_SEPARABLE, GL_TRUE);
    if (compileStatus == GL_FALSE) [[unlikely]]
    {
        GLchar infoLog[LOCAL_CHAR_BUF_SIZE];
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        std::println("Error compiling shader with ID {}: {}", program, infoLog);
        return 0;
    }

    glAttachShader(program, shader);
    glLinkProgram(program);
    glDetachShader(program, shader);
    glDeleteShader(shader);
    GLint linkStatus = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE) [[unlikely]]
    {
        GLchar infoLog[LOCAL_CHAR_BUF_SIZE];
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        std::println("Error linking shader with ID {}: {}", program, infoLog);
        return 0;
    }

    return program;
}

//------------------------------------------------------------------------

 void Pipeline::setupHeaders() const noexcept
 {
    for (const auto& header : m_headers)
    {
        const fs::path path = SOURCE_PATH / fs::path{header}.filename();
        const std::string contents = readFileContents(path);
        glNamedStringARB(GL_SHADER_INCLUDE_ARB, header.size(), header.c_str(), contents.size(), contents.c_str());
    }
 }

//------------------------------------------------------------------------

void Pipeline::setupStageProgram(PipelineStage::Type stage, const fs::path& shaderPath) const noexcept
{
    m_stages[stage] = createShaderProgram(stage, readFileContents(shaderPath));
    glUseProgramStages(m_name, PipelineStage2GLShaderBit[stage], m_stages[stage]);
}

//------------------------------------------------------------------------

void Pipeline::validate() const noexcept
{
    glValidateProgramPipeline(m_name);

    GLint status = GL_FALSE;
    glGetProgramPipelineiv(m_name, GL_VALIDATE_STATUS, &status);
    if (status == GL_FALSE) [[unlikely]]
    {
        GLchar infoLog[LOCAL_CHAR_BUF_SIZE];
        glGetProgramPipelineInfoLog(m_name, sizeof(infoLog), nullptr, infoLog);
        std::println("Error validating pipeline with ID {}: {}", m_name, infoLog);
    }
}

//------------------------------------------------------------------------

}  // namespace Zhade

//------------------------------------------------------------------------
