#ifndef TPC_DEBUG_HPP
#define TPC_DEBUG_HPP

#include <iostream>
#include <utility>

#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>

namespace detail {

	extern boost::mutex debug_mutex;

    inline void debug_impl()
    {
        std::cerr << "\n";
    }

    template< typename T0, typename... T >
    void debug_impl( T0 const& arg0, T&&... args )
    {
        std::cerr << arg0;
        debug_impl( std::forward< T >( args )... );
    }

} // namespace detail

template< typename... T >
void debug( T&&... args )
{
	boost::lock_guard< boost::mutex > lock( detail::debug_mutex );
    detail::debug_impl( std::forward< T >( args )... );
}

#endif // TPC_DEBUG_HPP
