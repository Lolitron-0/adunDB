#pragma once
#include <boost/assert.hpp>
#define adun_assert_nomsg(expr) BOOST_ASSERT((expr))
#define adun_assert(expr, msg) BOOST_ASSERT_MSG((expr), (msg))
