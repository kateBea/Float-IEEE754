/**
 * @file main.cc
 * @author kt
 * @brief
 * @version 1.0
 * @date 2022-12-27
 *
 * @copyright Copyright (c) 2022
 * @category Tests
 *
 */

/**
 * @brief IEEE-754 single precision float converter tool.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

#include <array>
#include <cstdint>
#include <numeric>
#include <bitset>
#include <iomanip>
#include <string>
#include <iostream>
#include <algorithm>
#include <cmath>

static constexpr std::size_t g_offset{ 127 };
static constexpr std::size_t g_mantissa_length{ 23 };
static constexpr std::size_t g_length{ 32 };

// returns the hexadecimal representation of a given number
auto to_hex(std::uint32_t num) -> std::string {
    static const char* rep[] { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"
                        "A", "B", "C", "D", "E", "F", };
    if (num < 16)
        return std::string(rep[num % 15]);

    std::string res{ std::move(to_hex(num / 16)) };
    return res + std::string(rep[num % 15]);
}

// prints formatted output to the standard channel 
template <typename std::size_t size_>
auto print(const std::array<char, size_>& bits) {
    auto two_power{ [&bits](const std::int64_t acc, char curr) -> auto {
            // begin() holds the most significant bit
            static auto exponent{ static_cast<std::uint32_t>(bits.size() - 1) };
            auto result{ static_cast<std::uint32_t>(acc + static_cast<std::uint32_t>(curr - '0') *
                static_cast<std::uint32_t>(std::pow(2, exponent))) };
            --exponent;
            return result;
        }
    };

    std::cout << "\n----------------------------------------------------------\n";
    std::cout << "sign bit (s)" << "   " <<"exponent bits (s)" << "   " << "    mantissa bit (s)   " << std::endl;
    std::cout << "------------" << "   " <<"-----------------" << "   " << "-----------------------" << std::endl;
    std::cout << "      ";
    std::for_each(bits.begin(), bits.begin() + 1, [](char ch) { std::cout << ch;});
    std::cout << "             ";
    std::for_each(bits.begin() + 1, bits.begin() + 9, [](char ch) { std::cout << ch;});
    std::cout << "       ";
    std::for_each(bits.begin() + 9, bits.end(), [](char ch) { std::cout << ch;});
    std::cout << "\n----------------------------------------------------------\n";

    // not properly showing hexadecimal representation
    auto temp{ static_cast<std::uint64_t>(std::accumulate(bits.begin(), bits.end(), 0, two_power)) };
    std::cout << "Hexadecimal representation: 0x" << to_hex(temp) << std::endl;
    std::cout << "Binary representation:      0b" << std::bitset<32>{ temp } << std::endl;
}

// returns the integer part of a floating point number
auto whole(const std::string& number) -> std::int32_t {
    std::string result{};
    auto it{ number.begin() };

    for (; it != number.end() and *it != '.'; ++it)
        result += *it;

    return std::stoi(result);
}

// returns the fractional part of a floating point number
auto decimal(const std::string& number) -> std::int32_t {
    std::string result{};
    auto it{ number.begin() };

    while (it != number.end() and *it != '.')
        ++it;

    // skip dot
    if (it != number.end())
        ++it;

    for (; it != number.end(); ++it)
        result += *it;

    return result.empty() ? 0 : std::stoi(result);
}

auto operator<(const std::string& rhs, float lhs) -> bool {
    return std::stof(rhs) < lhs;
}

// Returns the binary representation of whole
// which is expected to be the integer part of a floating point number
auto bin_whole_part(std::int32_t whole) -> std::string {
    if (whole == 0)
        return std::string{ "0" };
    if (whole == 1)
        return std::string{ "1" };

    std::string res{ std::move(bin_whole_part(whole / 2)) };
    return res + std::to_string(whole % 2);
}

// Returns the binary representation of dec
// which is expected to be the fractional part of a floating point number
auto bin_decimal_part(std::int32_t dec) -> std::string {
    float temp{ std::stof("0." + std::to_string(dec)) };
    std::string result{};

    for (std::size_t iter{}; iter < g_mantissa_length; ++iter) {
        temp = temp * 2.0f;
        if (temp < 1)
            result.append("0");
        else {
            result.append("1");
            temp = temp - 1.0f;
        }
    }

    return result;
}

auto normalize(std::int32_t whole, std::int32_t decimal) -> std::pair<std::string, std::int32_t> {
    std::string result{ "1." };

    std::int32_t exponent{ static_cast<std::int32_t>(bin_whole_part(whole).size()) - 1 };
    auto whole_str{ bin_whole_part(whole) };
    auto dec_str{ bin_decimal_part(decimal) };

    auto it_whole{ whole_str.begin()++ };
    auto it_dec{ dec_str.begin() };

    // used to make sure the mantisa is 23 bits long
    std::size_t counter{ 0 };

    if (whole != 0) {

        for ( ; it_whole != whole_str.end() and counter < g_mantissa_length; ++it_whole) {
            result += *it_whole;
            ++counter;
        }

        for ( ; it_dec != dec_str.end() and counter < g_mantissa_length; ++it_dec) {
            result += *it_dec;
            ++counter;
        }

        exponent = exponent + g_offset;
    }
    else {
        std::size_t placement{};

        if (*it_dec == '1') {
            ++it_dec;
            placement = 1;
        }

        for ( ; it_dec != dec_str.end() and counter < g_mantissa_length and *it_dec != '1'; ++it_dec) {
            result += *it_dec;
            ++counter;
            ++placement;
        }

        for ( ; counter < g_mantissa_length ; ++counter) {
            if (it_dec == dec_str.end())
                result += "0";
            else {
                result += *it_dec;
                ++it_dec;
            }
        }

        exponent = g_offset - placement;
    }

    return std::make_pair(result, exponent);
}

// outputs the contents of a vector between two indices
// both are expected to be valid indices in the range [0, bits.size() - 1]
template <typename std::size_t size_>
auto print(const std::array<char, size_>& bits, std::size_t start, std::size_t end, const char* str) -> void {
    std::cout << str;
    for ( ; start <= end; ++start) {
        std::cout << bits[start];
    }

    std::cout << std::endl;
}

template <typename std::size_t size_>
auto get_bits(const std::string& p_number, std::array<char, size_>& bits) -> void {
    auto number{ p_number };
    bool is_negative{ number < 0.0f };

    if (is_negative)
        number.erase(0, 1);

    // Normalized value. norm.first constaints the binary value
    // norm.second contains the value of biased exponent
    auto norm{ normalize(whole(number), decimal(number)) };

    for (std::size_t index{}; index < g_mantissa_length; ++index) {
        bits[bits.size() - 1 - index] = norm.first[norm.first.size() - 1 - index];
    }

    auto exponent_str{ bin_whole_part(norm.second) };

    while (exponent_str.size() < 8)
        // sign extension to fill remaining bits
        exponent_str.insert(0, 1, norm.second < 0 ? '1' : '0');


    bits[0] = is_negative ? '1' : '0';

    // exponent bits
    for (std::size_t index{}; index < 8; ++index) 
        bits[index + 1] = exponent_str[index];
    

#if false
    // print stuff
    std::cout << "normalized value: " << norm.first << std::endl;
    print(bits, 0, 0, "sign bit (s): ");
    print(bits, 1, 8, "exponent bit (s): ");
    print(bits, 9, 31, "mantissa bit (s): ");
#endif
}

int main() {
    std::string num{};
    std::cout << "Value: ";
    std::cin >> num;
    std::array<char, g_length> bits{};

    get_bits(num, bits);
    print(bits);

    return 0;
}
