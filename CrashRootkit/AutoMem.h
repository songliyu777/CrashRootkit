//--------------------------------------------------------------------
// �ļ���:		AutoMem.h
// ��  ��:		
// ˵  ��:		
// ��������:	2002��7��9��
// ������:		½����
// ��Ȩ����:	������ţ�������޹�˾
//--------------------------------------------------------------------

#ifndef _PUBLIC_AUTOMEM_H
#define _PUBLIC_AUTOMEM_H

// TAutoMem
// �Զ�������ڴ棬�ɸ�����Ҫʹ�þֲ�ջ���

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
