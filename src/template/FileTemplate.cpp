#include "FileTemplate.h"
#include "File.h"
#include "ByteBuffer.h"

namespace bluemei{

FileTemplate::FileTemplate(const String& path)
{
	File file(path);

	//StringBuilder sb(file.getSize());
	ByteBuffer buf(file.getSize());
	file.readAll((char*)buf.array(), buf.size());

	m_tpl = new StringTemplate(String((cstring)buf.array(), buf.size()));
}

FileTemplate::~FileTemplate(void)
{
	delete m_tpl;
}

String FileTemplate::render(const TemplateCtx& ctx)
{
	checkNullPtr(m_tpl);
	return m_tpl->render(ctx);
}

}