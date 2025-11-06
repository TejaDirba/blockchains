#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cstdint>

using std::string;
using std::vector;
using std::cout;
using std::endl;

class CustomHash {
private:
    static const uint64_t INIT_A = 0x428a2f98d728ae22ULL;
    static const uint64_t INIT_B = 0x7137449123ef65cdULL;
    static const uint64_t INIT_C = 0xb5c0fbcfec4d3b2fULL;
    static const uint64_t INIT_D = 0xe9b5dba58189dbbcULL;
    static const uint64_t PRIME1 = 0x9e3779b185ebca87ULL;
    static const uint64_t PRIME2 = 0xc2b2ae3d27d4eb4fULL;
    static const uint64_t PRIME3 = 0x165667b19e3779f9ULL;
    static const uint64_t PRIME4 = 0x85ebca77c2b2ae63ULL;
    
    static uint64_t rotateLeft(uint64_t v, int s) {
        return (v << s) | (v >> (64 - s));
    }
    
    static uint64_t rotateRight(uint64_t v, int s) {
        return (v >> s) | (v << (64 - s));
    }
    
    static uint64_t mix(uint64_t a, uint64_t b, uint64_t c) {
        a ^= b;
        a = rotateLeft(a, 23);
        a *= PRIME1;
        a ^= c;
        return rotateLeft(a, 41);
    }

public:
    static string hash(const string& input) {
        uint64_t h1 = INIT_A, h2 = INIT_B, h3 = INIT_C, h4 = INIT_D;
        size_t len = input.length();
        
        for (size_t i = 0; i < len; i++) {
            uint64_t b = static_cast<uint64_t>(static_cast<unsigned char>(input[i])) 
                        ^ ((i + 1) * PRIME1);
            
            h1 ^= b * PRIME1;
            h1 = rotateLeft(h1, 13);
            h1 *= PRIME2;
            
            h2 ^= b * PRIME2;
            h2 = rotateRight(h2, 17);
            h2 += h1;
            
            h3 ^= b * PRIME3;
            h3 = rotateLeft(h3, 31);
            h3 ^= h2;
            
            h4 ^= b * PRIME4;
            h4 = rotateRight(h4, 19);
            h4 += h3;
            
            if (i % 4 == 3) {
                h1 = mix(h1, h2, h3);
                h2 = mix(h2, h3, h4);
                h3 = mix(h3, h4, h1);
                h4 = mix(h4, h1, h2);
            }
        }
        
        h1 ^= len * PRIME1;
        h2 ^= len * PRIME2;
        h3 ^= len * PRIME3;
        h4 ^= len * PRIME4;
        
        h1 = mix(h1, h2, h3);
        h2 = mix(h2, h3, h4);
        h3 = mix(h3, h4, h1);
        h4 = mix(h4, h1, h2);
        
        h1 += h2 + h3 + h4;
        h2 += h1;
        h3 += h1;
        h4 += h1;
        
        std::stringstream ss;
        ss << std::hex << std::setfill('0')
           << std::setw(16) << h1
           << std::setw(16) << h2
           << std::setw(16) << h3
           << std::setw(16) << h4;
        
        return ss.str();
    }
};

string getTimestamp() {
    std::time_t now = std::time(nullptr);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return string(buf);
}

void printSeparator(char c = '=', int len = 80) {
    cout << string(len, c) << endl;
}

void printHeader(const string& text) {
    printSeparator();
    cout << "  " << text << endl;
    printSeparator();
}


class Transaction {
private:
    string txId;
    string sender;
    string receiver;
    uint64_t amount;

public:
    Transaction(const string& from, const string& to, uint64_t amt) 
        : sender(from), receiver(to), amount(amt) {
        string data = from + "|" + to + "|" + std::to_string(amt);
        txId = CustomHash::hash(data);
    }

    string getId() const { return txId; }
    string getSender() const { return sender; }
    string getReceiver() const { return receiver; }
    uint64_t getAmount() const { return amount; }

    void print(int index = -1) const {
        if (index >= 0) {
            cout << "    [TX " << index << "] ";
        } else {
            cout << "    ";
        }
        cout << sender.substr(0, 12) << "... -> " 
             << receiver.substr(0, 12) << "... : " 
             << amount << " units" << endl;
    }
};

class BlockHeader {
private:
    string prevBlockHash;
    uint64_t timestamp;
    uint32_t version;
    string txRootHash;
    uint64_t nonce;
    string difficulty;

public:
    BlockHeader() : timestamp(0), version(1), nonce(0), difficulty("000") {
        prevBlockHash = string(64, '0');
    }

    void setPrevHash(const string& hash) { prevBlockHash = hash; }
    void setTimestamp(uint64_t ts) { timestamp = ts; }
    void setVersion(uint32_t v) { version = v; }
    void setTxRootHash(const string& hash) { txRootHash = hash; }
    void setNonce(uint64_t n) { nonce = n; }
    void setDifficulty(const string& diff) { difficulty = diff; }

    string getPrevHash() const { return prevBlockHash; }
    uint64_t getTimestamp() const { return timestamp; }
    uint32_t getVersion() const { return version; }
    string getTxRootHash() const { return txRootHash; }
    uint64_t getNonce() const { return nonce; }
    string getDifficulty() const { return difficulty; }

    string computeHash() const {
        string data = prevBlockHash + "|" +
                     std::to_string(timestamp) + "|" +
                     std::to_string(version) + "|" +
                     txRootHash + "|" +
                     difficulty + "|" +
                     std::to_string(nonce);
        return CustomHash::hash(data);
    }
};

class Block {
private:
    BlockHeader header;
    vector<Transaction> transactions;
    string blockHash;
    int blockNumber;

public:
    Block(int num) : blockNumber(num) {}

    BlockHeader& getHeader() { return header; }
    const BlockHeader& getHeader() const { return header; }
    
    void addTransaction(const Transaction& tx) {
        transactions.push_back(tx);
    }

    const vector<Transaction>& getTransactions() const {
        return transactions;
    }

    void computeTxRootHash() {
        string combined;
        for (const auto& tx : transactions) {
            combined += tx.getId();
        }
        string root = CustomHash::hash(combined);
        header.setTxRootHash(root);
    }

    bool mine(uint64_t maxIterations = 10000000) {
        string target = header.getDifficulty();
        
        cout << "\n  Mining block #" << blockNumber << "..." << endl;
        cout << "     Target: " << target << "..." << endl;
        cout << "     Transactions: " << transactions.size() << endl;
        
        auto startTime = std::time(nullptr);
        
        for (uint64_t i = 0; i < maxIterations; i++) {
            header.setNonce(i);
            blockHash = header.computeHash();
            
            if (blockHash.substr(0, target.length()) == target) {
                auto endTime = std::time(nullptr);
                cout << "  Block mined successfully!" << endl;
                cout << "     Nonce: " << i << endl;
                cout << "     Hash: " << blockHash.substr(0, 20) << "..." << endl;
                cout << "     Time: " << (endTime - startTime) << " seconds" << endl;
                return true;
            }
            
            if (i % 100000 == 0 && i > 0) {
                cout << "     Attempts: " << i << "..." << endl;
            }
        }
        
        cout << "  Mining failed after " << maxIterations << " attempts" << endl;
        return false;
    }

    string getHash() const { return blockHash; }
    int getNumber() const { return blockNumber; }

    void print() const {
        printSeparator('-', 80);
        cout << "  BLOCK #" << blockNumber << endl;
        printSeparator('-', 80);
        cout << "  Hash: " << blockHash.substr(0, 24) << "..." << endl;
        cout << "  Previous Hash: " << header.getPrevHash().substr(0, 24) << "..." << endl;
        cout << "  Timestamp: " << getTimestamp() << endl;
        cout << "  Nonce: " << header.getNonce() << endl;
        cout << "  Difficulty: " << header.getDifficulty() << endl;
        cout << "  Transactions: " << transactions.size() << endl;
        cout << "  TX Root Hash: " << header.getTxRootHash().substr(0, 24) << "..." << endl;
        cout << "\n  First 5 Transactions:" << endl;
        
        int displayCount = std::min((int)transactions.size(), 5);
        for (int i = 0; i < displayCount; i++) {
            transactions[i].print(i);
        }
        if (transactions.size() > 5) {
            cout << "    ... and " << (transactions.size() - 5) << " more" << endl;
        }
    }
};

class User {
private:
    string name;
    string publicKey;
    uint64_t balance;

public:
    User() : name(""), publicKey(""), balance(0) {}
    
    User(const string& n, const string& pk, uint64_t bal) 
        : name(n), publicKey(pk), balance(bal) {}

    string getName() const { return name; }
    string getPublicKey() const { return publicKey; }
    uint64_t getBalance() const { return balance; }
    
    void setBalance(uint64_t bal) { balance = bal; }
    void addBalance(uint64_t amt) { balance += amt; }
    bool subtractBalance(uint64_t amt) {
        if (balance >= amt) {
            balance -= amt;
            return true;
        }
        return false;
    }
};

class Blockchain {
private:
    vector<Block> chain;
    vector<Transaction> mempool;
    std::unordered_map<string, User> users;
    string difficulty;
    int txPerBlock;

    string generatePublicKey(std::mt19937_64& rng) {
        string key = "pk_";
        for (int i = 0; i < 24; i++) {
            int val = rng() % 16;
            key += "0123456789abcdef"[val];
        }
        return key;
    }

    string generateName(std::mt19937_64& rng) {
        static const vector<string> names = {
            "Aiste", "Mantas", "Ieva", "Lukas", "Ugne", 
            "Tomas", "Goda", "Kasparas", "Emilija", "Jonas",
            "Paulius", "Gabija", "Domas", "Austeja", "Matas"
        };
        return names[rng() % names.size()] + "_" + std::to_string(rng() % 100000);
    }

public:
    Blockchain(const string& diff = "000", int txPerBlk = 100) 
        : difficulty(diff), txPerBlock(txPerBlk) {}

    void generateUsers(int count) {
        printHeader("GENERATING USERS");
        cout << "  Creating " << count << " users..." << endl;
        
        std::mt19937_64 rng(123456789);
        std::uniform_int_distribution<uint64_t> balanceDist(100, 1000000);
        
        for (int i = 0; i < count; i++) {
            string pk = generatePublicKey(rng);
            string name = generateName(rng);
            uint64_t balance = balanceDist(rng);
            
            users[pk] = User(name, pk, balance);
        }
        
        cout << "  Generated " << users.size() << " users" << endl;
        cout << "\n  Sample users:" << endl;
        
        int shown = 0;
        for (const auto& pair : users) {
            if (shown++ >= 5) break;
            cout << "    " << pair.second.getName() 
                 << " (" << pair.first.substr(0, 16) << "...) : "
                 << pair.second.getBalance() << " units" << endl;
        }
    }

    void generateTransactions(int count) {
        printHeader("GENERATING TRANSACTIONS");
        cout << "  Creating " << count << " transactions..." << endl;
        
        if (users.size() < 2) {
            cout << "  Not enough users!" << endl;
            return;
        }
        
        vector<string> keys;
        for (const auto& pair : users) {
            keys.push_back(pair.first);
        }
        
        std::mt19937_64 rng(987654321);
        std::uniform_int_distribution<size_t> userDist(0, keys.size() - 1);
        std::uniform_int_distribution<uint64_t> amountDist(1, 50000);
        
        for (int i = 0; i < count; i++) {
            string sender = keys[userDist(rng)];
            string receiver = keys[userDist(rng)];
            
            while (receiver == sender) {
                receiver = keys[userDist(rng)];
            }
            
            uint64_t amount = amountDist(rng);
            mempool.push_back(Transaction(sender, receiver, amount));
        }
        
        cout << "  Generated " << mempool.size() << " transactions" << endl;
        cout << "\n  Sample transactions:" << endl;
        
        for (int i = 0; i < std::min(5, (int)mempool.size()); i++) {
            mempool[i].print(i);
        }
    }

    void mineNextBlock() {
        if (mempool.empty()) {
            cout << "\n  Mempool is empty!" << endl;
            return;
        }
        
        printHeader("MINING NEW BLOCK");
        
        vector<Transaction> selectedTx;
        int toTake = std::min(txPerBlock, (int)mempool.size());
        
        std::sort(mempool.begin(), mempool.end(), 
                 [](const Transaction& a, const Transaction& b) {
                     return a.getAmount() > b.getAmount();
                 });
        
        for (int i = 0; i < toTake; i++) {
            selectedTx.push_back(mempool[i]);
        }
        
        int blockNum = chain.size();
        Block newBlock(blockNum);
        
        if (blockNum == 0) {
            newBlock.getHeader().setPrevHash(string(64, '0'));
        } else {
            newBlock.getHeader().setPrevHash(chain.back().getHash());
        }
        
        newBlock.getHeader().setTimestamp(std::time(nullptr));
        newBlock.getHeader().setVersion(1);
        newBlock.getHeader().setDifficulty(difficulty);
        
        for (const auto& tx : selectedTx) {
            newBlock.addTransaction(tx);
        }
        
        newBlock.computeTxRootHash();
        
        if (newBlock.mine()) {
            applyBlock(newBlock);
            chain.push_back(newBlock);
            mempool.erase(mempool.begin(), mempool.begin() + toTake);
        }
    }

    void applyBlock(const Block& block) {
        cout << "\n  Applying transactions..." << endl;
        
        for (const auto& tx : block.getTransactions()) {
            string sender = tx.getSender();
            string receiver = tx.getReceiver();
            uint64_t amount = tx.getAmount();
            
            if (users.find(sender) != users.end() && 
                users.find(receiver) != users.end()) {
                
                if (users[sender].getBalance() >= amount) {
                    users[sender].subtractBalance(amount);
                    users[receiver].addBalance(amount);
                }
            }
        }
        
        cout << "  Transactions applied" << endl;
    }

    void printSummary() const {
        printHeader("BLOCKCHAIN SUMMARY");
        cout << "  Total Blocks: " << chain.size() << endl;
        cout << "  Remaining Transactions: " << mempool.size() << endl;
        cout << "  Total Users: " << users.size() << endl;
        cout << "  Difficulty: " << difficulty << endl;
        
        if (!chain.empty()) {
            cout << "\n  Latest Block:" << endl;
            cout << "    Number: " << chain.back().getNumber() << endl;
            cout << "    Hash: " << chain.back().getHash().substr(0, 24) << "..." << endl;
            cout << "    Transactions: " << chain.back().getTransactions().size() << endl;
        }
    }

    int getMempoolSize() const { return mempool.size(); }
};

int main() {
    printHeader("BLOCKCHAIN v0.1 - CENTRALIZED SIMULATION");
    
    cout << "\n  Starting blockchain simulation..." << endl;
    cout << "  Timestamp: " << getTimestamp() << endl;
    
    Blockchain blockchain("000", 100);
    
    blockchain.generateUsers(1000);
    blockchain.generateTransactions(10000);
    
    cout << "\n";
    printHeader("STARTING MINING PROCESS");
    
    for (int i = 0; i < 10 && blockchain.getMempoolSize() > 0; i++) {
        blockchain.mineNextBlock();
        
        if ((i + 1) % 2 == 0) {
            cout << "\n  Progress: " << (i + 1) << " blocks mined" << endl;
        }
    }
    
    cout << "\n";
    blockchain.printSummary();
    
    printSeparator();
    cout << "  Blockchain simulation completed!" << endl;
    printSeparator();
    
    return 0;
}


