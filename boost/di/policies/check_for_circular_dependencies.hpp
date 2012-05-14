//
// Copyright (c) 2012 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#if !BOOST_PP_IS_ITERATING

    #ifndef BOOST_DI_POLICIES_CHECK_FOR_CIRCULAR_DEPENDENCIES_HPP
    #define BOOST_DI_POLICIES_CHECK_FOR_CIRCULAR_DEPENDENCIES_HPP

    #include <boost/preprocessor/iteration/iterate.hpp>
    #include <boost/preprocessor/repetition/repeat.hpp>
    #include <boost/preprocessor/control/expr_if.hpp>
    #include <boost/preprocessor/punctuation/comma_if.hpp>
    #include <boost/utility/enable_if.hpp>
    #include <boost/mpl/count.hpp>
    #include <boost/mpl/int.hpp>
    #include <boost/mpl/plus.hpp>
    #include <boost/mpl/bool.hpp>
    #include <boost/mpl/accumulate.hpp>
    #include <boost/mpl/transform.hpp>
    #include <boost/mpl/size.hpp>
    #include <boost/mpl/at.hpp>
    #include <boost/mpl/assert.hpp>
    #include "boost/di/aux/ctor_traits.hpp"
    #include "boost/di/aux/dependency.hpp"
    #include "boost/di/aux/make_plain.hpp"
    #include "boost/di/detail/binder.hpp"
    #include "boost/di/scopes/per_request.hpp"
    #include "boost/di/config.hpp"

    #define BOOST_PP_ITERATION_PARAMS_1 (                               \
        BOOST_DI_ITERATION_PARAMS(                                      \
            0                                                           \
          , BOOST_DI_FUNCTION_ARITY_LIMIT_SIZE                          \
          , "boost/di/policies/check_for_circular_dependencies.hpp"     \
        )                                                               \
    )

    namespace boost {
    namespace di {
    namespace policies {

    class check_for_circular_dependencies
    {
    public:
        template<
            typename TDeps
          , typename TGiven
          , bool Assert = true
          , template<
                typename
              , typename
              , typename = TDeps
              , typename = aux_::dependency<scopes::per_request, mpl::_1, mpl::_2>
            > class TBinder = detail::binder
          , template<
                typename
              , typename = void
            > class TCtorTraits = aux_::ctor_traits
        >
        class verify
        {
            template<typename TDependency>
            struct ctor
                : TCtorTraits<typename TDependency::given>::type
            { };

            template<typename TCallStack>
            struct is_unique_call_stack
                : mpl::bool_<
                      static_cast<std::size_t>(mpl::accumulate<
                          typename mpl::transform<
                              TCallStack
                            , mpl::count<TCallStack, mpl::_>
                          >::type
                        , mpl::int_<0>
                        , mpl::plus<mpl::_1, mpl::_2>
                      >::type::value
                    ) == mpl::size<TCallStack>::value
                  >
            { };

            template<
                typename
              , typename
              , typename = void
              , typename = void
            >
            struct circular_dependencies_impl;

            template<
                typename T
              , typename TCallStack
              , typename = void
              , typename = void
            >
            struct circular_dependencies
               : circular_dependencies_impl<
                      typename TBinder<T, TCallStack>::type
                    , typename mpl::push_back<
                          TCallStack
                        , typename TBinder<T, TCallStack>::type::given
                      >::type
                  >
            { };

            #include BOOST_PP_ITERATE()

        public:
            typedef circular_dependencies<
                typename aux_::make_plain<TGiven>::type
              , mpl::vector0<>
            > type;
        };
    };

    } // namespace policies
    } // namespace di
    } // namespace boost

    #endif

#else

    #define BOOST_DI_CHECK_FOR_CIRCULAR_DEPENDENCIES_IMPL(z, n, _)          \
        BOOST_PP_COMMA_IF(n)                                                \
        circular_dependencies<                                              \
            typename mpl::at_c<typename ctor<TDependency>::type, n>::type   \
          , TCallStack                                                      \
        >

    template<typename TDependency, typename TCallStack>
    struct circular_dependencies_impl<
        TDependency
      , TCallStack
      , typename enable_if_c<
            mpl::size<typename ctor<TDependency>::type>::value
            ==
            BOOST_PP_ITERATION()
        >::type
      , typename enable_if<is_unique_call_stack<TCallStack> >::type
    >
    BOOST_PP_EXPR_IF(BOOST_PP_ITERATION(), :)
        BOOST_PP_REPEAT(BOOST_PP_ITERATION(), BOOST_DI_CHECK_FOR_CIRCULAR_DEPENDENCIES_IMPL, ~)
    { };

    template<typename TDependency, typename TCallStack>
    struct circular_dependencies_impl<
        TDependency
      , TCallStack
      , typename enable_if_c<
            mpl::size<typename ctor<TDependency>::type>::value
            ==
            BOOST_PP_ITERATION()
        >::type
      , typename disable_if<is_unique_call_stack<TCallStack> >::type
    >
    :
        mpl::false_
    {
       BOOST_MPL_ASSERT_MSG(
            !Assert
          , CIRCULAR_DEPENDENCIES_ARE_NOT_ALLOWED
          , (typename TDependency::given, TCallStack)
        );
    };

    #undef BOOST_DI_CHECK_FOR_CIRCULAR_DEPENDENCIES_IMPL

#endif

