#include <utility>
#include <string_view>
#include <string>
#include <iostream>

using namespace std;

constexpr size_t constfn_hash(const char * str, size_t seed = 0)
{
	return 0 == *str ? seed : constfn_hash(str + 1, seed ^ (*str + 0x9e3779b9 + (seed << 6) + (seed >> 2)));
}

constexpr size_t constfn_hash(size_t n, const char * str, size_t seed = 0)
{
	return 0 == n ? seed : constfn_hash(n - 1, str + 1, seed ^ (*str + 0x9e3779b9 + (seed << 6) + (seed >> 2)));
}

inline size_t constfn_hash(const std::string& str, size_t seed = 0)
{
	std::cout << "constfn_hash: " << str << " seed: " << seed<<std::endl;
	return constfn_hash(str.c_str(), seed);
}

size_t compile_eval_in_case_label(const std::string& str) {
	switch (constfn_hash(str)) {
	case constfn_hash("vertex_shader"):
		return 1;
	case constfn_hash("pixel_shader"):
		return 2;
	default:
		return 3;
	}
}

int main() {
	std::cout << compile_eval_in_case_label("vertex_shader");//msvc output : 3
}
