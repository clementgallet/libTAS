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

/* We want to store the state of each thread somewhere,
 * to be able to differentiate if some code is being called by the game
 * or by our own program.
 *
 * Indeed, if we use some library that eventually call a function that we hooked,
 * we don't want to process that call in the same way as if the game called that
 * function itself.
 *
 * This is also necessary to avoid some recursive loops.
 * For example, the logging function calls fwrite at some point,
 * so printing a log message inside the hooked fwrite function leads to a loop.
 *
 * This state is different for each thread, so we have to declare an instance
 * using c++11 thread_local.
 */

#ifndef LIBTAS_GLOBALSTATE_H_INCLUDED
#define LIBTAS_GLOBALSTATE_H_INCLUDED

class GlobalState
{
    public:
        /* Add or remove the NATIVE, OWNCODE and NOLOG flags.
         * This function and all other set* functions have a call count,
         * meaning we must call set*(false) by the same number as set*(true)
         * to disable the flag */
        static void setNative(bool state);

        /* Check the NATIVE flag */
        static bool isNative(void);

        /* Add or remove the OWNCODE and NOLOG flags. */
        static void setOwnCode(bool state);
        /* Check the OWNCODE flag */
        static bool isOwnCode(void);

        /* Add or remove the NOLOG flag. */
        static void setNoLog(bool state);
        /* Check the NOLOG flag */
        static bool isNoLog(void);

    private:

        /* When NATIVE flag is on, we ask each hooked function to act
         * as closely as possible as the real function. This disable
         * any log message, side effects, etc.
         */
        static thread_local int native;

        /* When OWNCODE flag is on, we indicate each hooked function that
         * the caller was our own code and not the game code. This usually
         * make the code closer to the original function, although it is not
         * as strong as NATIVE.
         */
        static thread_local int owncode;

        /* When NOLOG flag is on, we ask each hooked function to not generate
         * any log message.
         */
        static thread_local int nolog;
};

class GlobalNative
{
public:
    GlobalNative();
    ~GlobalNative();
};

#define NATIVECALL(expr) do{GlobalNative gn; expr;} while (false)

class GlobalOwnCode
{
public:
    GlobalOwnCode();
    ~GlobalOwnCode();
};

#define OWNCALL(expr) do{GlobalOwnCode gn; expr;} while (false)

class GlobalNoLog
{
public:
    GlobalNoLog();
    ~GlobalNoLog();
};

#endif
