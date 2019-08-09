/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "localewrappers.h"
#include "logging.h"
#include "hook.h"

#include <cstring>

namespace libtas {

DEFINE_ORIG_POINTER(setlocale)
DEFINE_ORIG_POINTER(getenv)

static const char* config_locale()
{
    switch (shared_config.locale) {
    case SharedConfig::LOCALE_ENGLISH:
        return "en_US.utf8";
    case SharedConfig::LOCALE_JAPANESE:
        return "ja_JP.utf8";
    case SharedConfig::LOCALE_KOREAN:
        return "ko_KR.utf8";
    case SharedConfig::LOCALE_CHINESE:
        return "zh_CN.utf8";
    case SharedConfig::LOCALE_SPANISH:
        return "es_ES.utf8";
    case SharedConfig::LOCALE_GERMAN:
        return "de_DE.utf8";
    case SharedConfig::LOCALE_FRENCH:
        return "fr_FR.utf8";
    case SharedConfig::LOCALE_ITALIAN:
        return "it_IT.utf8";
    case SharedConfig::LOCALE_NATIVE:
        return "";
    default:
        return "";
    }
}

/* Override */ char *setlocale (int category, const char *locale) throw()
{
    debuglog(LCF_LOCALE, __func__, " called with category ", category, " and locale ", locale?locale:"<NULL>");
    char* mylocale = const_cast<char*>(config_locale());
    if (mylocale[0] == '\0') {
        /* Return native locale */
        LINK_NAMESPACE_GLOBAL(setlocale);
        return orig::setlocale(category, locale);
    }
    return mylocale;
}

char *getenv (const char *name) throw()
{
    LINK_NAMESPACE_GLOBAL(getenv);
    if (GlobalState::isNative()) {
        return orig::getenv(name);
    }

    debuglog(LCF_LOCALE, __func__, " called with name ", name);
    if (0 == strncmp(name, "LANG", 4)) {
        char* mylocale = const_cast<char*>(config_locale());
        if (mylocale[0] != '\0') {
            return mylocale;
        }
    }
    char* ret = orig::getenv(name);
    debuglog(LCF_LOCALE, "  returning ", ret);

    return ret;
}

}
