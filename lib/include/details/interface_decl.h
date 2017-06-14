// Simple High-level Inter-process Communications Library
// Copyright (c) 2011 HHD Software Ltd. http://www.hhdsoftware.com
// Written by Alexander Bessonov

// Distributed under the terms of The Code Project Open License (http://www.codeproject.com)

#pragma once


#define F_GEN_HELPER_WITH_P(ret,name,params) ret name(BOOST_PP_SEQ_ENUM(params))
#define F_GEN_HELPER_VOID(ret,name) ret name()

#define F_GEN_HELPER(ret,name,params) \
	virtual BOOST_PP_IF(BOOST_PP_SEQ_SIZE(params), \
		F_GEN_HELPER_WITH_P(ret,name,params), \
		F_GEN_HELPER_VOID(ret,name) \
		) = 0;\
// end of macro

#define F_GEN(r,data,seq) F_GEN_HELPER(BOOST_PP_SEQ_ELEM(0,seq),BOOST_PP_SEQ_ELEM(1,seq),BOOST_PP_SEQ_REST_N(2,seq))
// end of macro

#define PIPE_GEN_INTERFACE(decl) \
class __declspec(novtable) BOOST_PP_SEQ_ELEM(0,decl) \
{ \
public: \
	BOOST_PP_SEQ_FOR_EACH(F_GEN,~,BOOST_PP_SEQ_REMOVE(decl,0)) \
} \
// end of macro

#define F_CL_PARAM(z,n,data) BOOST_PP_SEQ_ELEM(n,data) BOOST_PP_CAT(p,n)
#define F_CL_PARAM2(z,n,data) BOOST_PP_CAT(p,n)

#define F_CL_GEN_HELPER_WITH_P(i,ret,name,params) \
	virtual ret \
	name \
	(BOOST_PP_ENUM(BOOST_PP_SEQ_SIZE(params),F_CL_PARAM,params)) \
	{ \
		return static_cast<Derived *>(this)->ForwardCall<ret>(i,boost::fusion::vector<BOOST_PP_SEQ_ENUM(params)> \
			(BOOST_PP_ENUM(BOOST_PP_SEQ_SIZE(params),F_CL_PARAM2,~))); \
	} \
// end of macro

#define F_CL_GEN_HELPER_VOID(i,ret,name,params) \
	virtual ret \
	name() \
	{ \
		return static_cast<Derived *>(this)->ForwardCall<ret>(i); \
	} \
// end of macro

#define F_CL_GEN_HELPER(i,ret,name,params) \
	BOOST_PP_IF(BOOST_PP_SEQ_SIZE(params), \
		F_CL_GEN_HELPER_WITH_P, \
		F_CL_GEN_HELPER_VOID)(i,ret,name,params)
// end of macro

#define F_CL_GEN(r,data,i,seq) F_CL_GEN_HELPER(i,BOOST_PP_SEQ_ELEM(0,seq),BOOST_PP_SEQ_ELEM(1,seq),BOOST_PP_SEQ_REST_N(2,seq))
// end of macro

#define PIPE_GEN_CLIENT(decl) \
template<class Derived> \
class BOOST_PP_CAT(BOOST_PP_SEQ_ELEM(0,decl),Client) : public BOOST_PP_SEQ_ELEM(0,decl) \
{ \
public: \
	typedef BOOST_PP_SEQ_ELEM(0,decl) interface_type; \
	BOOST_PP_SEQ_FOR_EACH_I(F_CL_GEN,~,BOOST_PP_SEQ_REMOVE(decl,0)) \
} \
// end of macro

////////////////

namespace pipe_transport
{
	namespace _details
	{
		template<class _OrigTypes>
		struct calc
		{
			typedef typename boost::mpl::transform<_OrigTypes,std::decay<boost::mpl::_1>>::type _Types;
			typedef typename boost::fusion::result_of::as_vector<_Types>::type type;
		};
	}
}

#define F_SR_GEN_HELPER_WITH_P(i,ret,name,params) \
	case i: \
	{ \
		typedef typename pipe_transport::_details::calc<boost::mpl::vector<interface_type *,BOOST_PP_SEQ_ENUM(params)>>::type parameters_type; \
		parameters_type pars; \
		typedef typename bf::result_of::begin<parameters_type>::type A; \
		typedef typename bf::result_of::end<parameters_type>::type B; \
		typedef typename bf::result_of::next<A>::type C; \
		auto &&view=bf::iterator_range<C,B>(C(pars),B(pars)); \
		reader>>view; \
		bf::at_c<0>(pars)=static_cast<Derived *>(this)->GetInterface(); \
		static_cast<Derived *>(this)->SendResponse(bf::make_fused(&Derived::interface_type::name)(pars)); \
		break; \
	} \
// end of macro

#define F_SR_GEN_HELPER_VOID(i,ret,name,params) \
	case i: \
	{ \
		static_cast<Derived *>(this)->SendResponse(static_cast<Derived *>(this)->GetInterface()->name()); \
		break; \
	} \
// end of macro

#define F_SR_GEN_HELPER(i,ret,name,params) \
	BOOST_PP_IF(BOOST_PP_SEQ_SIZE(params), \
		F_SR_GEN_HELPER_WITH_P, \
		F_SR_GEN_HELPER_VOID)(i,ret,name,params)
// end of macro

#define F_SR_GEN(r,data,i,seq) F_SR_GEN_HELPER(i,BOOST_PP_SEQ_ELEM(0,seq),BOOST_PP_SEQ_ELEM(1,seq),BOOST_PP_SEQ_REST_N(2,seq))

#define PIPE_GEN_SERVER(decl) \
template<class Derived> \
class BOOST_PP_CAT(BOOST_PP_SEQ_ELEM(0,decl),Server) \
{ \
protected: \
	typedef BOOST_PP_SEQ_ELEM(0,decl) interface_type; \
	template<class Reader> \
	void Dispatch(int id,Reader &reader) \
	{ \
		namespace bf=boost::fusion; \
		switch (id) \
		{ \
			BOOST_PP_SEQ_FOR_EACH_I(F_SR_GEN,~,BOOST_PP_SEQ_REMOVE(decl,0)) \
		} \
	} \
} \
// end macro
