/////////////////////////////////////////////////////////////////////////
// MD5.cpp
// Implementation file for MD5 class
//
// This C++ Class implementation of the original RSA Data Security, Inc.
// MD5 Message-Digest Algorithm is copyright (c) 2002, Gary McNickle.
// All rights reserved.  This software is a derivative of the "RSA Data
//  Security, Inc. MD5 Message-Digest Algorithm"
//
// You may use this software free of any charge, but without any
// warranty or implied warranty, provided that you follow the terms
// of the original RSA copyright, listed below.
//
// Original RSA Data Security, Inc. Copyright notice
/////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
// rights reserved.
//
// License to copy and use this software is granted provided that it
// is identified as the "RSA Data Security, Inc. MD5 Message-Digest
// Algorithm" in all material mentioning or referencing this software
// or this function.
// License is also granted to make and use derivative works provided
// that such works are identified as "derived from the RSA Data
// Security, Inc. MD5 Message-Digest Algorithm" in all material
// mentioning or referencing the derived work.
// RSA Data Security, Inc. makes no representations concerning either
// the merchantability of this software or the suitability of this
// software for any particular purpose. It is provided "as is"
// without express or implied warranty of any kind.
// These notices must be retained in any copies of any part of this
// documentation and/or software.
/////////////////////////////////////////////////////////////////////////
#ifndef _MD5_API_
#define _MD5_API_

//##ModelId=3FEBE1C8035B
typedef unsigned       int uint4;
//##ModelId=3FEBE1C8036F
typedef unsigned short int uint2;
//##ModelId=3FEBE1C8038D
typedef unsigned      char uchar;

char* PrintMD5(uchar md5Digest[16]);
char* MD5String(char* szString);
char* MD5File(char* szFilename);
char* MD5Buffer(PBYTE buff,SIZE_T size);

//##ModelId=3FEBE1C803BF
class md5
{
// Methods
public:
	//##ModelId=3FEBE1C803D3
	md5() { Init(); }
	//##ModelId=3FEBE1C803D4
	void	Init();
	//##ModelId=3FEBE1C803DD
	void	Update(uchar* chInput, uint4 nInputLen);
	//##ModelId=3FEBE1C90009
	void	Finalize();
	//##ModelId=3FEBE1C90013
	uchar*	Digest() { return m_Digest; }

private:

	//##ModelId=3FEBE1C90014
	void	Transform(uchar* block);
	//##ModelId=3FEBE1C90027
	void	Encode(uchar* dest, uint4* src, uint4 nLength);
	//##ModelId=3FEBE1C90045
	void	Decode(uint4* dest, uchar* src, uint4 nLength);


	//##ModelId=3FEBE1C9005A
	inline	uint4	rotate_left(uint4 x, uint4 n)
	                 { return ((x << n) | (x >> (32-n))); }

	//##ModelId=3FEBE1C9006E
	inline	uint4	F(uint4 x, uint4 y, uint4 z)
	                 { return ((x & y) | (~x & z)); }

	//##ModelId=3FEBE1C9008C
	inline  uint4	G(uint4 x, uint4 y, uint4 z)
	                 { return ((x & z) | (y & ~z)); }

	//##ModelId=3FEBE1C900A9
	inline  uint4	H(uint4 x, uint4 y, uint4 z)
	                 { return (x ^ y ^ z); }

	//##ModelId=3FEBE1C900C7
	inline  uint4	I(uint4 x, uint4 y, uint4 z)
	                 { return (y ^ (x | ~z)); }

	//##ModelId=3FEBE1C900DC
	inline	void	FF(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
	                 { a += F(b, c, d) + x + ac; a = rotate_left(a, s); a += b; }

	//##ModelId=3FEBE1C9010D
	inline	void	GG(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
                     { a += G(b, c, d) + x + ac; a = rotate_left(a, s); a += b; }

	//##ModelId=3FEBE1C90137
	inline	void	HH(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
                     { a += H(b, c, d) + x + ac; a = rotate_left(a, s); a += b; }

	//##ModelId=3FEBE1C90167
	inline	void	II(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
                     { a += I(b, c, d) + x + ac; a = rotate_left(a, s); a += b; }

// Data
private:
	//##ModelId=3FEBE1C901A4
	uint4		m_State[4];
	//##ModelId=3FEBE1C901C2
	uint4		m_Count[2];
	//##ModelId=3FEBE1C901E1
	uchar		m_Buffer[64];
	//##ModelId=3FEBE1C901FF
	uchar		m_Digest[16];
	//##ModelId=3FEBE1C9021D
	uchar		m_Finalized;

};

#endif
