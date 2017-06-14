// Simple High-level Inter-process Communications Library
// Copyright (c) 2011 HHD Software Ltd. http://www.hhdsoftware.com
// Written by Alexander Bessonov

// Distributed under the terms of The Code Project Open License (http://www.codeproject.com)

#pragma once

#include "overlapped.h"
#include "writer.h"
#include "reader.h"
#include "pipebase.h"

namespace pipe_transport
{

template<template<class> class InterfaceImpl>
class PipeClient : public InterfaceImpl<PipeClient<InterfaceImpl>>, public PipeBase
{
	friend class InterfaceImpl<PipeClient>;
	typedef std::array<BYTE,PIPE_BUFFER_SIZE> IncomingType;
	typedef IncomingType::iterator iterator;
	typedef boost::iterator_range<iterator> Range;

	IncomingType Incoming;

	template<class result_type>
	result_type ForwardCall(const Writer &writer)
	{
		auto range=Transact(writer);
		result_type ret;
		range>>ret;
		return ret;
	}

	template<class result_type>
	result_type ForwardCall(int id)
	{
		Writer writer;
		writer<<id;
		return ForwardCall<result_type>(writer);
	}

	template<class result_type,class Seq>
	result_type ForwardCall(int id,const Seq &seq)
	{
		Writer writer;
		writer<<id<<seq;
		return ForwardCall<result_type>(writer);
	}

	Reader<Range> Transact(const Writer &writer)
	{
		const auto &range=writer.get();

		DWORD dw;
		BOOL result=TransactNamedPipe(pipe,(void *) std::addressof(*boost::begin(range)),(DWORD) boost::size(range),Incoming.data(),(DWORD) Incoming.size(),&dw,&over);
		if (!result)
		{
			if (GetLastError()==ERROR_IO_PENDING)
				pipe.GetOverlappedResult(&over,dw,TRUE);
		}
		return Reader<Range>(boost::make_iterator_range(Incoming.begin(),Incoming.begin()+dw));
	}

public:
	~PipeClient()
	{
		if (pipe)
		{
			int id=-1;
			Writer writer;
			writer<<id;

			Send(writer);
		}
	}
	
	bool Connect(const std::wstring &pipename_,DWORD Timeout=10000,const std::wstring &servername_=L".")
	{
		auto &&pipename=L"\\\\"+servername_+L"\\pipe\\"+pipename_;
		HRESULT hr=pipe.Create(pipename.c_str(),
			GENERIC_READ | GENERIC_WRITE,0,OPEN_EXISTING,FILE_FLAG_OVERLAPPED);
		if (FAILED(hr))
		{
			if (hr==HRESULT_FROM_WIN32(ERROR_PIPE_BUSY))
			{
				if (!WaitNamedPipe(pipename.c_str(),Timeout))
					return false;
				else
					return Connect(pipename_);
			} else
				return false;
		}
		DWORD dwMode = PIPE_READMODE_MESSAGE | PIPE_WAIT;
		return SetNamedPipeHandleState(pipe,&dwMode,NULL,NULL)!=FALSE;
	}
};

}
