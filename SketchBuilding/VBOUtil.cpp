#include "VBOUtil.h"
#include "qimage.h"
#include <QGLWidget>

#define DEBUG_GL 0

#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049

void disaplay_memory_usage() {
	GLint total_mem_kb = 0;
	glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX, 
		&total_mem_kb);

	GLint cur_avail_mem_kb = 0;
	glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX, 
		&cur_avail_mem_kb);
	if (total_mem_kb>0)
		printf("GPU Memory Available %d of %d (%f%)\n",cur_avail_mem_kb,total_mem_kb,((float)cur_avail_mem_kb)/total_mem_kb);
}//


