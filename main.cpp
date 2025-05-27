#include "classes.h"
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

int main() {
  bool success = true;

  // Create the index
  HashIndex hashIndex("EmployeeIndex.dat");
  // hashIndex.createFromFile("Employee.csv");
  if (!hashIndex.createFromFile("Employee.csv"))
    return EXIT_FAILURE;

  // Loop to lookup IDs until user is ready to quit
  std::string searchID;
  // TODO: replace
  // std::cout << "Enter the employee ID to find or type \"exit\" to terminate:
  // ";
  while (std::cin >> searchID && searchID != " exit ") {
    int id = std::stoi(searchID);
    std::string record;

    // printf("RUNNING WITH id=%d\n", id);

    success &= hashIndex.findAndPrintEmployee(id);
  }

  // hashIndex.processPages();

  if (success) {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}
