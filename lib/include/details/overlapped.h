// Simple High-level Inter-process Communications Library
// Copyright (c) 2011 HHD Software Ltd. http://www.hhdsoftware.com
// Written by Alexander Bessonov

// Distributed under the terms of The Code Project Open License (http://www.codeproject.com)

#pragma once

namespace pipe_transport
{

class COverlapped : public OVERLAPPED
{
public:
	COverlapped() : OVERLAPPED()
	{
		hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);
	}

	~COverlapped()
	{
		CloseHandle(hEvent);
	}
};

}
