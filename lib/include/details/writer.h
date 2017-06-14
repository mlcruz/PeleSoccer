// Simple High-level Inter-process Communications Library
// Copyright (c) 2011 HHD Software Ltd. http://www.hhdsoftware.com
// Written by Alexander Bessonov

// Distributed under the terms of The Code Project Open License (http://www.codeproject.com)

#pragma once

#include "marshal.h"

namespace pipe_transport
{

namespace bf=boost::fusion;

class Writer
{
	typedef Writer thisClass;
	typedef std::vector<BYTE> Container;
	
	Container storage;
public:
	const Container &get() const
	{
		return storage;
	}

	template<class T>
	typename std::enable_if<std::is_fundamental<T>::value>::type
	operator ()(const T &val)
	{
		boost::push_back(storage,boost::make_iterator_range((BYTE *) &val,(BYTE *) (&val+1)));
	}

	template<class ConstIterator>
	void operator ()(ConstIterator begin,ConstIterator end,std::false_type)
	{
		for (;begin!=end;++begin)
			operator <<(*begin);
	}

	template<class ConstIterator>
	void operator ()(ConstIterator begin,ConstIterator end,std::true_type)
	{
		const size_t size=(end-begin)*sizeof(boost::iterator_value<ConstIterator>::type);
		auto cbegin=(const BYTE *) std::addressof(*begin);
		boost::push_back(storage,boost::make_iterator_range(cbegin,cbegin+size));
	}		

	template<class T>
	Writer &operator <<(const T &v)
	{
		marshal::Marshaller<T,typename marshal::tag<T>::type>::write(*this,v);
		return *this;
	}

};

}
