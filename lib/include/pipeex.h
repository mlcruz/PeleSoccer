// Simple High-level Inter-process Communications Library
// Copyright (c) 2011 HHD Software Ltd. http://www.hhdsoftware.com
// Written by Alexander Bessonov

// Distributed under the terms of The Code Project Open License (http://www.codeproject.com)

#pragma once

// You may override the following constant
#if !defined(PIPE_BUFFER_SIZE)
#define PIPE_BUFFER_SIZE 10240
#endif

// External dependencies
#if !defined(_SCL_SECURE_NO_WARNINGS)
#define _SCL_SECURE_NO_WARNINGS
#endif

// boost::preprocessor
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/comma_if.hpp>
#include <boost/preprocessor/punctuation/paren.hpp>
#include <boost/preprocessor/facilities/expand.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/rest_n.hpp>
#include <boost/preprocessor/seq/remove.hpp>
#include <boost/preprocessor/seq/fold_left.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/logical/and.hpp>

// boost::fusion
#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/functional/generation/make_fused.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/size.hpp>
#include <boost/fusion/include/mpl.hpp>

//#include <boost/fusion/include/make_vector.hpp>
//#include <boost/fusion/include/size.hpp>
//#include <boost/fusion/include/mpl.hpp>
//#include <boost/fusion/include/at.hpp>
//#include <boost/fusion/include/category_of.hpp>
//#include <boost/fusion/include/joint_view.hpp>
//
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/container/vector/convert.hpp>
#include <boost/fusion/support/is_sequence.hpp>
//
//#include <boost/fusion/adapted/mpl.hpp>
//
//

// boost::mpl
#include <boost/mpl/transform.hpp>
#include <boost/mpl/vector.hpp>

// boost::range
#include <boost/range/category.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

// STL
#include <vector>
#include <memory>
#include <array>
#include <string>

// Windows
#include <Windows.h>

// Implementation
#include "details/interface_decl.h"
#include "details/client.h"
#include "details/server.h"
