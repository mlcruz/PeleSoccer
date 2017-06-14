// Simple High-level Inter-process Communications Library
// Copyright (c) 2011 HHD Software Ltd. http://www.hhdsoftware.com
// Written by Alexander Bessonov

// Distributed under the terms of The Code Project Open License (http://www.codeproject.com)

#pragma once

#include <atlfile.h>

#include "overlapped.h"
#include "writer.h"

namespace pipe_transport
{

class PipeBase
{
protected:
	ATL::CAtlFile pipe;
	COverlapped over;

	void Send(const Writer &writer)
	{
		const auto &range=writer.get();
		pipe.Write(std::addressof(*boost::begin(range)),(DWORD) boost::size(range),&over);
		DWORD size;
		pipe.GetOverlappedResult(&over,size,TRUE);
	}
};

}
