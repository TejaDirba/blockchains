#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <ctime>
#include <sstream>
#include <iomanip>

using namespace std;

class Hash {
public:
    static string calculate(const string& text) {
        unsigned long long h = 5381;
        
        for (char c : text) {
            h = ((h << 5) + h) + c; // h * 33 + c
        }
        
        // Padarom 64 simbolių ilgio hex stringą
        stringstream ss;
        ss << hex << setfill('0') << setw(16) << h;
        string result = ss.str();
        
        // Prailginam iki 64 simbolių
        while (result.length() < 64) {
            result += result;
        }
        return result.substr(0, 64);
    }
};

// ============================================================================
// 2. TRANSAKCIJA (PINIGŲ PERVEDIMAS)
// Pvz: Jonas siunčia Petrai 100 eurų
// ============================================================================

class Transaction {
public:
    string id;          // Unikalus ID
    string from;        // Kas siunčia
    string to;          // Kas gauna
    int amount;         // Kiek siunčia
    
    // Konstruktorius - sukuria naują transakciją
    Transaction(string sender, string receiver, int money) {
        from = sender;
        to = receiver;
        amount = money;
        
        // ID = hash iš visų duomenų
        string data = from + to + std::to_string(amount);
        id = Hash::calculate(data);
    }
    
    // Atspausdinti gražiai
    void print() {
        cout << "      " << from.substr(0, 10) << "... -> " 
             << to.substr(0, 10) << "... : " << amount << " EUR\n";
    }
};

// ============================================================================
// 3. BLOKAS
// Tai kaip "dėžė" su daug transakcijų viduje
// ============================================================================

class Block {
public:
    int number;                      // Bloko numeris (0, 1, 2, ...)
    string previousHash;             // Ankstesnio bloko hash
    vector<Transaction> transactions; // Transakcijos
    long timestamp;                  // Kada sukurtas
    int nonce;                       // "Magic" skaičius mining'ui
    string hash;                     // Šio bloko hash
    
    // Konstruktorius
    Block(int num, string prevHash) {
        number = num;
        previousHash = prevHash;
        timestamp = time(nullptr);
        nonce = 0;
        hash = "";
    }
    
    // Pridėti transakciją
    void addTransaction(Transaction tx) {
        transactions.push_back(tx);
    }
    
    // Apskaičiuoti bloko hash
    string calculateHash() {
        // Sujungiame visus duomenis
        string data = previousHash + 
                     std::to_string(timestamp) + 
                     std::to_string(nonce);
        
        // Pridedame visas transakcijas
        for (auto& tx : transactions) {
            data += tx.id;
        }
        
        return Hash::calculate(data);
    }
    
    // MINING - ieškome hash'o, kuris prasideda "000"
    bool mine() {
        cout << "\n  Mining block #" << number << "...\n";
        cout << "      Transactions: " << transactions.size() << "\n";
        cout << "      Looking for hash starting with 000...\n";
        
        time_t startTime = time(nullptr);
        
        // Bandome skirtingus nonce, kol radom tinkamą hash
        for (nonce = 0; nonce < 10000000; nonce++) {
            hash = calculateHash();
            
            // Ar hash prasideda "000"?
            if (hash[0] == '0' && hash[1] == '0' && hash[2] == '0') {
                time_t endTime = time(nullptr);
                cout << "  Block mined!\n";
                cout << "      Nonce: " << nonce << "\n";
                cout << "      Hash: " << hash.substr(0, 16) << "...\n";
                cout << "      Time taken: " << (endTime - startTime) << " sec.\n";
                return true;
            }
            
            // Kas 100k bandymų - parodom progress
            if (nonce % 100000 == 0 && nonce > 0) {
                cout << "      Bandymas: " << nonce << "...\n";
            }
        }
        
        cout << "  Mining failed\n";
        return false;
    }
    
    // Atspausdinti bloko info
    void print() {
        cout << "\n  ╔════════════════════════════════════════════════════════════╗\n";
        cout << "  ║  BLOKAS #" << number << "\n";
        cout << "  ╚════════════════════════════════════════════════════════════╝\n";
        cout << "    Hash:     " << hash.substr(0, 20) << "...\n";
        cout << "    Prev:     " << previousHash.substr(0, 20) << "...\n";
        cout << "    Nonce:    " << nonce << "\n";
        cout << "    TX count: " << transactions.size() << "\n";
        cout << "\n    Pirmos 3 transakcijos:\n";
        
        for (int i = 0; i < min(3, (int)transactions.size()); i++) {
            transactions[i].print();
        }
        
        if (transactions.size() > 3) {
            cout << "      ... ir dar " << (transactions.size() - 3) << " transakcijų\n";
        }
    }
};



class User {
public:
    string name;
    string publicKey;
    int balance;
    
    User() {
        name = "";
        publicKey = "";
        balance = 0;
    }
    
    User(string n, string key, int bal) {
        name = n;
        publicKey = key;
        balance = bal;
    }
};

// ============================================================================
// 5. BLOCKCHAIN (Visa sistema)
// ============================================================================

class Blockchain {
public:
    vector<Block> chain;              // Blokų grandinė
    vector<Transaction> mempool;      // Laukiančios transakcijos
    map<string, User> users;          // Visi vartotojai
    
    // Konstruktorius
    Blockchain() {
        cout << "\n  Creating blockchain system...\n";
    }
    
    // ŽINGSNIS 1: Sukurti vartotojus
    void createUsers(int count) {
        cout << "\n  ----------------------------------------\n";
        cout << "  CREATING USERS\n";
        cout << "  ----------------------------------------\n";
        
        vector<string> names = {
            "Alice", "Bob", "Charlie", "David", "Eve",
            "Frank", "Grace", "Henry", "Ivy", "Jack"
        };
        
        for (int i = 0; i < count; i++) {
            // Sugeneruojame public key
            string key = "user_" + std::to_string(i);
            
            // Atsitiktinis vardas
            string name = names[i % names.size()] + std::to_string(i);
            
            // Atsitiktinis balansas 100-1000
            int balance = 100 + (rand() % 900);
            
            users[key] = User(name, key, balance);
        }
        
        cout << "  Created " << users.size() << " users\n\n";
        
        // Parodome pirmus 5
        cout << "  Pavyzdžiai:\n";
        int shown = 0;
        for (auto& pair : users) {
            if (shown++ >= 5) break;
            cout << "    " << pair.second.name 
                 << " - " << pair.second.balance << " EUR\n";
        }
    }
    
    // ŽINGSNIS 2: Sukurti transakcijas
    void createTransactions(int count) {
        cout << "\n  ----------------------------------------\n";
        cout << "  CREATING TRANSACTIONS\n";
        cout << "  ----------------------------------------\n";
        
        // Padarome user keys listą
        vector<string> keys;
        for (auto& pair : users) {
            keys.push_back(pair.first);
        }
        
        for (int i = 0; i < count; i++) {
            // Atsitiktinis siuntėjas ir gavėjas
            string sender = keys[rand() % keys.size()];
            string receiver = keys[rand() % keys.size()];
            
            // Jei tas pats - ieškome kito
            while (receiver == sender) {
                receiver = keys[rand() % keys.size()];
            }
            
            // Atsitiktinė suma
            int amount = 1 + (rand() % 100);
            
            mempool.push_back(Transaction(sender, receiver, amount));
        }
        
        cout << "  Created " << mempool.size() << " transactions\n\n";
        
        // Parodome pirmas 3
        cout << "  Pavyzdžiai:\n";
        for (int i = 0; i < min(3, (int)mempool.size()); i++) {
            mempool[i].print();
        }
    }
    
    // ŽINGSNIS 3: Iškasti naują bloką
    void mineBlock() {
        if (mempool.empty()) {
            cout << "\n  Mempool is empty - no transactions!\n";
            return;
        }
        
        cout << "\n  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        cout << "  ⛏️  NAUJAS BLOKAS\n";
        cout << "  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        
        // Nustatome previous hash
        string prevHash = "0000000000000000000000000000000000000000000000000000000000000000";
        if (!chain.empty()) {
            prevHash = chain.back().hash;
        }
        
        // Kuriame naują bloką
        Block newBlock(chain.size(), prevHash);
        
        // Įdedame iki 10 transakcijų
        int txCount = min(10, (int)mempool.size());
        for (int i = 0; i < txCount; i++) {
            newBlock.addTransaction(mempool[i]);
        }
        
        // Kasome bloką
        if (newBlock.mine()) {
            // Pritaikome transakcijas (atnaujiname balansus)
            applyTransactions(newBlock);
            
            // Pridedame į grandinę
            chain.push_back(newBlock);
            
            // Išmetame iš mempool
            mempool.erase(mempool.begin(), mempool.begin() + txCount);
            
            cout << "\n  ✅ Blokas #" << newBlock.number << " pridėtas į grandinę!\n";
        }
    }
    
    // Pritaikome transakcijas (atnaujiname balansus)
    void applyTransactions(Block& block) {
        cout << "\n  Updating balances...\n";
        
        for (auto& tx : block.transactions) {
            // Jei abu vartotojai egzistuoja
            if (users.count(tx.from) && users.count(tx.to)) {
                // Patikriname ar siuntėjas turi pakankamai
                if (users[tx.from].balance >= tx.amount) {
                    users[tx.from].balance -= tx.amount;
                    users[tx.to].balance += tx.amount;
                }
            }
        }
        
        cout << "  Balances updated\n";
    }
    
    // Parodyti statistiką
    void printStats() {
        cout << "\n  ----------------------------------------\n";
        cout << "  BLOCKCHAIN STATISTICS\n";
        cout << "  ----------------------------------------\n";
        cout << "    Blokų grandinės ilgis: " << chain.size() << "\n";
        cout << "    Laukiančių transakcijų: " << mempool.size() << "\n";
        cout << "    Vartotojų: " << users.size() << "\n";
        
        if (!chain.empty()) {
            cout << "\n    Paskutinis blokas:\n";
            cout << "      Numeris: #" << chain.back().number << "\n";
            cout << "      Hash: " << chain.back().hash.substr(0, 20) << "...\n";
            cout << "      Transakcijų: " << chain.back().transactions.size() << "\n";
        }
    }
    
    // Parodyti visą grandinę
    void printChain() {
        cout << "\n  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        cout << "  🔗 BLOKŲ GRANDINĖ\n";
        cout << "  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        
        for (auto& block : chain) {
            block.print();
        }
    }
};

// ============================================================================
// MAIN - PROGRAMA PRASIDEDA ČIA
// ============================================================================

int main() {
    // Nustatome random seed
    srand(time(nullptr));
    
    cout << "\n";
    cout << "  ========================================================\n";
    cout << "                   BLOCKCHAIN v0.1                           \n";
    cout << "  ========================================================\n";
    
    // Sukuriame blockchain sistemą
    Blockchain blockchain;
    
    // 1. Sukuriame vartotojus (100 vartotojų)
    blockchain.createUsers(100);
    
    // 2. Sukuriame transakcijas (50 transakcijų)
    blockchain.createTransactions(50);
    
    // 3. Kasome 5 blokus
    cout << "\n\n";
    cout << "  ========================================================\n";
    cout << "                   STARTING TO MINE BLOCKS                    \n";
    cout << "  ========================================================\n";
    
    for (int i = 0; i < 5 && blockchain.mempool.size() > 0; i++) {
        blockchain.mineBlock();
    }
    
    // 4. Parodome rezultatus
    cout << "\n\n";
    blockchain.printStats();
    
    // 5. (Neprivaloma) Parodome visą grandinę
    // blockchain.printChain();
    
    cout << "\n\n";
    cout << "  ========================================================\n";
    cout << "                   PROGRAM COMPLETED                         \n";
    cout << "  ========================================================\n\n";
    
    return 0;
}