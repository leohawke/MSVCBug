#ifndef T_H
#define T_H 1

#include <type_traits>

namespace sad {
	using std::is_convertible;
	using std::true_type;
	using std::false_type;
	using std::void_t;
	using std::is_same;

	template<class _type>
	using _t = typename _type::type;

	/*!
	\brief 不接受任意参数类型构造的类型。
	\see WG21 N4502 6.6 。
	\since build 1.4
	*/
	struct nonesuch
	{
		nonesuch() = delete;
		nonesuch(const nonesuch&) = delete;
		~nonesuch() = delete;

		void
			operator=(const nonesuch&) = delete;
	};

	/*!
	\ingroup metafunctions
	\since build 1.4
	\see WG21 N4502 。
	*/
	//@{
	namespace details
	{

		template<typename _tDefault, typename, template<typename...> class, typename...>
		struct detector
		{
			using value_t = false_type;
			using type = _tDefault;
		};

		template<typename _tDefault, template<typename...> class _gOp,
			typename... _tParams>
			struct detector<_tDefault, void_t<_gOp<_tParams...>>, _gOp, _tParams...>
		{
			using value_t = true_type;
			using type = _gOp<_tParams...>;
		};

	} // namespace details;

	template<template<typename...> class _gOp, typename... _tParams>
	using is_detected
		= typename details::detector<nonesuch, void, _gOp, _tParams...>::value_t;

	template<template<typename...> class _gOp, typename... _tParams>
	using detected_t = _t<details::detector<nonesuch, void, _gOp, _tParams...>>;

	template<typename _tDefault, template<typename...> class _gOp,
		typename... _tParams>
		using detected_or = details::detector<_tDefault, void, _gOp, _tParams...>;

	template<typename _tDefault, template<typename...> class _gOp,
		typename... _tParams>
		using detected_or_t = _t<detected_or<_tDefault, _gOp, _tParams...>>;

	template<typename _tExpected, template<typename...> class _gOp,
		typename... _tParams>
		using is_detected_exact = is_same<_tExpected, detected_t<_gOp, _tParams...>>;

	template<typename _tTo, template<typename...> class _gOp, typename... _tParams>
	using is_detected_convertible
		= is_convertible<detected_t<_gOp, _tParams...>, _tTo>;
	//@}
}

#endif