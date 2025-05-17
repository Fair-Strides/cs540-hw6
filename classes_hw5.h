/*** This is just a Skeleton/Starter Code for the External Storage Assignment.
 * This is by no means absolute, in terms of assignment approach/ used
 * functions, etc. ***/
/*** You may modify any part of the code, as long as you stick to the
 * assignments requirements we do not have any issue ***/

// Include necessary standard library headers
#include <bitset>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const bool DEBUG_ENABLED = false;

int write_int_to_memory(void *dest, int value) {
  auto str = reinterpret_cast<const char *>(value);
  memcpy(dest, &str, sizeof(int));
  return sizeof(int);
}

void debugf(const char *format, ...) {
  // Early return if debugging is disabled
  if (!DEBUG_ENABLED)
    return;

  va_list args;
  va_start(args, format);

  // Print "DEBUG: "
  fprintf(stderr, "DEBUG: ");

  // Print formatted string
  vfprintf(stderr, format, args);

  va_end(args);
}

class Record {
public:
  // Employee ID and their manager's ID
  int id, manager_id;
  // Fixed length string to store employee name and biography
  std::string bio, name;

  Record(vector<std::string> &fields) {
    id = stoi(fields[0]);
    name = fields[1];
    bio = fields[2];
    manager_id = stoi(fields[3]);
  }

  // You may use this for debugging / showing the record to standard output.
  void print() {
    // NOTE: Alternate output for our python tests
    // printf("ID: %d, NAME: %s, BIO: %s, MANAGER_ID: %d\n", id, name.c_str(),
    //  bio.c_str(), manager_id);
    cout << "\tID: " << id << "\n";
    cout << "\tNAME: " << name << "\n";
    cout << "\tBIO: " << bio << "\n";
    cout << "\tMANAGER_ID: " << manager_id << "\n";
  }

  // Function to get the size of the record
  int get_size() {
    // sizeof(int) is for name/bio size() in serialize function
    return (int)(sizeof(id) + sizeof(manager_id) + sizeof(int) + name.size() +
      sizeof(int) + bio.size());
  }

  // Take a look at Figure 9.9 and read the Section 9.7.2 [Record Organization
  // for Variable Length Records]
  string serialize() const {
    ostringstream oss;
    int name_len = (int)name.size();
    int bio_len = (int)bio.size();

    // Writes the binary representation of the ID.
    oss.write(reinterpret_cast<const char *>(&id), sizeof(id));

    // Writes the binary representation of the Manager id
    oss.write(reinterpret_cast<const char *>(&manager_id), sizeof(manager_id));

    // Writes the size of the Name in binary format.
    oss.write(reinterpret_cast<const char *>(&name_len), sizeof(name_len));

    // writes the name in binary form
    oss.write(name.c_str(), (std::streamsize)name.size());

    // Writes the size of the Bio in binary format.
    oss.write(reinterpret_cast<const char *>(&bio_len), sizeof(bio_len));

    // writes bio in binary form
    oss.write(bio.c_str(), (std::streamsize)bio.size());

    return oss.str();
  }

  static Record *deserialize(const char *data, size_t offset, size_t size) {
    istringstream iss(string(data + offset, size));
    // char* to integer directly without a string intermediary
    int id, manager_id, name_size, bio_size;

    // Read the binary representation of the ID.
    iss.read(reinterpret_cast<char *>(&id), sizeof(int));

    // Read the binary representation of the Manager id
    iss.read(reinterpret_cast<char *>(&manager_id), sizeof(int));

    // Read the size of the Name in binary format, then retrieve it
    iss.read(reinterpret_cast<char *>(&name_size), sizeof(int));
    string name((size_t)name_size, '\0');
    iss.read(&name[0], name_size);

    // Read the size of the Bio in binary format, then retrieve it
    iss.read(reinterpret_cast<char *>(&bio_size), sizeof(int));
    string bio((size_t)bio_size, '\0');
    iss.read(&bio[0], bio_size);

    // Create a new Record object with the deserialized data
    auto fields =
      vector<string>{ to_string(id), name, bio, to_string(manager_id) };
    auto record = new Record(fields);
    return record;
  }
};

// Take a look at Figure 9.7 and read Section 9.6.2 [Page
// organization for variable length records]
class page {
public:
  // Data Area: Stores records.
  vector<Record> records;

  // This slot directory contains the starting position
  // (offset), and size of the record.
  vector<pair<int, int>> slot_directory;

  // holds the current size of the page
  int total_records_size = 0;
  int slot_directory_size = 0;

  // Function to insert a record into the page
  bool insert_record_into_page(Record r) {
    int record_size = r.get_size();
    int slot_size = sizeof(int) * 2;

    debugf("Total size of records for page: %d\n",
      total_records_size);
    // Check if page size limit exceeded, considering slot directory size
    if (total_records_size + record_size + slot_directory_size + slot_size >
      4096) {
    // Cannot insert the record into this page
      return false;
    } else {
      // Record stored in current page
      records.push_back(r);

      // Updating page size
      total_records_size += r.get_size();

      // NOTE: COMPLETE: update slot directory information
      // NOTE: this would be searched for in a real implementaion where
      //       deletion could occur
      int slot_offset = total_records_size - record_size;
      slot_directory.push_back(pair<int, int>(slot_offset, record_size));
      slot_directory_size += slot_size;

      return true;
    }
  }

  // Function to write the page to a binary file, i.e., EmployeeRelation.dat
  // file
  void write_into_data_file(ostream &out) const {
    debugf("writing to data file\n");

    // Write the page contents (records and slot directory) into this char
    // array so that the page can be written to the data file in one go.
    char page_data[4096] = { 0 };

    // Used as an iterator to indicate where the next item
    // should be stored. Section 9.6.2 contains information that
    // will help you with the implementation.
    int offset = 0;

    // Writing the records into the page_data
    for (const auto &record : records) {
      string serialized = record.serialize();

      memcpy(page_data + offset, serialized.c_str(), serialized.size());

      offset += (int)serialized.size();
    }
    auto free_space_offset = offset;

    // NOTE: COMPLETE: Write out the slot directory. Optimally, it should start
    // at the end of the page and grow backwards

    // zero pad the empty space
    size_t free_space_size =
      sizeof(page_data) - (size_t)offset - (size_t)slot_directory_size - (2 * sizeof(int));
    debugf("Writing the following sizes: records=%d free=%zd slots=%d\n",
      offset, free_space_size, slot_directory_size);
    memset(page_data + offset, 0, free_space_size);
    offset += (int)free_space_size;
    debugf("write free space\n");

    // REVERSED:
    // offset
    // length of slot directory
    // slot 0
    // slot 1
    // ...
    // slot N
    for (int i = (int)slot_directory.size() - 1; i >= 0; i--) {
      auto slot = slot_directory.at((size_t)i);

      offset += write_int_to_memory(page_data + offset, slot.first);
      offset += write_int_to_memory(page_data + offset, slot.second);
    }
    debugf("writing int offset=%d value=%zd\n", offset, slot_directory.size());
    offset += write_int_to_memory(page_data + offset, (int)slot_directory.size());
    offset += write_int_to_memory(page_data + offset, free_space_offset);

    assert(offset == 4096);

    // Write the page_data to the EmployeeRelation.dat file
    out.write(page_data, sizeof(page_data));
  }

  // Read a page from a binary input stream, i.e., EmployeeRelation.dat file to
  // populate a page object
  bool read_from_data_file(istream &in) {
    // Character array used to read 4 KB from the data file to your main
    // memory.
    char page_data[4096] = { 0 };

    // Read a page of 4 KB from the data file
    in.read(page_data, 4096);

    // used to check if 4KB was actually read from the data file
    streamsize bytes_read = in.gcount();
    if (bytes_read == 4096) {

      // NOTE: You may process page_data (4 KB page) and put the information to
      // the records and slot_directory (main memory).

      size_t offset = sizeof(page_data);

      int free_space_offset;
      memcpy(&free_space_offset, page_data + offset - sizeof(int), sizeof(int));
      offset -= sizeof(int);

      int slot_directory_size;
      memcpy(&slot_directory_size, page_data + offset - sizeof(int),
        sizeof(int));
      offset -= sizeof(int);

      for (int i = 0; i < slot_directory_size; i++) {
        int record_offset, record_size;

        memcpy(&record_size, page_data + offset - sizeof(int), sizeof(int));
        offset -= sizeof(int);

        memcpy(&record_offset, page_data + offset - sizeof(int), sizeof(int));
        offset -= sizeof(int);

        // Adding the record offset and size to the slot directory
        slot_directory.push_back(pair<int, int>(record_offset, record_size));
      }

      // use the slots to parse the data area
      for (const auto &slot : slot_directory) {
        records.push_back(
          *Record::deserialize(page_data, (size_t)slot.first, (size_t)slot.second));
      }

      return true;
    }

    if (bytes_read > 0) {
      cerr << "Incomplete read: Expected " << 4096 << " bytes, but only read "
        << bytes_read << " bytes." << endl;
    }

    return false;
  }

  void reset() {
    // Clear the records and slot directory
    records.clear();
    slot_directory.clear();
    total_records_size = 0;
    slot_directory_size = 0;
  }
};

class StorageManager {

public:
  // Name of the file (EmployeeRelation.dat) where we will
  // store the Pages
  string filename;

  // fstream to handle both input and output binary file
  // operations
  fstream data_file;

  // You can have maximum of 3 Pages.
  vector<page> buffer;

  // Constructor that opens a data file for binary input/output; truncates any
  // existing data file
  StorageManager(const string &filename) : filename(filename) {
    data_file.open(filename, ios::binary | ios::out | ios::in | ios::trunc);

    // Check if the data_file was successfully opened
    if (!data_file.is_open()) {
      cerr << "Failed to open data_file: " << filename << endl;

      // Exit if the data_file cannot be opened
      exit(EXIT_FAILURE);
    }
  }

  // Destructor closes the data file if it is still open
  ~StorageManager() {
    if (data_file.is_open()) {
      data_file.close();
    }
  }

  // Reads data from a CSV file and writes it to EmployeeRelation.dat
  void createFromFile(const string &csvFilename) {
    debugf("creating from file\n");
    // You can have maximum of 3 Pages.
    buffer.resize(3);

    // Open the Employee.csv file for reading
    ifstream csvFile(csvFilename);

    if (csvFile.fail()) {
      printf("could not read csv file");
      exit(EXIT_FAILURE);
    }

    // Current page we are working on [at most 3 pages]
    string line;
    int page_number = 0;

    // Read each line from the CSV file, parse it, and create Employee objects
    while (getline(csvFile, line)) {
      debugf("reading line from file\n");
      stringstream ss(line);
      string item;
      vector<string> fields;

      while (getline(ss, item, ',')) {
        fields.push_back(item);
      }
      // create a record object
      Record r = Record(fields);

      debugf("inserting record into page...\n");
      // inserting that record object to the current page
      if (!buffer[(std::vector<page>::size_type)page_number].insert_record_into_page(r)) {
        debugf("inserting record into page failed! page is full\n");

        // Current page is full, move to the next page
        page_number++;

        // Checking if page limit has been reached.
        if (page_number >= (int)buffer.size()) {
          debugf("buffer limit reached\n");

          // using write_into_data_file() to write the pages into the data file
          for (page &p : buffer) {
            p.write_into_data_file(data_file);

            // Resetting the page to avoid reusing old data
            p.reset();
          }
          // Starting again from page 0
          page_number = 0;
        }
        // Reattempting the insertion of record 'r' into the newly created page
        buffer[(std::vector<page>::size_type)page_number].insert_record_into_page(r);
      }
    }

    // NOTE: make sure to write out any records left over in memory
    for (int i = 0; i <= page_number; i++) {
      auto &p = buffer.at((size_t)i);
      p.write_into_data_file(data_file);

      // Resetting the page
      p.reset();
    }

    debugf("closing data file\n");
    // Close the CSV file
    csvFile.close();
  }

  // Searches for an Employee ID in EmployeeRelation.dat
  bool findAndPrintEmployee(int searchId) {
    buffer.clear();
    buffer.resize(3);

    // Rewind the data_file to the beginning for reading
    data_file.seekg(0, ios::beg);

    // NOTE: Read pages from your data file (using read_from_data_file) and
    // search for the employee ID in those pages. Be mindful of the page limit
    // in main memory.
    int page_number = 0;
    while (buffer[(std::vector<page>::size_type)page_number].read_from_data_file(data_file)) {
      // Tryss Note: Since this grabs a whole page or fails with an error
      // message, we know we will always have a page at this point in the loop.
      // We also need to bear in mind that we can only have 3 pages in memory
      // at a time. Since we start at 0, and we enter the loop with a new page,
      // we need to clear the buffer if we'd ever go beyond 2.
      auto page = buffer.at((size_t)page_number);
      for (auto &record : page.records) {
        if (record.id == searchId) {
          record.print();
          return true;
        }
      }

      page_number++;
      if (page_number >= (int)buffer.size()) {
        page_number = 0;

        for (auto &p : buffer) {
          // Resetting the page
          p.reset();
        }
      }
    }

    // NOTE: Print "Record not found" if no records match.
    cout << "Record not found" << endl;
    return false;
  }
};
