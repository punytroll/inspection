#ifndef INSPECTION_COMMON_COLORS_H
#define INSPECTION_COMMON_COLORS_H

namespace Inspection
{
    auto const g_Black = std::string{"\x1b[30m"};
    auto const g_Red = std::string{"\x1b[31m"};
    auto const g_Green = std::string{"\x1b[32m"};
    auto const g_Yellow = std::string{"\x1b[33m"};
    auto const g_Blue = std::string{"\x1b[34m"};
    auto const g_Magenta = std::string{"\x1b[35m"};
    auto const g_Cyan = std::string{"\x1b[36m"};
    auto const g_White = std::string{"\x1b[37m"};
    auto const g_BrightBlack = std::string{"\x1b[90m"};
    auto const g_BrightRed = std::string{"\x1b[91m"};
    auto const g_BrightGreen = std::string{"\x1b[92m"};
    auto const g_BrightYellow = std::string{"\x1b[93m"};
    auto const g_BrightBlue = std::string{"\x1b[94m"};
    auto const g_BrightMagenta = std::string{"\x1b[95m"};
    auto const g_BrightCyan = std::string{"\x1b[96m"};
    auto const g_BrightWhite = std::string{"\x1b[97m"};
    auto const g_Reset = std::string{"\x1b[0m"};
}

#endif
