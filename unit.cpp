#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"
#include "text.hpp"
#include "sort.hpp"
#include <unistd.h>
#include <limits.h>

TEST_CASE("UTF-8 enforcement", "[utf8]") {
	REQUIRE(check_utf8("") == std::string(""));
	REQUIRE(check_utf8("hello world") == std::string(""));
	REQUIRE(check_utf8("Καλημέρα κόσμε") == std::string(""));
	REQUIRE(check_utf8("こんにちは 世界") == std::string(""));
	REQUIRE(check_utf8("👋🌏") == std::string(""));
	REQUIRE(check_utf8("Hola m\xF3n") == std::string("\"Hola m\xF3n\" is not valid UTF-8 (0xF3 0x6E)"));
}

TEST_CASE("UTF-8 truncation", "[trunc]") {
	REQUIRE(truncate16("0123456789abcdefghi", 16) == std::string("0123456789abcdef"));
	REQUIRE(truncate16("0123456789éîôüéîôüç", 16) == std::string("0123456789éîôüéî"));
	REQUIRE(truncate16("0123456789😀😬😁😂😃😄😅😆", 16) == std::string("0123456789😀😬😁"));
	REQUIRE(truncate16("0123456789😀😬😁😂😃😄😅😆", 17) == std::string("0123456789😀😬😁"));
	REQUIRE(truncate16("0123456789あいうえおかきくけこさ", 16) == std::string("0123456789あいうえおか"));
}

int intcmp(const void *v1, const void *v2) {
	return *((int *) v1) - *((int *) v2);
}

TEST_CASE("External quicksort", "fqsort") {
	std::vector<FILE *> inputs;

	size_t written = 0;
	for (size_t i = 0; i < 5; i++) {
		std::string tmpname = "/tmp/in.XXXXXXX";
		int fd = mkstemp((char *) tmpname.c_str());
		unlink(tmpname.c_str());
		FILE *f = fdopen(fd, "w+b");
		inputs.emplace_back(f);
		size_t iterations = 2000 + rand() % 200;
		for (size_t j = 0; j < iterations; j++) {
			int n = rand();
			fwrite((void *) &n, sizeof(int), 1, f);
			written++;
		}
		rewind(f);
	}

	std::string tmpname = "/tmp/out.XXXXXX";
	int fd = mkstemp((char *) tmpname.c_str());
	unlink(tmpname.c_str());
	FILE *f = fdopen(fd, "w+b");

	fqsort(inputs, sizeof(int), intcmp, f, 256);
	rewind(f);

	int prev = INT_MIN;
	int here;
	size_t nread = 0;
	while (fread((void *) &here, sizeof(int), 1, f)) {
		REQUIRE(here >= prev);
		prev = here;
		nread++;
	}

	fclose(f);
	REQUIRE(nread == written);
}
