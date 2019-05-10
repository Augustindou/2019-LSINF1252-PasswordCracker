#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
//#include "CUnit/Automated.h"
//#include "CUnit/Console.h"

//so can pick function signature
#include "v3.h"

#include <stdio.h>  // for printf
#include <string.h>

/*
download CUnit with  http://cunit.sourceforge.net/

type to run the test :
gcc -Wall -c v3Func.c
gcc -Wall -pthread -L/usr/local/lib -o v3Func_test v3Func_test.c v3Func.o -lcunit -std=c99
./strlenVo_test

source: http://wpollock.com/CPlus/CUnitNotes.htm
doesn't work with main, don't think you need to test the main function
TODO with all functions, but how!?
*/


/* Test Suite setup and cleanup functions: */

int init_suite(void) { return 0; }
int clean_suite(void) { return 0; }

/************* Test case functions ****************/

void test_case_sample(void)
{
   CU_ASSERT(CU_TRUE);
   CU_ASSERT_NOT_EQUAL(2, -1);
   CU_ASSERT_STRING_EQUAL("string #1", "string #1");
   CU_ASSERT_STRING_NOT_EQUAL("string #1", "string #2");

   CU_ASSERT(CU_FALSE);
   CU_ASSERT_EQUAL(2, 3);
   CU_ASSERT_STRING_NOT_EQUAL("string #1", "string #1");
   CU_ASSERT_STRING_EQUAL("string #1", "string #2");
}

void strlenVo_test_1(void) {
  CU_ASSERT_EQUAL( strlenVo("assert", false), 2);
  CU_ASSERT_EQUAL( strlenVo("assert", true), 4);
}

void strlenVo_test_2(void) {
  CU_ASSERT_EQUAL( strlenVo("a", false), 1);
  CU_ASSERT_EQUAL( strlenVo("a", true), 0);
}
void strlenVo_test_3(void) {
  CU_ASSERT_EQUAL( strlenVo("c", false), 0);
  CU_ASSERT_EQUAL( strlenVo("c", true), 1);
}
void getSemValue_test_1(void){
  sem_t sem;
  sem_init(&sem, 0 , 0);
  CU_ASSERT_EQUAL(getSemValue(&sem),0);
  sem_post(&sem);
  CU_ASSERT_EQUAL(getSemValue(&sem), 1);
  sem_wait(&sem);
  CU_ASSERT_EQUAL(getSemValue(&sem),0);
  sem_destroy(&sem);
}void getSemValue_test_2(void){
  sem_t sem;
  sem_init(&sem, 0 , 0);
  CU_ASSERT_EQUAL(getSemValue(&sem),0);
  sem_post(&sem);
  CU_ASSERT_EQUAL(getSemValue(&sem), 1);
  sem_post(&sem);
  CU_ASSERT_EQUAL(getSemValue(&sem),2);
  sem_destroy(&sem);
}
void getSemValue_test_3(void){
  sem_t sem;
  sem_init(&sem, 0 , 0);
  CU_ASSERT_EQUAL(getSemValue(&sem),0);
  sem_post(&sem);
  sem_wait(&sem);
  CU_ASSERT_EQUAL(getSemValue(&sem), 0);
  sem_post(&sem);
  sem_wait(&sem);
  CU_ASSERT_EQUAL(getSemValue(&sem),0);
  sem_destroy(&sem);
}
void insertInBuffer_test_1(void) {
  int NumberOfElements = 3;
  int sizeOfString = 5;
  char * PC = (char *) malloc (NumberOfElements*  sizeOfString);
  char * str = malloc(sizeof(char)*sizeOfString);
  strcpy(str, "tesd");
  insertInBuffer(str, (char*)PC, NumberOfElements, true);
  CU_ASSERT_STRING_EQUAL(PC, str);
}
void removeFromBuffer_test_1(void) {
  int NumberOfElements = 3;
  int sizeOfString = 5;
  char * PC = (char *) malloc (NumberOfElements*  sizeOfString);
  char * str = malloc(sizeof(char)*sizeOfString);
  char * str2 = malloc(sizeof(char)*sizeOfString);
  strcpy(str, "tesd");
  insertInBuffer(str, (char*)PC, NumberOfElements, true);
  removeFromBuffer(str2, PC, NumberOfElements,true);
  CU_ASSERT_STRING_EQUAL(str, str2);
  strcpy(str, "tesa");
  insertInBuffer(str, (char*)PC, NumberOfElements, true);
  strcpy(str, "tesb");
  insertInBuffer(str, (char*)PC, NumberOfElements, true);
  removeFromBuffer(str2, PC, NumberOfElements,true);
  CU_ASSERT_STRING_EQUAL("tesa", str2);
}

/************* Test Runner Code goes here **************/

int main ( void )
{
  CU_pSuite pSuite = NULL;

  /* initialize the CUnit test registry */
  if ( CUE_SUCCESS != CU_initialize_registry() )
    return CU_get_error();

       //strlenVo
       /* add a suite to the registry */
       pSuite = CU_add_suite( "strlenVo_test_suite", init_suite, clean_suite );
       if ( NULL == pSuite ) {
          CU_cleanup_registry();
          return CU_get_error();
       }

       /* add the tests to the suite */
       if ( (NULL == CU_add_test(pSuite, "strlenVo_test_1", strlenVo_test_1)) ||
            (NULL == CU_add_test(pSuite, "strlenVo_test_2", strlenVo_test_2)) ||
            (NULL == CU_add_test(pSuite, "strlenVo_test_3", strlenVo_test_3))
          )
       {
          CU_cleanup_registry();
          return CU_get_error();
       }

       //getSemValue
       /* add a suite to the registry */
       pSuite = CU_add_suite( "getSemValue_test_suite", init_suite, clean_suite );
       if ( NULL == pSuite ) {
          CU_cleanup_registry();
          return CU_get_error();
       }

       /* add the tests to the suite */
       if ( (NULL == CU_add_test(pSuite, "getSemValue_test_1", getSemValue_test_1))||
            (NULL == CU_add_test(pSuite, "getSemValue_test_2", getSemValue_test_2))||
            (NULL == CU_add_test(pSuite, "getSemValue_test_3", getSemValue_test_3))
          )
       {
          CU_cleanup_registry();
          return CU_get_error();
       }

       //insertInBuffer
       /* add a suite to the registry */
       pSuite = CU_add_suite( "insertInBuffer_test_suite", init_suite, clean_suite );
       if ( NULL == pSuite ) {
          CU_cleanup_registry();
          return CU_get_error();
       }

       /* add the tests to the suite */
       if ( (NULL == CU_add_test(pSuite, "insertInBuffer_test_1", insertInBuffer_test_1))
                 )
       {
          CU_cleanup_registry();
          return CU_get_error();
       }

       //removeFromBuffer
       /* add a suite to the registry */
       pSuite = CU_add_suite( "removeFromBuffer_test_suit", init_suite, clean_suite );
       if ( NULL == pSuite ) {
          CU_cleanup_registry();
          return CU_get_error();
       }

       /* add the tests to the suite */
       if ( (NULL == CU_add_test(pSuite, "removeFromBuffer_test_1", removeFromBuffer_test_1))                 )
       {
          CU_cleanup_registry();
          return CU_get_error();
       }

  // Run all tests using the basic interface
  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  printf("\n");
  CU_basic_show_failures(CU_get_failure_list());
  printf("\n\n");
/*
  // Run all tests using the automated interface
  CU_automated_run_tests();
  CU_list_tests_to_file();

  // Run all tests using the console interface
  CU_console_run_tests();
*/
  /* Clean up registry and return */
  CU_cleanup_registry();
  return CU_get_error();
}
