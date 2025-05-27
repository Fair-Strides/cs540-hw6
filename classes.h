#include <bitset>
#include <cassert>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

const bool DEBUG_ENABLED = true;
// Size of each page in bytes
const int PAGE_SIZE = 4096;

int write_int_to_memory(void *dest, int value) {
  memcpy(dest, &value, sizeof(int));
  return sizeof(int);
}

void debugf(const char *format, ...) {
  // Early return if debugging is disabled
  if (!DEBUG_ENABLED)
    return;

  va_list args;
  va_start(args, format);

  char buffer[1024];
  vsnprintf(buffer, sizeof(buffer), format, args);
  // Print "DEBUG: "
  // fprintf(stderr, "DEBUG: %s", buffer);

  // Print formatted string
  fstream logFile("DEBUG.txt", ios::out | ios::app);
  if (logFile.is_open()) {
    cerr << "DEBUG: " << buffer;
    logFile.close();
  }

  va_end(args);
}

class Record {
public:
  // Employee ID and their manager's ID
  int id, manager_id;
  // Fixed length string to store employee name and biography
  string bio, name;

  Record(int id, string name, string bio, int manager_id)
      : id(id), name(name), bio(bio), manager_id(manager_id) {}

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

  static unique_ptr<Record> deserialize(const char *data, size_t offset,
                                        size_t size) {
    int id, manager_id, name_size, bio_size;

    const char *ptr = data + offset;

    memcpy(&id, ptr, sizeof(int));
    ptr += sizeof(id);

    memcpy(&manager_id, ptr, sizeof(int));
    ptr += sizeof(manager_id);

    memcpy(&name_size, ptr, sizeof(int));
    ptr += sizeof(name_size);

    auto name = string(ptr, (size_t)name_size);
    ptr += name_size;

    memcpy(&bio_size, ptr, sizeof(int));
    ptr += sizeof(bio_size);

    auto bio = string(ptr, (size_t)bio_size);
    ptr += bio_size;

    return unique_ptr<Record>(new Record(id, name, bio, manager_id));
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
  // Data_Area containing the records
  vector<Record> records;
  // Slot directory containing offset and size of each record
  vector<pair<int, int>> slot_directory;

  // Current size of the page including the overflow page pointer. if you also
  // write the length of slot directory change it accordingly.
  int total_records_size = 0;
  int slot_directory_size = 0;
  // Initially set to -1, indicating the page has no overflow page. Update it
  // to the position of the overflow page when one is created.
  int overflowPointerIndex;

  // Constructor
  Page() : overflowPointerIndex(-1) {
    assert(this->records.size() == 0);
    assert(this->slot_directory.size() == 0);
    assert(this->total_records_size == 0);
    assert(this->slot_directory_size == 0);
    assert(this->overflowPointerIndex == -1);
  }

  // Function to insert a record into the page
  bool insert_record_into_page(Record r) {
    int record_size = r.get_size();
    int slot_size = sizeof(int) * 2;
    int total_size = total_records_size + record_size +
                     (slot_directory.size() * slot_size) + slot_size +
                     (3 * sizeof(int));
    if (total_size > PAGE_SIZE) {
      // Check if page size limit exceeded, considering slot directory size
      debugf("Cannot insert record. Total Size: %d (Total Record Size: %d, "
             "Record Size: %d, Slot Directory Size: %d, Slot Size: %d, slot "
             "directory info and overflow pointer: %d)\n",
             total_size, total_records_size, record_size,
             (slot_directory.size() * 2 * sizeof(int)), slot_size,
             (3 * sizeof(int)));
      // Cannot insert the record into this page
      return false;
    } else {
      records.push_back(std::move(r)); // TODO: does this help?
      int slot_offset = total_records_size;
      total_records_size += record_size;

      // NOTE: COMPLETE: update slot directory information
      // NOTE: this would be searched for in a real implementaion where
      //       deletion could occur
      slot_directory.emplace_back(slot_offset, record_size);
      slot_directory_size += slot_size;

      return true;
    }
  }

  // Function to write the page to a binary output stream. You may use
  void write_into_data_file(ostream &out, size_t offset) const {
    auto target_offset = offset;
    offset = 0;

    // Buffer to hold page data
    char page_data[PAGE_SIZE] = {0};

    // Write records into page_data buffer
    for (const auto &record : this->records) {
      string serialized = record.serialize();
      memcpy(page_data + offset, serialized.c_str(), serialized.size());

      offset += (int)serialized.size();
    }
    auto free_space_offset = offset;

    // NOTE: Complete:
    // - Write slot_directory in reverse order into page_data buffer.
    // - Write overflowPointerIndex into page_data buffer.
    // You should write the first entry of the slot_directory, which have the
    // info about the first record at the bottom of the page, before
    // overflowPointerIndex.

    // zero pad the empty space
    // Calculate free space using
    // size of the page data total
    // - the offset to store the next record
    // - the size of the slot directory
    // - the space for the slot directory size and free space offset
    // - the space for the overflow page index
    size_t free_space_size = sizeof(page_data) - (size_t)offset -
                             (size_t)slot_directory_size - (3 * sizeof(int));
    memset(page_data + offset, 0, free_space_size);
    offset += (int)free_space_size;

    // REVERSED:
    // overflow
    // offset
    // length of slot directory
    // slot 0
    // slot 1
    // ...
    // slot N
    for (int i = (int)this->slot_directory.size() - 1; i >= 0; i--) {
      auto slot = this->slot_directory.at((size_t)i);

      offset += write_int_to_memory(page_data + offset, slot.first);
      offset += write_int_to_memory(page_data + offset, slot.second);
    }
    offset += write_int_to_memory(page_data + offset,
                                  (int)this->slot_directory.size());
    offset += write_int_to_memory(page_data + offset, free_space_offset);

    //  Now handle the Overflow Page Index, even if it's -1
    auto pleasepleaseplease = this->overflowPointerIndex;
    offset += write_int_to_memory(page_data + offset, pleasepleaseplease);

    assert(offset == PAGE_SIZE);

    // Write the page_data buffer to the output stream
    out.flush();
    out.seekp(target_offset, ios::beg);
    out.write(page_data, sizeof(page_data));
    out.flush();
    debugf("Wrote page with %d records, total size %d, slot directory count "
           "%d, slot directory size %d, overflow pointer index %d\n",
           (int)records.size(), total_records_size, slot_directory.size(),
           slot_directory_size, overflowPointerIndex);
  }

  // Function to read a page from a binary input stream
  bool read_from_data_file(istream &in, size_t offset) {
    // Buffer to hold page data
    char page_data[PAGE_SIZE] = {0};

    // Read data from input stream
    in.seekg(offset, ios::beg);
    in.read(page_data, PAGE_SIZE);

    streamsize bytes_read = in.gcount();
    if (bytes_read != PAGE_SIZE) {
      cerr << "Incomplete read: Expected " << PAGE_SIZE
           << " bytes, but only read " << bytes_read << " bytes." << endl;
      return false;
    }

    // NOTE: Complete: Process data to fill the records, slot_directory, and
    //       overflowPointerIndex

    // do so backwards
    offset = PAGE_SIZE;

    int overflowIndex;
    memcpy(&overflowIndex, page_data + offset - sizeof(int), sizeof(int));
    offset -= sizeof(int);

    debugf("Page overflow pointer before reading: %d\n",
           this->overflowPointerIndex);
    this->overflowPointerIndex = overflowIndex;
    debugf("Page overflow pointer after reading: %d\n",
           this->overflowPointerIndex);

    int free_space_offset;
    memcpy(&free_space_offset, page_data + offset - sizeof(int), sizeof(int));
    offset -= sizeof(int);

    int slot_directory_size;
    memcpy(&slot_directory_size, page_data + offset - sizeof(int), sizeof(int));
    offset -= sizeof(int);
    this->slot_directory_size = slot_directory_size;
    debugf("Slot Directory Size: %d, Slot Directory actual size: %d, record "
           "count: %d\n",
           slot_directory_size, (int)slot_directory.size(),
           (int)records.size());
    for (int i = 0; i < slot_directory_size; i++) {
      int record_offset, record_size;

      memcpy(&record_size, page_data + offset - sizeof(int), sizeof(int));
      offset -= sizeof(int);

      memcpy(&record_offset, page_data + offset - sizeof(int), sizeof(int));
      offset -= sizeof(int);

      // Adding the record offset and size to the slot directory
      slot_directory.emplace_back(record_offset, record_size);

      // Update the total record size
      this->total_records_size += record_size;
    }
    this->slot_directory_size = slot_directory_size * sizeof(int) * 2;

    // use the slots to parse the data area
    for (const auto &slot : slot_directory) {
      records.push_back(*Record::deserialize(page_data, (size_t)slot.first,
                                             (size_t)slot.second));
    }

    debugf("Read page in with # of records %d, total size %d, "
           "slot directory size %d, overflow pointer index %d\n",
           (int)records.size(), total_records_size, slot_directory_size,
           overflowPointerIndex);

    return true;
  }
};

class HashIndex {
private:
  class FileCloser {
    std::fstream &file;

  public:
    explicit FileCloser(std::fstream &f) : file(f) {}
    ~FileCloser() { file.close(); }
  };

  // Maximum number of pages in the buffer
  const size_t maxCacheSize = 1;

  // Map h(id) to a bucket location in EmployeeIndex(e.g., the jth bucket). Can
  // scan to correct bucket using j*PAGE_SIZE as offset (using seek function).
  // Can initialize to a size of 256 (assume that we will never have more than
  // 256 regular(i.e., non - overflow) buckets)
  vector<int> PageDirectory;

  // Next place to write a bucket
  int nextFreePage;

  string fileName;

  // Function to compute hash value for a given ID
  int compute_hash_value(int id) { return id % (1 << 8); }

  // Function to add a new record to an existing page in the index file.
  //
  // Returns the page index that was inserted into
  void addRecordToIndex(int pageIndex, Page &page, Record &record) {
    // Open index file in binary mode for updating
    fstream indexFile(fileName, ios::binary | ios::in | ios::out);
    FileCloser closer(indexFile);

    // TODO:
    // - Use seekp() to seek to the offset of the correct page in the index file
    // indexFile.seekp(pageIndex * PAGE_SIZE, ios::beg);
    // - try insert_record_into_page()
    // - if it fails, then you'll need to either...
    // - go to next overflow page and try inserting there
    //   (keep doing this until you find a spot for the record)
    // - create an overflow page (if page.overflowPointerIndex == -1) using
    // nextFreePage.
    //   update nextFreePage index and pageIndex.
    debugf("Attempting to write record with ID: %d to page index: %d\n",
           record.id, pageIndex);
    while (!page.insert_record_into_page(record)) {
      debugf("Failed to insert record with ID: %d into page index: %d. "
             "Checking overflow page %d\n",
             record.id, pageIndex, page.overflowPointerIndex);
      // Since it failed, we need to see if there's an existing override page.
      if (page.overflowPointerIndex == -1) {
        // If not, then we update the page, create an overflow page,
        // and insert the record into our new page.
        page.overflowPointerIndex = nextFreePage++;
        debugf("Page with ID %d has no overflow page. Assigning new overflow "
               "page index: %d\n",
               pageIndex, page.overflowPointerIndex);

        // Write out the page now so we have our overflow page set up
        page.write_into_data_file(indexFile, pageIndex * PAGE_SIZE);
        pageIndex = page.overflowPointerIndex;

        // Reset the page since we can only have one in memory
        page = Page();
        assert(page.overflowPointerIndex == -1);
        debugf("Creating new overflow page with index: %d\n", nextFreePage - 1);
        debugf("Resetting page for overflow insertion. Now has %d records.\n",
               page.records.size());

        continue;
      }
      // If there *is* an overflow page, then we start a chain of
      // insert attempts until we either succeed or we have to make a new
      // overflow page.
      pageIndex = page.overflowPointerIndex;
      debugf("Reading overflow page with index: %d\n", pageIndex);

      page = Page();
      assert(page.overflowPointerIndex == -1);
      page.read_from_data_file(indexFile, pageIndex * PAGE_SIZE);
    }
    debugf("Successfully inserted record with ID: %d into page index: %d\n",
           record.id, pageIndex);

    // Seek to the appropriate position in the index file
    // TODO: After inserting the record, write the modified page back to the
    // index file. Remember to use the correct position (i.e., pageIndex) if you
    // are writing out an overflow page!
    debugf("Writing page with index: %d, overflow index: %d\n", pageIndex,
           page.overflowPointerIndex);
    page.write_into_data_file(indexFile, pageIndex * PAGE_SIZE);

    // Close the index file
    indexFile.close();
  }

  // Function to search for a record by ID in a given page of the index file
  unique_ptr<Record> searchRecordByIdInPage(int pageIndex, int id) {
    // Open index file in binary mode for reading
    ifstream indexFile(fileName, ios::binary | ios::in);

    // Read the page from the index file
    Page page;
    bool pageRead = false;

    // TODO:
    // - Search for the record by ID in the page
    // - Check for overflow pages and report if record with given ID is not
    // found

    // As long as we don't run into a non-existent overflow page,
    // we can seek to the right spot, read a page, and check for the record.
    // If not found, we assign the overflow page index and repeat.
    // Will exit the moment we have an overflow page that doesn't exist
    // or we find the record.
    while (pageIndex != -1) {
      // Seek to the appropriate position in the index file
      page = Page();
      pageRead = page.read_from_data_file(indexFile, pageIndex * PAGE_SIZE);

      if (!pageRead) {
        cerr << "Error! Unable to read page with index: " << pageIndex << "."
             << endl;
        return nullptr;
      }

      for (const auto &record : page.records) {
        if (record.id == id) {
          unique_ptr<Record> recordPtr(new Record(record));
          return recordPtr;
        }
      }

      // If not found in the current page, check for overflow pages
      pageIndex = page.overflowPointerIndex;
    }

    // If we reach here, the record was not found
    return nullptr;
  }

public:
  HashIndex(string indexFileName) : nextFreePage(0), fileName(indexFileName) {
    // Initialize PageDirectory with 256 buckets, with an invalid value
    PageDirectory.resize(256, -1);

    // Make sure we have an index file.
    fstream o(fileName, ios::binary | ios::in | ios::out | ios::trunc);
    if (!o.is_open()) {
      cerr << "Error: Unable to open index file." << endl;
      exit(EXIT_FAILURE);
    }
    o.close();
  }

  // Function to create hash index from Employee CSV file
  bool createFromFile(string csvFileName) {
    // Read CSV file and add records to index
    // Open the CSV file for reading
    ifstream csvFile(csvFileName);
    if (csvFile.fail()) {
      return false;
    }
    Page page = Page();

    string line;
    int recordCount = 0;
    // Read each line from the CSV file
    while (getline(csvFile, line)) {
      // Parse the line and create a Record object
      stringstream ss(line);
      string item;
      vector<string> fields;
      while (getline(ss, item, ',')) {
        fields.push_back(item);
      }
      recordCount++;
      Record record(fields);
      debugf("Processing record %d: ID=%d, Name=%s\n", recordCount, record.id,
             record.name.c_str());

      // TODO:
      // - Compute hash value for the record's ID using
      //   compute_hash_value() function.
      // - Get the page index from PageDirectory. If it's not in
      //   PageDirectory, define a new page using nextFreePage.
      // - Insert the record into the appropriate page in the index file
      //   using addRecordToIndex() function.
      int bucket = this->compute_hash_value(record.id);
      int pageIndex = this->PageDirectory.at(bucket);

      // Check if the page index is valid
      page = Page();
      if (pageIndex == -1) {
        pageIndex = this->nextFreePage;
        this->PageDirectory[bucket] = pageIndex;
        this->nextFreePage++;
      } else {
        ifstream ifs(this->fileName, ios::binary | ios::in);
        page.read_from_data_file(ifs, pageIndex * PAGE_SIZE);
        ifs.close();
      }

      // Add the record to the index file
      debugf("Adding record (ID: %d, Hash: %d) to page index: %d\n", record.id,
             bucket, pageIndex);
      this->addRecordToIndex(pageIndex, page, record);
    }

    // Close the CSV file
    csvFile.close();

    return true;
  }

  // Function to search for a record by ID in the hash index
  bool findAndPrintEmployee(int id) {
    // Open index file in binary mode for reading
    ifstream indexFile(fileName, ios::binary | ios::in);

    // TODO:
    // - Compute hash value for the given ID using [compute_hash_value] function
    // - Search for the record in the page corresponding to the hash value using
    // [searchRecordByIdInPage] function Close the index file

    unique_ptr<Record> record = unique_ptr<Record>();

    auto hash = this->compute_hash_value(id);
    debugf("Searching for record with ID: %d, hash value: %d\n", id, hash);
    if (hash < this->PageDirectory.size()) {
      int pageIndex = this->PageDirectory.at(hash);
      debugf("Searching for record with ID: %d in page index: %d and hash "
             "value: %d\n",
             id, pageIndex, hash);
      // TODO: should this be a sentinel?
      if (pageIndex != -1) {
        record = this->searchRecordByIdInPage(pageIndex, id);
      }
    }

    indexFile.close();
    if (record) {
      printf("ID: %d\tName: %s\tBio: %s\tManager: %d\n", record->id,
             record->name.c_str(), record->bio.c_str(), record->manager_id);
      // record->print();
      // cout << "Record found: " << id << endl;
      return true;
    } else {
      cout << "Record not found: " << id << endl;
      return false;
    }
  }
};
