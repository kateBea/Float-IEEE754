#include <array>
#include <cstdint>
#include <numeric>
#include <bitset>
#include <iomanip>
#include <string>
#include <iostream>
#include <algorithm>
#include <cmath>

// IEEE-754 float (32 bits) conversion
// cpp20 required for std::accumulate...

static constexpr std::size_t g_offset{ 127 };
static constexpr std::size_t g_mantissa_length{ 23 };
static constexpr std::uint32_t g_length{ 32 };

template <typename std::size_t size_>
auto print(const std::array<std::int8_t, size_>& bits) {
    auto two_power{ [&bits](const std::int32_t acc, std::int8_t curr) -> auto {
            // begin() holds the most significant bit
            static std::int32_t exponent{ bits.size() - 1};
            std::int32_t result{ acc + static_cast<std::int32_t>(curr) *
                static_cast<std::int32_t>(std::pow(2, exponent)) };
            --exponent;
            return result;
        }
    };

    auto temp{ std::accumulate(bits.begin(), bits.end(), 0, two_power) };
    std::cout << std::ios::hex << std::uppercase << "Value hex: 0x" << temp << std::endl;
    std::cout << "Value bin: 0b" << std::bitset<32>{ temp } << std::endl;
}

auto whole(const std::string& number) -> std::int32_t {
    std::string result{};
    auto it{ number.begin() };

    for (; it != number.end() and *it != '.'; ++it)
        result += *it;

    return std::stoi(result);
}

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

auto bin_whole_part(std::int32_t whole) -> std::string {
    if (whole == 0)
        return std::string{ "0" };
    if (whole == 1)
        return std::string{ "1" };

    std::string res{ std::move(bin_whole_part(whole / 2)) };
    return res + std::to_string(whole % 2);
}

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

auto normalize(std::int32_t whole, std::int32_t decimal) -> std::pair<std::string, int> {
    constexpr std::int32_t g_offset{ 127 };
    std::string result{ "1." };

    std::int32_t exponent{ bin_whole_part(whole).size() - 1 };
    auto whole_str{ bin_whole_part(whole) };
    auto dec_str{ bin_decimal_part(decimal) };
    std::cout << "exponent float ieee754: " << exponent << std::endl;
    std::cout << "whole part bin: " << whole_str << std::endl;
    std::cout << "decimal part bin: " << dec_str << std::endl;

    auto it_whole{ whole_str.begin()++ };
    auto it_dec{ dec_str.begin()++ };

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
        for ( ; it_dec != dec_str.end() and counter < g_mantissa_length and *it_dec != '1'; ++it_dec) {
            ++placement;
        }

        exponent = g_offset - placement;
    }

    return std::make_pair(result, exponent);
}

template <typename std::size_t size_>
auto print(const std::array<char, size_>& bits, std::size_t start, std::size_t end, const char* str) -> void {
    std::cout << str;
    for ( ; start <= end; ++start) {
        std::cout << bits[start];
    }

    std::cout << std::endl;
}

template <typename std::size_t size_>
auto get_float(const std::string& number, std::array<char, size_>& bits) -> void {
    bool is_negative{ number < 0.0f };

    // Normalized value. norm.first constaints the binary value
    // norm.second contains the value of biased exponent
    auto norm{ normalize(whole(number), decimal(number)) };

    for (std::size_t index{}; index < g_mantissa_length; ++index) {
        bits[bits.size() - 1 - index] = norm.first[norm.first.size() - 1 - index];
    }

    auto exponent_str{ bin_whole_part(norm.second) };

    while (exponent_str.size() <= 8)
        // sign extension to fill remaining bits
        exponent_str.insert(0, 1, norm.second < 0 ? '1' : '0');


    bits[0] = is_negative ? '1' : '0';

    // exponent bits
    for (std::size_t index{}; index < 8; ++index) {
        bits[index + 1] = exponent_str[index];
    }

#if true
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
    std::array<char, g_length> float_bits{};

    get_float(num, float_bits);



#if 0
    print(float_bits);
#endif
    return 0;
}
