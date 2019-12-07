#ifndef _rm_h_
#define _rm_h_

#include <string>
#include <vector>
#include <map>

#include "../rbf/rbfm.h"
#include "../ix/ix.h"

# define RM_EOF (-1)  // end of a scan operator

// RM_ScanIterator is an iterator to go through tuples
class RM_ScanIterator {
    RBFM_ScanIterator rbfmIt;

public:
    RM_ScanIterator() = default;

    ~RM_ScanIterator() = default;

    void setRbfmIt(const RBFM_ScanIterator &rbfmIt) {
        RM_ScanIterator::rbfmIt = rbfmIt;
    }

    RBFM_ScanIterator& getRbfmIt() {
        return rbfmIt;
    }

    // "data" follows the same format as RelationManager::insertTuple()
    RC getNextTuple(RID &rid, void *data) { return rbfmIt.getNextRecord(rid, data); };

    RC close() { return rbfmIt.close(); };
};

// RM_IndexScanIterator is an iterator to go through index entries
class RM_IndexScanIterator {
    IX_ScanIterator ix_ScanIterator;

public:
    RM_IndexScanIterator() {};    // Constructor
    ~RM_IndexScanIterator() {};    // Destructor

    IX_ScanIterator &getIxScanIterator() {
        return ix_ScanIterator;
    }

    void setIxScanIterator(const IX_ScanIterator &ixScanIterator) {
        ix_ScanIterator = ixScanIterator;
    }

    // "key" follows the same format as in IndexManager::insertEntry()
    RC getNextEntry(RID &rid, void *key) { return ix_ScanIterator.getNextEntry(rid, key); };    // Get next matching entry
    RC close() { return ix_ScanIterator.close(); };                        // Terminate index scan
};

//Helper class used in RelationManager::processIndexesForTable method
struct IndexAttributeInfo {
    RID rid;                //rid of entry in Indexes table, so far used only in RelationManager::destroyIndexes method
    std::string filename;   //index filename associated with a table's attribute
};

// Relation Manager
class RelationManager {
public:
    static RelationManager &instance();

    RC createTableDescriptor();

    RC createColumnDescriptor();

    RC createIndexDescriptor();

    RC openFile(const std::string &tableName,FileHandle &fileHandle);

    RC createFile(const std::string &fileName);

    RC closeFile(FileHandle &fileHandle);

    RC getIdFromTableName(const std::string &tableName);

    std::string getTableFilename(const std::string &tableName);

    RC insertCatalogTableTuple(const std::string &tableName, const std::vector<Attribute> &attrs, const void *data, RID &rid);

    void createTableTableRow(const unsigned& tableID, const std::string &tableName, std::vector<byte>& bytesToWrite, bool isSystemTable);

    void createColumnTableRow(const unsigned& tableID, const Attribute &attribute, const unsigned& colPos, std::vector<byte>& bytesToWrite);

    void createIndexTableRow(const unsigned& tableID, const std::string& attributeName, const std::string& filename, std::vector<byte>& bytesToWrite);

    RC createCatalog();

    RC deleteCatalog();

    RC createTable(const std::string &tableName, const std::vector<Attribute> &attrs);

    RC createTableHelper(const std::string &tableName, const std::vector<Attribute> &attrs, bool isSystemTable);

    RC isSystemTable(const std::string& tableName, bool& isSysTable);

    RC deleteTable(const std::string &tableName);

    RC getAttributes(const std::string &tableName, std::vector<Attribute> &attrs);

    RC insertTuple(const std::string &tableName, const void *data, RID &rid);

    RC deleteTuple(const std::string &tableName, const RID &rid);

    RC updateTuple(const std::string &tableName, const void *data, const RID &rid);

    RC readTuple(const std::string &tableName, const RID &rid, void *data);

    // Print a tuple that is passed to this utility method.
    // The format is the same as printRecord().
    RC printTuple(const std::vector<Attribute> &attrs, const void *data);

    RC readAttribute(const std::string &tableName, const RID &rid, const std::string &attributeName, void *data);

    // Scan returns an iterator to allow the caller to go through the results one by one.
    // Do not store entire results in the scan iterator.
    RC scan(const std::string &tableName,
            const std::string &conditionAttribute,
            const CompOp compOp,                  // comparison type such as "<" and "="
            const void *value,                    // used in the comparison
            const std::vector<std::string> &attributeNames, // a list of projected attributes
            RM_ScanIterator &rm_ScanIterator);

    RC processIndexesForTable(const unsigned &tableID, const std::set<std::string> &attributeNames, std::map<std::string, IndexAttributeInfo>& attributes);

    // Extra credit work (10 points)
    RC addAttribute(const std::string &tableName, const Attribute &attr);

    RC dropAttribute(const std::string &tableName, const std::string &attributeName);

    // QE IX related
    RC createIndex(const std::string &tableName, const std::string &attributeName);

    RC destroyIndex(const std::string &tableName, const std::string &attributeName);

    RC destroyIndexes(const unsigned &tableID, const std::set<std::string> &attributeNames);

    // indexScan returns an iterator to allow the caller to go through qualified entries in index
    RC indexScan(const std::string &tableName,
                 const std::string &attributeName,
                 const void *lowKey,
                 const void *highKey,
                 bool lowKeyInclusive,
                 bool highKeyInclusive,
                 RM_IndexScanIterator &rm_IndexScanIterator);

protected:
    RelationManager();                                                  // Prevent construction
    ~RelationManager();                                                 // Prevent unwanted destruction
    RelationManager(const RelationManager &);                           // Prevent construction by copying
    RelationManager &operator=(const RelationManager &);                // Prevent assignment

private:
    std::vector<Attribute> tablesDescriptor; //Descriptor of 'Tables' which is  initialized when object created
    std::vector<Attribute> columnDescriptor; //Descriptor of 'Columns' which is  initialized when object created
    std::vector<Attribute> indexesDescriptor; //Descriptor of 'Indexes' which is  initialized when object created

    RC handleIndexesForInsertion(const std::string &tableName, const std::vector<Attribute> &attrs, const void *data, const RID &rid);
    RC handleIndexesForDeletion(const std::string &tableName, const std::vector<Attribute> &attrs, const RID &rid);
    RC handleIndexesForUpdate(const std::string &tableName, const std::vector<Attribute> &attrs, const void *data, const RID &rid);

    unsigned lastTableID;
    bool modifySystemTable_AdminRequest;
};

#endif
