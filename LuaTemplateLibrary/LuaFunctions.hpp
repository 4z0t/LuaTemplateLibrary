#pragma once
#include "LuaAux.hpp"

namespace Lua
{
	template<auto fn, typename ...TUpvalues>
	struct CClosure
	{
		using FnType = decltype(fn);
		template<typename ...TArgs>
		using TReturn = typename std::invoke_result<FnType, TUpvalues&..., TArgs...>::type;

		template<typename ...TArgs>
		using ArgsTuple = std::tuple<TArgs...>;
		using Upvalues = std::tuple<TUpvalues...>;


	public:
		template<typename ...TArgs>
		static int Function(lua_State* l)
		{
			using namespace std;

			using Indexes = index_sequence_for<TArgs...>;
			static_assert(is_invocable<FnType, TUpvalues&..., TArgs...>::value, "Given function can't be called with such arguments!");
			using ArgsTupleT = ArgsTuple<TArgs...>;

			Upvalues upvalues;
			GetUpvalues<0, Upvalues, TUpvalues...>(l, upvalues);
			ArgsTupleT args;
			GetArgs<0, ArgsTupleT, TArgs ...>(l, args);
			if constexpr (is_void<TReturn<TArgs...>>::value)
			{
				CClosure::CallHelper(upvalues, args, index_sequence_for<TUpvalues...>{}, Indexes{});
				ReplaceUpvalues(l, upvalues);
				return 0;
			}
			else
			{
				TReturn<TArgs...> result = CClosure::CallHelper(upvalues, args, index_sequence_for<TUpvalues...>{}, Indexes{});
				ReplaceUpvalues(l, upvalues);
				size_t n_results = PushResult(l, result);
				return n_results;
			}
		}
	private:
		template <size_t ... UpIs, typename ...TArgs, size_t ... Is>
		inline static TReturn<TArgs...> CallHelper(Upvalues& upvalues, ArgsTuple<TArgs...>& args, std::index_sequence<UpIs...> const, std::index_sequence<Is...> const)
		{
			return fn(std::get<UpIs>(upvalues)..., std::get<Is>(args)...);
		}
	};

	template<auto fn>
	struct CFunction : public CClosure<fn> {
	};


	template<typename FnClass, typename ...TUpvalues>
	struct ClassClosure
	{
		template<typename ...TArgs>
		using TReturn = typename std::invoke_result<FnClass, TUpvalues&..., TArgs...>::type;

		template<typename ...TArgs>
		using ArgsTuple = std::tuple<TArgs...>;
		using Upvalues = std::tuple<TUpvalues...>;


	public:
		template<typename ...TArgs>
		static int Function(lua_State* l)
		{
			using namespace std;

			using Indexes = index_sequence_for<TArgs...>;
			static_assert(std::is_invocable<FnClass, TUpvalues&..., TArgs...>::value, "Given class doesnt provide callable or can't be called with such arguments!");
			using ArgsTupleT = ArgsTuple<TArgs...>;

			Upvalues upvalues;
			GetUpvalues<0, Upvalues, TUpvalues...>(l, upvalues);
			ArgsTupleT args;
			GetArgs<0, ArgsTupleT, TArgs ...>(l, args);
			if constexpr (is_void<TReturn<TArgs...>>::value)
			{
				ClassClosure::CallHelper(l, upvalues, args, index_sequence_for<TUpvalues...>{}, Indexes{});
				ReplaceUpvalues(l, upvalues);
				return 0;
			}
			else
			{
				TReturn<TArgs...> result = ClassClosure::CallHelper(l, upvalues, args, index_sequence_for<TUpvalues...>{}, Indexes{});
				ReplaceUpvalues(l, upvalues);
				size_t n_results = PushResult(l, result);
				return n_results;
			}
		}
	private:
		template <size_t ... UpIs, typename ...TArgs, size_t ... Is>
		inline static TReturn<TArgs...> CallHelper(lua_State* l, Upvalues& upvalues, ArgsTuple<TArgs...>& args, std::index_sequence<UpIs...> const, std::index_sequence<Is...> const)
		{

			if constexpr (std::is_constructible<FnClass, lua_State*>::value)
			{
				return FnClass{ l }(std::get<UpIs>(upvalues)..., std::get<Is>(args)...);
			}
			else
			{
				return FnClass{}(std::get<UpIs>(upvalues)..., std::get<Is>(args)...);
			}
		}
	};

	template<typename FnClass>
	struct ClassFunction : public ClassClosure<FnClass> {
	};
}