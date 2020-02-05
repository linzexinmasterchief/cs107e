#include "assert.h"
#include "printf.h"
#include "../printf_internal.h"
#include <stddef.h>
#include "strings.h"
#include "uart.h"

static void test_memset(void)
{
    int numA = 0xefefefef;
    int numB = 2;

    memset(&numB, 0xef, sizeof(int));
    assert(numA == numB);
	
	char str[] = "107e gods please be merciful";

	memset(str, 'x', 0 * sizeof(char));
	assert(strcmp(str, "107e gods please be merciful") == 0);

	memset(str, '.', 10 * sizeof(char));
	assert(strcmp(str, "..........please be merciful") == 0);

	memset(str, 'x', strlen(str));
	for(int i = 0; i < strlen(str); i++)
		assert(str[i] == 'x');
	
	memset(str + 5, 'o', 5 * sizeof(char));
	assert(str[4] == 'x');
	for(int i = 0; i < 5; i++)
		assert(str[i + 5] == 'o');
	assert(str[10] == 'x');
}

static void test_memcpy(void)
{
    int numA = 0x12345678;
    int numB = 2;

    memcpy(&numB, &numA, sizeof(int));
    assert(numA == numB);

	char *strA = "HATE";
	char *strB = "LOVE";
	memcpy(strA, strB, strlen(strA));
	assert(strcmp(strA, strB) == 0);

	char str[] = "..........";
	memcpy(strA, str, sizeof(str));
	for(int i = 0; i < 10; i++){
		assert(*(strA + i) == '.');
	}
}

static void test_strlen(void)
{
    assert(strlen("green") == 5);
}

static void test_strcmp(void)
{
    assert(strcmp("apple", "apple") == 0);
    assert(strcmp("apple", "applesauce") < 0);
    assert(strcmp("pears", "apples") > 0);
}

static void test_strlcat(void)
{
    char buf[20];
    memset(buf, 0x77, sizeof(buf)); // init contents with known value

    buf[0] = '\0'; // start with empty string
    assert(strlen(buf) == 0);
    strlcat(buf, "CS", sizeof(buf));
    assert(strlen(buf) == 2);
    assert(strcmp(buf, "CS") == 0);
    strlcat(buf, "107e", sizeof(buf));
    assert(strlen(buf) == 6);
    assert(strcmp(buf, "CS107e") == 0);
}

static void test_strtonum(void)
{
    int val = strtonum("013", NULL);
    assert(val == 13);

    const char *input = "107rocks", *rest = NULL;
    val = strtonum(input, &rest);
    assert(val == 107);
    assert(rest == &input[3]);

	val = strtonum("0x013", NULL);
	assert(val == 19);

	input = "0x107rocks...wait that's not right";
	val = strtonum(input, &rest);
	assert(val == 263);
	assert(rest == &input[5]);
}

static void test_to_base(void)
{
	char simple[5];
	unsigned_to_base(simple, sizeof(simple), 1234, 10, 0);
	assert(strcmp(simple, "1234") == 0);

	char with_width[7];
	unsigned_to_base(with_width, sizeof(with_width), 1234, 10, 6);
	assert(strcmp(with_width, "001234") == 0);
	
	char hex_num[5];
	unsigned_to_base(hex_num, sizeof(hex_num), 35, 16, 4);
	assert(strcmp(hex_num, "0023") == 0);

    char buf[5];
    size_t bufsize = sizeof(buf);

    memset(buf, 0x77, bufsize); // init contents with known value

    int n = signed_to_base(buf, bufsize, -9999, 10, 6);
    assert(strcmp(buf, "-099") == 0);
    assert(n == 6);
}

static void test_snprintf(void)
{
    char buf[100];
    size_t bufsize = sizeof(buf);

    memset(buf, 0x77, sizeof(buf)); // init contents with known value

    // Start off simple...
    snprintf(buf, bufsize, "Hello, world!");
    assert(strcmp(buf, "Hello, world!") == 0);

	// Ramp it up a bit...
	snprintf(buf, bufsize, "%c%c = 100%% fresh", 'C', 'S');
	assert(strcmp(buf, "CS = 100% fresh") == 0);

    // Decimal
    snprintf(buf, bufsize, "%d", 45);
    assert(strcmp(buf, "45") == 0);

	// Simple hexadcimal
	snprintf(buf, bufsize, "test %x", 0xef);
	assert(strcmp(buf, "test ef") == 0);
	snprintf(buf, bufsize, "test %x", 0xbe);
	assert(strcmp(buf, "test be") == 0);

    // Hexadecimal with width
    snprintf(buf, bufsize, "%04x", 0xef);
    assert(strcmp(buf, "00ef") == 0);

	// Width sanity check
	snprintf(buf, bufsize, "%0x", 0xef);
	assert(strcmp(buf, "ef") == 0);

    // Pointer
    snprintf(buf, bufsize, "%p", (void *) 0x20200004);
    assert(strcmp(buf, "0x20200004") == 0);

    // Character
    snprintf(buf, bufsize, "%c", 'A');
    assert(strcmp(buf, "A") == 0);

    // String
    snprintf(buf, bufsize, "%s", "binky");
    assert(strcmp(buf, "binky") == 0);

    // Format string with intermixed codes
    snprintf(buf, bufsize, "CS%d%c!", 107, 'e');
    assert(strcmp(buf, "CS107e!") == 0);

  //  // Test return value
  //  assert(snprintf(buf, bufsize, "Hello") == 5);
  //  assert(snprintf(buf, 2, "Hello") == 5);
}

void main(void)
{
    // TODO: Add more and better tests!

    uart_init();
    uart_putstring("Start execute main() in tests/test_strings_printf.c\n");

    test_memset();
    test_memcpy();
    test_strlen();
    test_strcmp();
    test_strlcat();
    test_strtonum();
    test_to_base();
    test_snprintf();

    uart_putstring("Successfully finished executing main() in tests/test_strings_printf.c\n");
    uart_putchar(EOT);
}
