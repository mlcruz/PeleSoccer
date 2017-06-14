// Simple High-level Inter-process Communications Library
// Copyright (c) 2011 HHD Software Ltd. http://www.hhdsoftware.com
// Written by Alexander Bessonov

// Distributed under the terms of The Code Project Open License (http://www.codeproject.com)

#pragma once

#include "marshal.h"

namespace pipe_transport
{

template<class Range>
class Reader : boost::noncopyable
{
	typedef Reader thisClass;
	typedef typename boost::range_iterator<Range>::type iterator;
	const Range &range;
	iterator it;

public:
	Reader(const Range &range_) : range(range_)
	{
		it=boost::begin(range);
	}

	template<class T>
	typename std::enable_if<std::is_fundamental<T>::value>::type
	operator ()(T &val)
	{
		std::copy(it,it+sizeof(T),(BYTE *) &val);
		it+=sizeof(T);
	}

	template<class Iterator>
	void operator ()(Iterator begin,Iterator end,std::false_type)
	{
		for (;begin!=end;++begin)
			operator >>(*begin);
	}

	template<class Iterator>
	void operator ()(Iterator begin,Iterator end,std::true_type)
	{
		const size_t size=(end-begin)*sizeof(boost::iterator_value<Iterator>::type);
		if (size)
		{
			std::copy(it,it+size,(BYTE *) std::addressof(*begin));
			it+=size;
		}
	}		

	template<class T>
	Reader &operator >>(T &v)
	{
		marshal::Marshaller<T,typename marshal::tag<T>::type>::read(*this,v);
		return *this;
	}

};

}
