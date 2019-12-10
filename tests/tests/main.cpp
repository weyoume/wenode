
#include <cstdlib>
#include <iostream>
#include <boost/test/included/unit_test.hpp>

// extern uint32_t TESTING_GENESIS_TIMESTAMP;

boost::unit_test::test_suite* init_unit_test_suite(int argc, char* argv[])
{
   std::srand(time(NULL));
   std::cout << "Random number generator seeded to " << time(NULL) << std::endl;
/*
   const char* genesis_timestamp_str = getenv("TESTING_GENESIS_TIMESTAMP");
   if( genesis_timestamp_str != nullptr )
   {
      TESTING_GENESIS_TIMESTAMP = std::stoul( genesis_timestamp_str );
   }
   std::cout << "TESTING_GENESIS_TIMESTAMP is " << TESTING_GENESIS_TIMESTAMP << std::endl;
*/
   return nullptr;
}
