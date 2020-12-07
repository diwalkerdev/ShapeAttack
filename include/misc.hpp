#ifndef SHAPE_MISC
#define SHAPE_MISC

#include "fmt/core.h"
#include "linalg/core.hpp"

template <typename Tp, std::size_t M, std::size_t N>
struct fmt::formatter<linalg::Matrix<Tp, M, N>> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(linalg::Matrix<Tp, M, N> const& A, FormatContext& ctx)
    {
        std::string buffer;
        buffer.reserve(16);
        for (auto const& row : iter(A))
        {
            for (auto el : row)
            {
                //  TODO: forward on the representation from the parse step.
                buffer += fmt::format("{0} ", el);
            }
            buffer += "\n";
        }
        return fmt::format_to(ctx.out(), "{0}", buffer);
    }
};

///////////////////////////////////////////////////////////////////////////////

inline void println(char const* message, float number)
{
    printf("%s", message);
    printf(": %f\n", number);
}

///////////////////////////////////////////////////////////////////////////////

#endif
