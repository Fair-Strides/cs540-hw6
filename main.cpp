#include "classes.h"
#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;

int main() {

  // Create the index
  HashIndex hashIndex("EmployeeIndex");
  hashIndex.createFromFile("Employee.csv");

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

/*
HW 5 Code
Skeleton code for External storage management
*/

/*
#include "classes_hw5.h"
#include <cstdlib>
#include <string>
using namespace std;

int main(int argc, char *const argv[]) {

  // Initialize the Storage Manager Class with the Binary .dat file name we want
  // to create
  StorageManager manager("EmployeeRelation.dat");

  // Assuming the Employee.CSV file is in the same directory,
  // we want to read from the Employee.csv and write into the new data_file
  manager.createFromFile("Employee.csv");

  // You'll receive employee IDs as arguments. For each, process it to retrieve
  // the record, or display a message if not found.
  bool success = true;
  for (int i = 1; i < argc; ++i) {
    int emp_id = stoi(argv[i]);
    success &= manager.findAndPrintEmployee(emp_id);
  }

  if (success) {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}
*/
