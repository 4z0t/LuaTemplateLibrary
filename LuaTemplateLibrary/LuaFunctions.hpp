#pragma once
#include "LuaAux.hpp"

namespace Lua
{



	template<typename FnClass, typename ...TArgs>
	struct FunctionWrapper
	{
		static_assert(std::is_invocable<FnClass, TArgs...>::value, "Given class doesnt provide callable or can't be called with such arguments!");
		using TReturn = typename std::invoke_result<FnClass, TArgs...>::type;
		using ArgsTupleT = std::tuple<TArgs...>;
		using Indexes = std::index_sequence_for<TArgs...>;

	public:
		static int Function(lua_State* l)
		{
			using namespace std;
			ArgsTupleT args;
			GetArgs<0, ArgsTupleT, TArgs ...>(l, args);

			if constexpr (std::is_void<TReturn>::value)
			{
				FunctionWrapper::CallHelper(l, args, Indexes{});
				return 0;
			}
			else
			{
				TReturn result = FunctionWrapper::CallHelper(l, args, Indexes{});
				size_t n_results = PushResult(l, result);
				return n_results;
			}
		}
	private:
		template <std::size_t ... Is>
		static TReturn CallHelper(lua_State* l, ArgsTupleT& args, std::index_sequence<Is...> const)
		{
			if constexpr (std::is_constructible<FnClass, lua_State*>::value)
			{
				return FnClass{ l }(std::get<Is>(args)...);
			}
			else
			{
				return FnClass{}(std::get<Is>(args)...);
			}
		}
	};


	template<auto fn, typename ...TArgs>
	struct CFunctionWrapper
	{
		using FnType = decltype(fn);
		static_assert(std::is_invocable<FnType, TArgs...>::value, "Given function can't be called with such arguments!");
		using TReturn = typename std::invoke_result<FnType, TArgs...>::type;
		using ArgsTupleT = std::tuple<TArgs...>;
		using Indexes = std::index_sequence_for<TArgs...>;

	public:
		static int Function(lua_State* l)
		{
			using namespace std;
			ArgsTupleT args;
			GetArgs<0, ArgsTupleT, TArgs ...>(l, args);
			if constexpr (std::is_void<TReturn>::value)
			{
				CFunctionWrapper::CallHelper(args, Indexes{});
				return 0;
			}
			else
			{
				TReturn result = CFunctionWrapper::CallHelper(args, Indexes{});
				size_t n_results = PushResult(l, result);
				return n_results;
			}
		}
	private:
		template <std::size_t ... Is>
		static TReturn CallHelper(ArgsTupleT& args, std::index_sequence<Is...> const)
		{
			return fn(std::get<Is>(args)...);
		}
	};



	template<typename ...CArgs>
	void ReplaceUpvalues(lua_State* l, std::tuple<CArgs...>& upvalues)
	{
		_ReplaceUpvalue<0, std::tuple<CArgs...>, CArgs...>(l, upvalues);
	}

	template<auto fn, template<typename ...CArgs> typename Upvalues, typename ...TArgs>
	struct CClosureWrapper
	{
		using FnType = decltype(fn);
		template<typename ...CArgs>
		using TReturn = typename std::invoke_result<FnType, Upvalues<CArgs...>&, TArgs...>::type;
		using ArgsTupleT = std::tuple<TArgs...>;
		using Indexes = std::index_sequence_for<TArgs...>;

	public:
		template<typename ...CArgs>
		static int Function(lua_State* l)
		{
			using namespace std;
			static_assert(std::is_invocable<FnType, Upvalues<CArgs...>&, TArgs...>::value, "Given function can't be called with such arguments!");

			Upvalues<CArgs...> upvalues;
			GetUpvalues<0, Upvalues<CArgs...>, CArgs...>(l, upvalues);
			ArgsTupleT args;
			GetArgs<0, ArgsTupleT, TArgs ...>(l, args);
			if constexpr (std::is_void<TReturn<CArgs...>>::value)
			{
				CClosureWrapper::CallHelper(upvalues, args, Indexes{});
				ReplaceUpvalues(l, upvalues);
				return 0;
			}
			else
			{
				TReturn<CArgs...> result = CClosureWrapper::CallHelper(upvalues, args, Indexes{});
				ReplaceUpvalues(l, upvalues);
				size_t n_results = PushResult(l, result);
				return n_results;
			}
		}
	private:
		template <std::size_t ... Is, typename ...CArgs>
		static TReturn<CArgs...> CallHelper(Upvalues<CArgs...>& upvalues, ArgsTupleT& args, std::index_sequence<Is...> const)
		{
			return fn(upvalues, std::get<Is>(args)...);
		}
	};


	template<auto fn, typename ...TArgs>
	struct ClosureWrapper
	{
		using FnType = decltype(fn);
		template<typename ...CArgs>
		using TReturn = typename std::invoke_result<FnType, CArgs&..., TArgs...>::type;
		using ArgsTupleT = std::tuple<TArgs...>;
		using Indexes = std::index_sequence_for<TArgs...>;

	public:
		template<typename ...CArgs>
		static int Function(lua_State* l)
		{
			using namespace std;
			static_assert(std::is_invocable<FnType, CArgs&..., TArgs...>::value, "Given function can't be called with such arguments!");
			using Upvalues = std::tuple<CArgs...>;

			Upvalues upvalues;
			GetUpvalues<0, Upvalues, CArgs...>(l, upvalues);
			ArgsTupleT args;
			GetArgs<0, ArgsTupleT, TArgs ...>(l, args);
			if constexpr (std::is_void<TReturn<CArgs...>>::value)
			{
				ClosureWrapper::CallHelper(upvalues, args, std::index_sequence_for<CArgs...>{}, Indexes{});
				ReplaceUpvalues(l, upvalues);
				return 0;
			}
			else
			{
				TReturn<CArgs...> result = ClosureWrapper::CallHelper(upvalues, args, std::index_sequence_for<CArgs...>{}, Indexes{});
				ReplaceUpvalues(l, upvalues);
				size_t n_results = PushResult(l, result);
				return n_results;
			}
		}
	private:
		template <std::size_t ... UpIs, typename ...CArgs, std::size_t ... Is>
		static TReturn<CArgs...> CallHelper(std::tuple<CArgs...>& upvalues, ArgsTupleT& args, std::index_sequence<UpIs...> const, std::index_sequence<Is...> const)
		{
			return fn(std::get<UpIs>(upvalues)..., std::get<Is>(args)...);
		}
	};
}