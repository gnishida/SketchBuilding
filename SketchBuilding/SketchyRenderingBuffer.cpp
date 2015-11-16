#include "SketchyRenderingBuffer.h"

SketchyRenderingBuffer::SketchyRenderingBuffer() {
}

/**
 * レンダリングバッファの初期化。
 * 本関数は、GLWidget3D::initializeGL()内で呼び出すこと。
 *
 * @param programId		シェイダーのprogram id
 * @param width			シャドウマッピングの幅
 * @param height		シャドウマッピングの高さ
 */
void SketchyRenderingBuffer::init(int programId, int textureNormalIndex, int textureDepthIndex, int width, int height) {
	this->programId = programId;
	this->width = width;
	this->height = height;

	// FBO作成
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		
	// normalバッファを保存するための2Dテクスチャを作成
	glGenTextures(1, &textureNormal);

	// GL_TEXTURE5に、このバッファをbindすることで、
	// シェーダからは5番でアクセスできる
	glActiveTexture(GL_TEXTURE0 + textureNormalIndex);
	glBindTexture(GL_TEXTURE_2D, textureNormal);

	// テクスチャパラメータの設定
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // テクスチャ領域の確保(GL_RGBAを用いる)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, 0);

	// 生成した2Dテクスチャを、デプスバッファとしてfboに括り付ける。
	// 以後、このfboに対するレンダリングを実施すると、デプスバッファのデータは
	// この2Dテクスチャに自動的に保存される。
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureNormal, 0);


	// depthバッファを保存するための2Dテクスチャを作成
	glGenTextures(1, &textureDepth);

	// GL_TEXTURE6に、このバッファをbindすることで、
	// シェーダからは6番でアクセスできる
	glActiveTexture(GL_TEXTURE0 + textureDepthIndex);
	glBindTexture(GL_TEXTURE_2D, textureDepth);

	// テクスチャパラメータの設定
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// テクスチャの外側、つまり、光源の外側は、影ってことにする(?)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);	// ただ、そもそも光源の外にならないよう、projection行列を設定すべき！
		
    // テクスチャ領域の確保(GL_DEPTH_COMPONENTを用いる)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	// 生成した2Dテクスチャを、デプスバッファとしてfboに括り付ける。
	// 以後、このfboに対するレンダリングを実施すると、デプスバッファのデータは
	// この2Dテクスチャに自動的に保存される。
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureDepth, 0);
	
	glActiveTexture(GL_TEXTURE0);
			
	// シェーダに、normalバッファとdepthバッファの番号を伝える
	glUniform1i(glGetUniformLocation(programId, "normalMap"), textureNormalIndex);
	glUniform1i(glGetUniformLocation(programId, "depthMap"), textureDepthIndex);

	glBindFramebuffer(GL_FRAMEBUFFER,0);
}

void SketchyRenderingBuffer::update(int width, int height) {
	this->width = width;
	this->height = height;

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		
	glBindTexture(GL_TEXTURE_2D, textureNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, 0);

	glBindTexture(GL_TEXTURE_2D, textureDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	glBindFramebuffer(GL_FRAMEBUFFER,0);
}

void SketchyRenderingBuffer::pass1() {
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glEnable(GL_TEXTURE_2D);

	glUniform1i(glGetUniformLocation(programId, "pass"), 1);

	//glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(1.1f, 4.0f);

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void SketchyRenderingBuffer::pass2() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glUniform1i(glGetUniformLocation(programId, "pass"), 2);
	glUniform1i(glGetUniformLocation(programId, "screenWidth"), width);
	glUniform1i(glGetUniformLocation(programId, "screenHeight"), height);

	glClearColor(1, 1, 1, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}