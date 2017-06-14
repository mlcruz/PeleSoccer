// Simple High-level Inter-process Communications Library
// Copyright (c) 2011 HHD Software Ltd. http://www.hhdsoftware.com
// Written by Alexander Bessonov

// Distributed under the terms of The Code Project Open License (http://www.codeproject.com)

#pragma once

#include "overlapped.h"
#include "reader.h"
#include "writer.h"
#include "pipebase.h"

namespace pipe_transport
{

namespace _details
{
	class OverBuffer
	{
		std::unique_ptr<BYTE[]> storage;
		size_t Allocated;

	public:
		OverBuffer() : Allocated(0)
		{
		}

		BYTE *Allocate(size_t Size)
		{
			if (Size>Allocated)
				storage.reset(new BYTE[Allocated=Size]);

			return storage.get();
		}

		BYTE *AllocateAndSave(size_t Size)
		{
			if (Size>Allocated)
			{
				std::unique_ptr<BYTE[]> newstorage(new BYTE[Size]);
				std::copy(storage.get(),storage.get()+Allocated,newstorage.get());
				std::swap(storage,newstorage);
				Allocated=Size;
			}

			return storage.get();
		}

		BYTE *GetBuffer() const
		{
			return storage.get();
		}

		size_t GetAllocatedSize() const
		{
			return Allocated;
		}
	};
}	// _details

template<template<class> class Impl>
class PipeServer : public Impl<PipeServer<Impl>>, public PipeBase
{
	typedef PipeServer thisClass;
	typedef Impl<thisClass> baseClass;
	typedef typename baseClass::interface_type Interface;
	friend class baseClass;

	Interface *pImpl;

	Interface *GetInterface()
	{
		return pImpl;
	}

	void Connect(const std::wstring &pipename,DWORD AdditionalFlags=0,LPSECURITY_ATTRIBUTES attr=nullptr)
	{
		auto &&PipeName=L"\\\\.\\pipe\\"+pipename;
		pipe.Attach(CreateNamedPipe(PipeName.c_str(),
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT | AdditionalFlags,1,
			10240,10240,0,attr));
	}

	bool WaitForClient(DWORD Timeout=INFINITE) throw()
	{
		DWORD dw;
		BOOL res=ConnectNamedPipe(pipe,&over);
		if (res)
			return true;
		switch (GetLastError())
		{
		case ERROR_PIPE_CONNECTED:
			return true;
		case ERROR_IO_PENDING:
			break;
		default:
			return false;
		}

		DWORD wait=WaitForSingleObject(over.hEvent,Timeout);
		if (wait==WAIT_TIMEOUT)
			return false;
		else
			return SUCCEEDED(pipe.GetOverlappedResult(&over,dw,FALSE));
	}

	template<class Range>
	bool ProcessMessage(const Range &range)
	{
		Reader<Range> reader(range);
		int id;
		reader>>id;
		if (id==-1)
			return false;
		else
		{
			Dispatch(id,reader);
			return true;
		}
	}

	template<class T>
	void SendResponse(const T &val)
	{
		Writer writer;
		writer<<val;

		Send(writer);
	}

public:
	PipeServer(Interface *pImpl_,const std::wstring &pipename,DWORD AdditionalFlags=0,LPSECURITY_ATTRIBUTES attr=nullptr) : pImpl(pImpl_)
	{
		Connect(pipename,AdditionalFlags,attr);
	}

	bool Operate(DWORD Timeout=INFINITE)
	{
		if (!WaitForClient(Timeout))
			return false;

		_details::OverBuffer buf;
		buf.Allocate(PIPE_BUFFER_SIZE);

		DWORD size=0,prevsize=0;
		while (true)
		{
again:
			HRESULT hr=pipe.Read((BYTE *) buf.GetBuffer()+prevsize,(DWORD) (buf.GetAllocatedSize()-prevsize),&over);

			if (!SUCCEEDED(hr))
			{
				while (true)
				{
					switch (hr)
					{
					case __HRESULT_FROM_WIN32(ERROR_IO_PENDING):
						WaitForSingleObject(over.hEvent,INFINITE);
						hr=pipe.GetOverlappedResult(&over,size,FALSE);

						if (FAILED(hr))
							continue;	// analyze an error again

						break;
					case __HRESULT_FROM_WIN32(ERROR_MORE_DATA):
						prevsize=size;
						buf.AllocateAndSave(2*buf.GetAllocatedSize());
						goto again;
					default:
						return false;	// client disconnected
					}
					break;
				}
			} else
				pipe.GetOverlappedResult(&over,size,FALSE);

			size+=prevsize;
			prevsize=0;
			if (SUCCEEDED(hr) && !ProcessMessage(boost::make_iterator_range((const BYTE *) buf.GetBuffer(),(const BYTE *) buf.GetBuffer()+size)))
				return true;
		}
	}
};

}
