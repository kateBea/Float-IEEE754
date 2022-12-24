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
    constexpr std::size_t mantissa_length{ 23 };
    std::string result{};

    for (std::size_t iter{}; iter < mantissa_length; ++iter) {
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

auto normalize(std::int32_t whole, std::int32_t decimal) -> std::string {
    return {};
}

template <typename std::size_t size_>
auto get_float(const std::string& number, std::array<std::int8_t, size_>& bits) -> void {
    bool is_negative{ number < 0.0f };
    std::int32_t whole_part{ whole(number) };
    std::int32_t dec_part{ decimal(number) };

    std::cout << bin_whole_part(whole_part) << std::endl;
    std::cout << bin_decimal_part(dec_part) << std::endl;

    std::string norm{ std::move(normalize(whole_part, dec_part)) };
}

int main() {
    std::string num{};
    std::cout << "Value: ";
    std::cin >> num;
    constexpr std::uint32_t bit_length{ 32 };

    std::array<std::int8_t, bit_length> float_bits{ 1, 0, 1, 1, 1, 0, 0, 1 };

    get_float(num, float_bits);



#if 0
    print(float_bits);
#endif
    return 0;
}
