//--------------------------------------------------------------------
// 文件名:		AutoMem.h
// 内  容:		
// 说  明:		
// 创建日期:	2002年7月9日
// 创建人:		陆利民
// 版权所有:	苏州蜗牛电子有限公司
//--------------------------------------------------------------------

#ifndef _PUBLIC_AUTOMEM_H
#define _PUBLIC_AUTOMEM_H

// TAutoMem
// 自动管理的内存，可根据需要使用局部栈或堆

template<typename TYPE, size_t SIZE>
class TAutoMem
{
public:
	explicit TAutoMem(size_t len)
	{
		if (len > SIZE)
		{
			m_pMem = new TYPE[len];
		}
		else
		{
			m_pMem = m_stack;
		}
	}

	~TAutoMem()
	{
		if (m_pMem != m_stack)
		{
			delete[] m_pMem;
		}
	}

	TYPE* GetBuffer()
	{
		return m_pMem;
	}

private:
	TAutoMem();
	TAutoMem(const TAutoMem&);
	TAutoMem& operator=(const TAutoMem&);

private:
	TYPE m_stack[SIZE];
	TYPE* m_pMem;
};

#endif // _PUBLIC_AUTOMEM_H
