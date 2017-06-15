/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "backtrace.h"
#include <cxxabi.h>
#include <execinfo.h>
#include <memory>
#include <cstdio>

namespace libtas {

/* Code taken from http://stackoverflow.com/a/19190421 */
static const char* demangle( const char* const symbol )
{
    /*
    const std::unique_ptr< char, decltype( &std::free ) > demangled(
            abi::__cxa_demangle( symbol, 0, 0, 0 ), &std::free );
    if( demangled ) {
        return demangled.get();
    }
    else {*/
        return symbol;
    //}
}

void printBacktrace(void)
{
    thread_local static int recurs = 0;
    if (recurs)
        return;
    recurs = 1;

    //threadState.setNoLog(true);
    void* addresses[256];
    const int n = backtrace(addresses, 256);
    char** symbols = backtrace_symbols(addresses, n);
    for( int i = 0; i < n; ++i ) {
        /* We parse the symbols retrieved from backtrace_symbols() to
         * extract the "real" symbols that represent the mangled names.
         */
        char* const symbol = symbols[i];
        char* end = symbol;
        while( *end ) {
            ++end;
        }
        /* Scanning is done backwards, since the module name
         * might contain both '+' or '(' characters.
         */
        while( end != symbol && *end != '+' ) {
            --end;
        }
        char* begin = end;
        while( begin != symbol && *begin != '(' ) {
            --begin;
        }

        if( begin != symbol ) {
            fprintf(stderr, "%.*s", static_cast<int>(++begin - symbol), symbol);
            *end++ = '\0';
            fprintf(stderr, "%s+%s\n", demangle( begin ), end);
        }
        else {
            fprintf(stderr, "%s\n", symbol);
        }
    }
    fprintf(stderr, "\n");
    free(symbols);
    //threadState.setNoLog(false);
    recurs = 0;
}

}
