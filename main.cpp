#include "classes.h"
#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;

int main() {

  // Create the index
  HashIndex hashIndex("EmployeeIndex.dat");
  // hashIndex.createFromFile("Employee.csv");
  hashIndex.createFromFile("Employee_large.csv");

  // Loop to lookup IDs until user is ready to quit
  std::string searchID;
  std::cout << "Enter the employee ID to find or type \"exit\" to terminate: ";
  while (std::cin >> searchID && searchID != "exit") {
    long long id = std::stoll(searchID);
    std::string record;
    hashIndex.findAndPrintEmployee(id);
  }

  return 0;
}