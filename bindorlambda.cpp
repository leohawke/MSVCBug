#include "foobar.h"

int main(){
	using namespace sad;
	using namespace std::placeholders;

	ValueNode root;

	RegisterStrict(root, "*", [](ValueNode& term) {
		CallBinaryFold<int, std::multiplies<>>(std::multiplies<>(), 1, term);
	});

	
	//error C2338: tuple index out of bounds
	//RegisterStrict(root, "*", std::bind(CallBinaryFold<int, std::multiplies<>>, (std::multiplies<>(), 1, _1)));
}