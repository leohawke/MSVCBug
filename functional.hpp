#ifndef F_H
#define F_H 1

#include <type_traits>
#include <tuple>
#include <utility>
#include <functional>

#define limpl(...) __VA_ARGS__
#define lforward(expr) std::forward<decltype(expr)>(expr)
#define lconstfn constexpr
#define lnoexcept noexcept
#define LPP_Comma ,

#pragma warning(disable:4003)

namespace sad {
	using std::decay_t;
	template<class _type>
	using _t = typename _type::type;
	using std::tuple_element_t;
	using std::is_object;
	using std::enable_if_t;
	using std::is_same;

	template<class _tClass, typename _tParam, typename _type = void>
	using exclude_self_t
		= enable_if_t<!is_same<_tClass&, decay_t<_tParam>&>::value, _type>;


#define LB_Impl_DeclIntT(_n, _t) \
	template<_t _vInt> \
	using _n = std::integral_constant<_t, _vInt>;
#define LB_Impl_DeclIntTDe(_t) LB_Impl_DeclIntT(_t##_, _t)
	LB_Impl_DeclIntTDe(size_t)

	template<typename...>
	struct or_;

	template<>
	struct or_<> : std::false_type
	{

	};

	template<typename _b1>
	struct or_<_b1> : _b1
	{
	};


	template<class _b1, class _b2, class... _bn>
	struct or_<_b1, _b2, _bn...>
		: std::conditional_t<_b1::value, _b1, or_<_b2, _bn...>>
	{};



	template<typename>
	struct make_parameter_tuple;

	template<typename _fCallable>
	using make_parameter_tuple_t = _t<make_parameter_tuple<_fCallable>>;

	template<typename _fCallable>
	struct make_parameter_tuple<_fCallable&> : make_parameter_tuple<_fCallable>
	{};

	template<typename _fCallable>
	struct make_parameter_tuple<_fCallable&&> : make_parameter_tuple<_fCallable>
	{};

#define LB_Impl_Functional_ptuple_spec(_exp, _p, _q) \
	template<typename _tRet, _exp typename... _tParams> \
	struct make_parameter_tuple<_tRet _p (_tParams...) _q> \
	{ \
		using type = std::tuple<_tParams...>; \
	};

	LB_Impl_Functional_ptuple_spec(, , )
		LB_Impl_Functional_ptuple_spec(, (*), )

#define LB_Impl_Functional_ptuple_spec_mf(_q) \
	LB_Impl_Functional_ptuple_spec(class _tClass LPP_Comma, (_tClass::*), _q)

		LB_Impl_Functional_ptuple_spec_mf()
		//@{
		LB_Impl_Functional_ptuple_spec_mf(const)
		LB_Impl_Functional_ptuple_spec_mf(volatile)
		LB_Impl_Functional_ptuple_spec_mf(const volatile)
		//@}

#undef LB_Impl_Functional_ptuple_spec_mf

#undef LB_Impl_Functional_ptuple_spec

		template<typename _tRet, typename... _tParams>
	struct make_parameter_tuple<std::function<_tRet(_tParams...)>>
	{
		using type = std::tuple<_tParams...>;
	};

	template<size_t _vIdx, typename _fCallable>
	struct parameter_of
	{
		using type = tuple_element_t<_vIdx,
			_t<make_parameter_tuple<_fCallable>>>;
	};

	template<size_t _vIdx, typename _fCallable>
	using parameter_of_t = _t<parameter_of<_vIdx, _fCallable>>;
	//@}


	/*!
	\ingroup metafunctions
	\brief 取参数列表大小。
	*/
	template<typename _fCallable>
	struct paramlist_size : size_t_<std::tuple_size<typename
		make_parameter_tuple<_fCallable>::type>::value>
	{};

	template<typename, class>
	struct make_function_type;

	template<typename _tRet, class _tTuple>
	using make_function_type_t = _t<make_function_type<_tRet, _tTuple>>;

	template<typename _tRet, typename... _tParams>
	struct make_function_type<_tRet, std::tuple<_tParams...>>
	{
		using type = _tRet(_tParams...);
	};

	//@{
	template<class, class>
	struct call_projection;

	template<typename _tRet, typename... _tParams, size_t... _vSeq>
	struct call_projection<_tRet(_tParams...), std::index_sequence<_vSeq...>>
	{
		template<typename _func>
		static lconstfn auto
			call(_func&& f, std::tuple<_tParams...>&& args, limpl(decay_t<
				decltype(lforward(f)(std::get<_vSeq>(std::move(args))...))>* = {}))
			-> decltype(lforward(f)(std::get<_vSeq>(std::move(args))...))
		{
			return lforward(f)(std::get<_vSeq>(lforward(args))...);
		}
		//@{
		template<typename _func>
		static lconstfn auto
			call(_func&& f, _tParams&&... args)
			-> decltype(call_projection::call(lforward(f),
				std::forward_as_tuple(lforward(args)...)))
		{
			return call_projection::call(lforward(f),
				std::forward_as_tuple(lforward(args)...));
		}

		template<typename _fCallable>
		static lconstfn auto
			invoke(_fCallable&& f, std::tuple<_tParams...>&& args,
				limpl(decay_t<decltype(std::invoke(lforward(f),
					std::get<_vSeq>(std::move(args))...))>* = {})) -> decltype(
						std::invoke(lforward(f), std::get<_vSeq>(lforward(args))...))
		{
			return std::invoke(lforward(f), std::get<_vSeq>(lforward(args))...);
		}
		template<typename _func>
		static lconstfn auto
			invoke(_func&& f, _tParams&&... args)
			-> decltype(call_projection::invoke(lforward(f),
				std::forward_as_tuple(lforward(args)...)))
		{
			return call_projection::invoke(lforward(f),
				std::forward_as_tuple(lforward(args)...));
		}
		//@}
	};

	template<typename _tRet, typename... _tParams, size_t... _vSeq>
	struct call_projection<std::function<_tRet(_tParams...)>,
		std::index_sequence<_vSeq...>> : private
		call_projection<_tRet(_tParams...), std::index_sequence<_vSeq...>>
	{
		using call_projection<_tRet(_tParams...), std::index_sequence<_vSeq...>>::call;
		using
			call_projection<_tRet(_tParams...), std::index_sequence<_vSeq...>>::invoke;
	};

	template<typename... _tParams, size_t... _vSeq>
	struct call_projection<std::tuple<_tParams...>, std::index_sequence<_vSeq...>>
	{
		template<typename _func>
		static lconstfn auto
			call(_func&& f, std::tuple<_tParams...>&& args)
			-> decltype(lforward(f)(std::get<_vSeq>(std::move(args))...))
		{
			return lforward(f)(std::get<_vSeq>(lforward(args))...);
		}

		//@{
		template<typename _func>
		static lconstfn auto
			call(_func&& f, _tParams&&... args)
			-> decltype(call_projection::call(lforward(f),
				std::forward_as_tuple(lforward(std::move(args))...)))
		{
			return call_projection::call(lforward(f),
				std::forward_as_tuple(lforward(lforward(args))...));
		}

		template<typename _fCallable>
		static lconstfn auto
			invoke(_fCallable&& f, std::tuple<_tParams...>&& args)
			-> decltype(std::invoke(lforward(f), std::get<_vSeq>(args)...))
		{
			return std::invoke(lforward(f), std::get<_vSeq>(args)...);
		}
		template<typename _func>
		static lconstfn auto
			invoke(_func&& f, _tParams&&... args)
			-> decltype(call_projection::invoke(lforward(f),
				std::forward_as_tuple(lforward(args)...)))
		{
			return call_projection::invoke(lforward(f),
				std::forward_as_tuple(lforward(args)...));
		}
		//@}
	};

	template<typename _fCallable, size_t _vLen = paramlist_size<_fCallable>::value>
	struct expand_proxy : private call_projection<_fCallable,
		std::make_index_sequence<_vLen>>, private expand_proxy<_fCallable, _vLen - 1>
	{
		/*!
		\see CWG 1393 。
		\see EWG 102 。
		*/
		using call_projection<_fCallable, std::make_index_sequence<_vLen>>::call;
		/*!
		\note 为避免歧义，不直接使用 using 声明。
		*/
		template<typename... _tParams>
		static auto
			call(_tParams&&... args) -> decltype(
				expand_proxy<_fCallable, _vLen - 1>::call(lforward(args)...))
		{
			return expand_proxy<_fCallable, _vLen - 1>::call(lforward(args)...);
		}
	};

	template<typename _fCallable>
	struct expand_proxy<_fCallable, 0>
		: private call_projection<_fCallable, std::index_sequence<>>
	{
		using call_projection<_fCallable, std::index_sequence<>>::call;
	};

	template<typename _fHandler, typename _fCallable>
	struct expanded_caller
	{
		static_assert(is_object<_fCallable>::value, "Callable object type is needed.");

		_fCallable caller;

		template<typename _fCaller,
			limpl(typename = exclude_self_t<expanded_caller, _fCaller>)>
			expanded_caller(_fCaller&& f)
			: caller(lforward(f))
		{}

		template<typename... _tParams>
		auto
			operator()(_tParams&&... args) const -> decltype(
				expand_proxy<_fHandler>::call(caller, lforward(args)...))
		{
			return expand_proxy<_fHandler>::call(caller, lforward(args)...);
		}
	};

	template<typename _fHandler, typename _fCallable>
	lconstfn expanded_caller<_fHandler, decay_t<_fCallable>>
		make_expanded(_fCallable&& f)
	{
		return expanded_caller<_fHandler, decay_t<_fCallable>>(lforward(f));
	}
}

#endif
