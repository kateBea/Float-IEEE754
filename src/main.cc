/**
 * @file main.cc
 * @author kt
 * @brief
 * @version 1.0
 * @date 2022-12-27
 *
 */

/**
 * @brief IEEE-754 single precision float converter tool.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

#include <array>
#include <cstdint>
#include <iterator>
#include <numeric>
#include <bitset>
#include <iomanip>
#include <string>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <bitset>
#include <cmath>

static constexpr std::size_t g_exponent_offset{ 127 };
static constexpr std::size_t g_mantissa_length{ 23 };
static constexpr std::size_t g_length{ 32 };

// returns the hexadecimal representation of a given number
auto to_hex(std::uint32_t num) -> std::string {
    static const char* rep[]{ "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"
                        "A", "B", "C", "D", "E", "F", };
    if (num < 16)
        return std::string(rep[num % 15]);

    std::string res{ std::move(to_hex(num / 16)) };
    return res + std::string(rep[num % 15]);
}

// prints formatted output to the standard channel 
template <typename std::size_t size_>
auto print(const std::array<char, size_>& bits) {


    std::cout << "\n----------------------------------------------------------\n";
    std::cout << "sign bit (s)" << "   " << "exponent bits (s)" << "   " << "    mantissa bit (s)   " << std::endl;
    std::cout << "------------" << "   " << "-----------------" << "   " << "-----------------------" << std::endl;
    std::cout << "      ";
    std::for_each(bits.begin(), bits.begin() + 1, [](char ch) { std::cout << ch; });
    std::cout << "             ";
    std::for_each(bits.begin() + 1, bits.begin() + 9, [](char ch) { std::cout << ch; });
    std::cout << "       ";
    std::for_each(bits.begin() + 9, bits.end(), [](char ch) { std::cout << ch; });
    std::cout << "\n----------------------------------------------------------\n";

    std::bitset<32> temp{ std::string{ bits.begin(), bits.end() } };
    std::cout << std::hex << std::uppercase << "Hexadecimal representation: 0x" << temp.to_ulong() << std::endl;
    std::cout << "Binary representation:      0b" << std::bitset<32>{ temp } << std::endl;
}

// returns the integer part of a floating point number
auto whole(const std::string& number) -> std::uint32_t {
    std::string result{};
    auto it{ number.begin() };

    for (; it != number.end() and *it != '.'; ++it)
        result += *it;

    return std::stoi(result);
}

// returns the fractional part of a floating point number
auto decimal(const std::string& number) -> std::uint32_t {
    std::string result{};
    auto it{ number.begin() };

    while (it != number.end() and *it != '.')
        ++it;

    // if there's not dot then number has no decimal part
    if (it != number.end())
        result = std::move(std::string(++it, number.end()));

    return result.empty() ? 0 : std::stoi(result);
}

auto operator<(const std::string& rhs, float lhs) -> bool {
    return std::stof(rhs) < lhs;
}

// Returns the binary representation of whole
// which is expected to be the integer part of a floating point number without the sign
// the result will have no leading zeros
auto bin_whole_part(std::uint32_t whole) -> std::string {
    if (whole == 0)
        return std::string{ "0" };
    if (whole == 1)
        return std::string{ "1" };

    std::string res{ std::move(bin_whole_part(whole / 2)) };
    return res + std::to_string(whole % 2);

    // return std::bitset<32>{ static_cast<std::size_t>(whole) }.to_string();
}

// Returns the binary representation of dec
// which is expected to be the fractional part of a floating point number without sign
auto bin_decimal_part(std::uint32_t dec) -> std::string {
    float temp{ std::stof("0." + std::to_string(dec)) };
    std::string result{};

    for (std::size_t iter{}; iter < g_mantissa_length; ++iter) {
        temp = temp * 2.0f;
        if (temp < 1.0)
            result += '0';
        else {
            result += '1';
            temp = temp - 1.0f;
        }
    }

    return result;
}

auto normalize(std::uint32_t whole, std::uint32_t decimal) -> std::pair<std::string, std::int32_t> {
    std::int32_t exponent{ static_cast<std::int32_t>(bin_whole_part(whole).size()) - 1 };
    auto whole_str{ bin_whole_part(whole) };
    auto dec_str{ bin_decimal_part(decimal) };
    std::string::size_type position{};

    // concatenate to obtain the binary representation f the floating point number
    // e.g.: 5.5 -> 101.1
    std::string result{};

    // There's three cases to account for: 0.xxxx..., 1.xxxx..., 
    // 1x.xxxx... (numbers greater than 1)

    switch (whole)
    {
    case 0:
        // case 0.xxxx...
        position = dec_str.find('1');

        // keep mantissa bits
        dec_str.erase(0, position + 1);

        result = std::move(dec_str);
        exponent = g_exponent_offset + (-1 * static_cast<std::int32_t>(position + 1));
        break;
    case 1:
        // 1.xxxx...
        // nothing to do really it's already normalised
        result = std::move(dec_str);
        exponent = static_cast<std::int32_t>(g_exponent_offset);
        break;
    default:
        // 1x.xxxx...
        result = std::move(std::string(whole_str.begin() + 1, whole_str.end()) + dec_str);

        // to make sure the mantissa is 
        // not longer than its max length
        if (result.length() > g_mantissa_length)
            // result[g_mantissa_length] is the first character
            // out of the max allowed range, from that one, erase rest till 
            // the end
            result.erase(g_mantissa_length, result.size() - g_mantissa_length);

        exponent = g_exponent_offset + static_cast<std::int32_t>(whole_str.size() - 1);
        break;
    }

    return std::make_pair(result, exponent);
}

// outputs the contents of a vector between two indices
// both are expected to be valid indices in the range [0, bits.size() - 1]
template <typename std::size_t size_>
auto print(const std::array<char, size_>& bits, std::size_t start, std::size_t end, const char* str) -> void {
    std::cout << str;
    for (; start <= end; ++start) {
        std::cout << bits[start];
    }

    std::cout << std::endl;
}

template <typename std::size_t size_>
auto get_bits(std::string number, std::array<char, size_>& bits) -> void {
    // number passed by value to avoid changing source

    bool is_negative{ number < 0.0f };

    if (is_negative)
        number.erase(0, 1);

    // Normalized value. norm.first containts the binary value
    // norm.second contains the value of biased exponent
    auto norm{ normalize(whole(number), decimal(number)) };


    auto exponent_str{ bin_whole_part(norm.second) };

    while (exponent_str.size() < 8)
        // sign extension to fill remaining bits
        exponent_str.insert(0, 1, norm.second < 0 ? '1' : '0');

    bits[0] = is_negative ? '1' : '0';

    // mantissa bits. norm.first.size() <= g_mantissa_length
    for (std::size_t index{}; index < norm.first.size(); ++index) 
        bits[9 + index] = norm.first[index];

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

int main(int, char**) {
    // 5.77
    // correct: 01000000101110001010001111010111
    // out:     01000000101110001010001111010111
    std::string num{};
    std::cout << "Value: ";
    std::cin >> num;
    std::array<char, g_length> bits{};

    get_bits(num, bits);
    print(bits);

    return 0;
}
