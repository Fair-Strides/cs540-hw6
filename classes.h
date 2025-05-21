#include <bitset>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <string>
#include <utility>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <cmath>

using namespace std;

const bool DEBUG_ENABLED = false;
const int PAGE_SIZE = 4096; // Size of each page in bytes

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
  int id, manager_id; // Employee ID and their manager's ID
  string bio, name; // Fixed length string to store employee name and biography

  Record(vector<string> &fields) {
    id = stoi(fields[0]);
    name = fields[1];
    bio = fields[2];
    manager_id = stoi(fields[3]);
  }
  // Function to get the size of the record
  int get_size() {
  // sizeof(int) is for name/bio size() in serialize function
    return sizeof(id) + sizeof(manager_id) + sizeof(int) + name.size() +
      sizeof(int) + bio.size();
  }

  // Function to serialize the record for writing to file
  string serialize() const {
    ostringstream oss;
    oss.write(reinterpret_cast<const char *>(&id), sizeof(id));
    oss.write(reinterpret_cast<const char *>(&manager_id), sizeof(manager_id));
    int name_len = name.size();
    int bio_len = bio.size();
    oss.write(reinterpret_cast<const char *>(&name_len), sizeof(name_len));
    oss.write(name.c_str(), name.size());
    oss.write(reinterpret_cast<const char *>(&bio_len), sizeof(bio_len));
    oss.write(bio.c_str(), bio.size());
    return oss.str();
  }

  void print() {
    cout << "\tID: " << id << "\n";
    cout << "\tNAME: " << name << "\n";
    cout << "\tBIO: " << bio << "\n";
    cout << "\tMANAGER_ID: " << manager_id << "\n";
  }
};

class Page {
public:
  vector<Record> records; // Data_Area containing the records
  vector<pair<int, int>> slot_directory; // Slot directory containing offset and
                                         // size of each record
  int total_records_size = sizeof(int); // Current size of the page including the overflow
                                        // page pointer. if you also write the length of slot directory change it accordingly.
  int slot_directory_size = 0;
  int overflowPointerIndex; // Initially set to -1, indicating the page has no
                            // overflow page.
                            // Update it to the position of the
                            // overflow page when one is created.
  // Constructor
  Page() : overflowPointerIndex(-1) {}

  // Function to insert a record into the page
  bool insert_record_into_page(Record r) {
    int record_size = r.get_size();
    int slot_size = sizeof(int) * 2;
    if (total_records_size + record_size + slot_directory_size + slot_size > PAGE_SIZE) {
      // Check if page size limit exceeded, considering slot directory size
      return false; // Cannot insert the record into this page
    } else {
      records.push_back(r);
      total_records_size += record_size;

      // NOTE: COMPLETE: update slot directory information
      // NOTE: this would be searched for in a real implementaion where
      //       deletion could occur
      int slot_offset = total_records_size - record_size;
      slot_directory.push_back(pair<int, int>(slot_offset, record_size));
      slot_directory_size += slot_size;

      return true;
    }
  }

  // Function to write the page to a binary output stream. You may use
  void write_into_data_file(ostream &out) const {
    char page_data[PAGE_SIZE] = { 0 }; // Buffer to hold page data
    int offset = 0;

    // Write records into page_data buffer
    for (const auto &record : records) {
      string serialized = record.serialize();
      memcpy(page_data + offset, serialized.c_str(), serialized.size());

      offset += (int)serialized.size();
    }
    auto free_space_offset = offset;

    // NOTE: Complete?:
    // - Write slot_directory in reverse order into page_data buffer.
    // - Write overflowPointerIndex into page_data buffer.
    // You should write the first entry of the slot_directory, which have the
    // info about the first record at the bottom of the page, before overflowPointerIndex.

    // zero pad the empty space
    // Calculate free space using
    // size of the page data total
    // - the offset to store the next record
    // - the size of the slot directory
    // - the space for the slot directory size and free space offset
    // - the space for the overflow page index
    size_t free_space_size =
      sizeof(page_data) - (size_t)offset - (size_t)slot_directory_size - (2 * sizeof(int)) - sizeof(int);
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

    // Now handle the Overflow Page Index, even if it's -1
    offset += write_int_to_memory(page_data + offset, overflowPointerIndex);

    assert(offset == PAGE_SIZE);

    // Write the page_data buffer to the output stream
    out.write(page_data, sizeof(page_data));
  }

  // Function to read a page from a binary input stream
  bool read_from_data_file(istream &in) {
    char page_data[PAGE_SIZE] = { 0 }; // Buffer to hold page data
    in.read(page_data, PAGE_SIZE); // Read data from input stream

    streamsize bytes_read = in.gcount();
    if (bytes_read == PAGE_SIZE) {
    // TODO: Process data to fill the records, slot_directory, and
    //       overflowPointerIndex
      return true;
    }

    if (bytes_read > 0) {
      cerr << "Incomplete read: Expected " << PAGE_SIZE << " bytes, but only read " <<
        bytes_read << " bytes." << endl;
    }

    return false;
  }
};

class HashIndex {
private:
  const size_t maxCacheSize = 1; // Maximum number of pages in the buffer
  vector<int> PageDirectory; // Map h(id) to a bucket location in
                             // EmployeeIndex(e.g., the jth bucket)
  // can scan to correct bucket using j*PAGE_SIZE as offset (using seek function)
  // can initialize to a size of 256 (assume that we will never have more than
  // 256 regular(i.e., non - overflow) buckets)
  int nextFreePage; // Next place to write a bucket
  string fileName;

  // Function to compute hash value for a given ID
  int compute_hash_value(int id) {
    int hash_value;
    // TODO: Implement the hash function h = id mod 2^8
    return hash_value;
  }

  // Function to add a new record to an existing page in the index file
  void addRecordToIndex(int pageIndex, Page &page, Record &record) {
  // Open index file in binary mode for updating
    fstream indexFile(fileName, ios::binary | ios::in | ios::out);

    if (!indexFile) {
      cerr << "Error: Unable to open index file for adding record." << endl;
      return;
    }
    // TODO:
    // - Use seekp() to seek to the offset of the correct page in the index file
    // indexFile.seekp(pageIndex * PAGE_SIZE, ios::beg);
    // - try insert_record_into_page()
    // - if it fails, then you'll need to either...
    // - go to next overflow page and try inserting there
    //   (keep doing this until you find a spot for the record)
    // - create an overflow page (if page.overflowPointerIndex == -1) using nextFreePage.
    //   update nextFreePage index and pageIndex.

    // Seek to the appropriate position in the index file
    // TODO: After inserting the record, write the modified page back to the index file.
    // Remember to use the correct position (i.e., pageIndex) if
    // you are writing out an overflow page!
    indexFile.seekp(pageIndex * PAGE_SIZE, ios::beg);


    // Close the index file
    indexFile.close();
  }

  // Function to search for a record by ID in a given page of the index file
  void searchRecordByIdInPage(int pageIndex, int id) {
  // Open index file in binary mode for reading
    ifstream indexFile(fileName, ios::binary | ios::in);

    // Seek to the appropriate position in the index file
    indexFile.seekg(pageIndex * PAGE_SIZE, ios::beg);

    // Read the page from the index file
    Page page;
    page.read_from_data_file(indexFile);

    // TODO:
    // - Search for the record by ID in the page
    // - Check for overflow pages and report if record with given ID is not found
  }

public:
  HashIndex(string indexFileName) : nextFreePage(0), fileName(indexFileName) {
  }

  // Function to create hash index from Employee CSV file
  void createFromFile(string csvFileName) {
  // Read CSV file and add records to index
  // Open the CSV file for reading
    ifstream csvFile(csvFileName);

    string line;
    // Read each line from the CSV file
    while (getline(csvFile, line)) {
    // Parse the line and create a Record object
      stringstream ss(line);
      string item;
      vector<string> fields;
      while (getline(ss, item, ',')) {
        fields.push_back(item);
      }
      Record record(fields);

      // TODO:
      // - Compute hash value for the record's ID using
      //   compute_hash_value() function.
      // - Get the page index from PageDirectory. If it's not in
      //   PageDirectory, define a new page using nextFreePage.
      // - Insert the record into the appropriate page in the index file
      //   using addRecordToIndex() function.
    }

    // Close the CSV file
    csvFile.close();
  }

  // Function to search for a record by ID in the hash index
  void findAndPrintEmployee(int id) {
  // Open index file in binary mode for reading
    ifstream indexFile(fileName, ios::binary | ios::in);

    // TODO:
    // - Compute hash value for the given ID using compute_hash_value() function
    // - Search for the record in the page corresponding to the hash value using searchRecordByIdInPage() function
    // Close the index file
    indexFile.close();
  }
};

