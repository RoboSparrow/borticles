#ifndef __TEST_H__
#define __TEST_H__

#define __DOING_TESTS__

extern unsigned int scount;
extern unsigned int gcount;

#define SECTION(message) do { fprintf(stderr, "\n\033[01;34mSECTION: %s\033[0m\n", message); scount++; } while(0)
#define GROUP(message) do { fprintf(stderr, "  \033[01;33mGROUP: %s\033[0m\n", message); gcount++; } while(0)
#define DESCRIBE(message) fprintf(stderr, "    \033[1;34m=>\033[0m %s() %s\n", __func__, message)
#define DONE() fprintf(stderr, "    \033[1;32mPASS\033[0m: %s\n\n", __func__)


#define ASSERT_FLOAT(a, b, epsilon) do { assert(fabs(a - b) < epsilon); } while(0)


void test_qtree(int argc, char **argv);
void test_qlist(int argc, char **argv);

#endif
