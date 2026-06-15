#ifndef BASE_FILTER_H
#define BASE_FILTER_H

#include <map>
#include <string>

#include "../../VideoRenderInfo.h"
#include "../common/ColorSpaceConvert.h"
#include "../common/OpenglContext.h"
#include "./ShaderFactory.h"

typedef struct BackgroundColor {
    float r;
    float g;
    float b;
    float a;
} BackgroundColor;


class BaseFilter {
public:
    BaseFilter();

    virtual ~BaseFilter();

    virtual int init(VideoFrameMetaData *inputFrameMetaData, std::shared_ptr<OpenGLContext> context);

    virtual bool initWithFragmentShader(const std::string &fragmentShader, int inputTexNum);

    bool initWithShader(const std::string &vertexShader, const std::string &fragmentShader);

    void setInputFrameMetaData(VideoFrameMetaData *inputFrameMetaData);

    void setInputTexture(GLuint textureId, int index);

    virtual void updateParam();

    virtual int onRender();

    int getPixelFormat() const;

protected:

    const GLfloat *getVertexCoordinate();

    const GLfloat *getTextureCoordinate(const RotationMode &rotationMode);

    void defaultVertexCoordinate();

    void updateTextureCoordinate(const RotationMode &rotationMode);

    void cropTextureCoordinate(RotationMode rotationMode, GLfloat cropSize);

protected:

    std::shared_ptr<OpenGLContext> mGLContext;

    VideoFrameMetaData *mInputFrameData{nullptr};

    BackgroundColor mBackgroundColor{0.0, 0.0, 0.0, 1.0};

    int mPixelFormat   = 0;
    int mInputTexNum   = 0;
    int mCurrentWidth  = 0;
    int mCurrentHeight = 0;

    float mTextureScale = 1.0f;
    GLsizei mPaddingPixels{0};
    GLfloat mVertexCoordinate[8]{};
    GLfloat mTextureCoordinate[8]{};

    GLuint mProgram{};
    GLuint mTextureId     = -1;
    GLuint mTextures[3]   = {0};
    GLuint mFrameBufferId = -1;
    GLuint mPositionAttribute{};

};

#endif //BASE_FILTER_H
