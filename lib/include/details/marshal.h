// Simple High-level Inter-process Communications Library
// Copyright (c) 2011 HHD Software Ltd. http://www.hhdsoftware.com
// Written by Alexander Bessonov

// Distributed under the terms of The Code Project Open License (http://www.codeproject.com)

#pragma once

namespace pipe_transport { namespace marshal {

struct no_tag {};
struct stream_tag {};

template<class T>
struct tag
{
	typedef no_tag type;
};

template<class T,class Tag=no_tag,typename Enable=void>
struct Marshaller;

template<class T>
struct Marshaller<T,stream_tag>
{
	template<typename Writer>
	static void write(Writer &writer,const T &v)
	{
		v.marshal(writer);
	}

	template<typename Reader>
	static void read(Reader &reader,T &v)
	{
		v.unmarshal(reader);
	}
};

// fundamental types
template<class T>
struct Marshaller<T,no_tag,typename std::enable_if<std::is_fundamental<T>::value>::type>
{
	template<typename Writer>
	static void write(Writer &writer,const T &v)
	{
		writer(v);
	}

	template<typename Reader>
	static void read(Reader &reader,T &v)
	{
		reader(v);
	}
};

// ranges (including arrays and containers + special case for strings)
template<class Range>
struct Marshaller<Range,no_tag,typename std::enable_if<
	!boost::fusion::traits::is_sequence<Range>::type::value &&
	!std::is_fundamental<Range>::value>::type>
{
	template<typename Writer,class Range>
	static void write_range(Writer &writer,const Range &range,std::random_access_iterator_tag)
	{
		writer((WORD) boost::size(range));
		writer(boost::begin(range),boost::end(range),std::is_fundamental<boost::range_value<Range>::type>());
	}

	template<typename Writer>
	static void write(Writer &writer,const Range &range)
	{
		write_range(writer,range,boost::range_category<Range>::type());
	}

	// strings
	template<typename Writer,class Elem,class Traits,class Ax>
	static void write(Writer &writer,const std::basic_string<Elem,Traits,Ax> &str)
	{
		assert(str.length()<65536);
		writer((WORD) str.length());
		writer(str.data(),str.data()+str.length(),std::true_type());
	}

	template<typename Reader>
	static void read(Reader &reader,Range &container)
	{
		typedef typename boost::range_value<Range>::type value_type;
		WORD count;
		reader(count);
		value_type v;
		while (count--)
		{
			reader>>v;
			container.push_back(std::move(v));
		}
	}
	// strings
	template<typename Reader,class Elem,class Traits,class Ax>
	static void read(Reader &reader,std::basic_string<Elem,Traits,Ax> &str)
	{
		WORD length;
		reader(length);
		str.resize(length);
		reader(str.begin(),str.end(),std::true_type());
	}
};

// fusion sequences (or adapted structures)
template<class Seq>
struct Marshaller<Seq,no_tag,typename std::enable_if<boost::fusion::traits::is_sequence<Seq>::type::value>::type>
{
	template<class Writer>
	struct w_dispatcher : private boost::noncopyable
	{
		Writer &writer;

		w_dispatcher(Writer &writer_) : writer(writer_)
		{
		}

		template<class T>
		void operator()(const T &value) const
		{
			writer<<value;
		}
	};

	template<class Reader>
	struct r_dispatcher : private boost::noncopyable
	{
		Reader &reader;

		r_dispatcher(Reader &reader_) : reader(reader_)
		{
		}

		template<class T>
		void operator()(T &value) const
		{
			reader>>value;
		}
	};

	template<typename Writer>
	static void write(Writer &writer,const Seq &seq)
	{
		boost::fusion::for_each(seq,w_dispatcher<Writer>(writer));
	}

	// fusion sequences
	template<typename Reader>
	static void read(Reader &reader,Seq &seq)
	{
		boost::fusion::for_each(seq,r_dispatcher<Reader>(reader));
	}
};

} }

