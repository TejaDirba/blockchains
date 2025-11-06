// blockchain.cpp — v0.1, ASCII-only summary logging

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <ctime>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <chrono>

using namespace std;

// -------- Simple 128-bit string hash (compact, deterministic) --------
string simple_hash(const string &input) {
    unsigned long long h1 = 0x123456789ABCDEFULL;
    unsigned long long h2 = 0xFEDCBA987654321ULL;
    for (char c : input) {
        h1 = (h1 * 131) ^ (unsigned char)c;
        h2 = (h2 * 137) + (unsigned char)c;
        h1 ^= (h2 >> 7);
    }
    stringstream ss;
    ss << hex << setfill('0')
       << setw(16) << h1
       << setw(16) << h2;
    return ss.str();
}

// -------- Types --------
struct Transaction {
    string from;
    string to;
    uint64_t amount = 0;
    bool coinbase = false;
    string id;
};

struct Block {
    string prev_hash;
    string hash;
    uint64_t nonce = 0;
    uint64_t timestamp = 0;
    vector<Transaction> txs;
};

// -------- Globals --------
vector<Block> blockchain;
vector<Transaction> mempool;
unordered_map<string, uint64_t> balances;

string DIFF = "000";                 // Proof-of-Work prefix
uint64_t BLOCK_REWARD = 50;          // coinbase amount
string MINER = "miner_demo";         // receiver of coinbase
string ZERO64 = string(64, '0');     // genesis prev-hash

// -------- Helpers --------
string tx_id(const Transaction &t) {
    return simple_hash(t.from + "|" + t.to + "|" + to_string(t.amount));
}

string block_hash(const Block &b) {
    string s = b.prev_hash + to_string(b.timestamp) + to_string(b.nonce);
    for (auto &tx : b.txs) s += tx.id;
    return simple_hash(s);
}

void generate_users(size_t n) {
    balances.clear();
    mt19937_64 rng(123);
    uniform_int_distribution<uint64_t> dist(100, 1000000);
    for (size_t i = 0; i < n; i++) {
        balances["user" + to_string(i)] = dist(rng);
    }
    balances[MINER] = 0;
}

void generate_txs(size_t n) {
    mempool.clear();
    vector<string> users;
    users.reserve(balances.size());
    for (auto &p : balances) users.push_back(p.first);

    if (users.size() < 3) return;

    mt19937_64 rng(321);
    uniform_int_distribution<size_t> pick(0, users.size() - 1);
    uniform_int_distribution<uint64_t> amount(1, 50000);

    mempool.reserve(n);
    for (size_t i = 0; i < n; i++) {
        string s = users[pick(rng)], r = users[pick(rng)];
        while (r == s) r = users[pick(rng)];
        Transaction t{s, r, amount(rng), false, ""};
        t.id = tx_id(t);
        mempool.push_back(t);
    }
}

// take up to 100 largest by amount
vector<Transaction> pick_top100() {
    vector<Transaction> v = mempool;
    sort(v.begin(), v.end(),
         [](const Transaction& a, const Transaction& b){ return a.amount > b.amount; });
    if (v.size() > 100) v.resize(100);
    return v;
}

// -------- Mining and apply --------
bool mine(Block &b) {
    for (uint64_t n = 0; n < 10000000; n++) {
        b.nonce = n;
        string h = block_hash(b);
        if (h.rfind(DIFF, 0) == 0) { b.hash = h; return true; }
    }
    return false;
}

void apply_block(Block &b) {
    for (auto &t : b.txs) {
        if (t.coinbase) {
            balances[t.to] += t.amount;
        } else if (balances[t.from] >= t.amount) {
            balances[t.from] -= t.amount;
            balances[t.to] += t.amount;
        }
    }
    blockchain.push_back(b);

    // remove included tx from mempool
    for (auto &t : b.txs) {
        auto it = find_if(mempool.begin(), mempool.end(),
                          [&](Transaction &x){ return x.id == t.id; });
        if (it != mempool.end()) mempool.erase(it);
    }
}

// -------- Summary helpers (nice for grading) --------
void print_block_summary(size_t i){
    if (i >= blockchain.size()) { cout << "[query] block " << i << " not found\n"; return; }
    const auto& b = blockchain[i];
    cout << "[block " << i << "] "
         << "time=" << b.timestamp
         << " txs=" << b.txs.size()
         << " prev=" << b.prev_hash.substr(0,16)
         << " hash=" << b.hash.substr(0,16)
         << " nonce=" << b.nonce << "\n";
}

void print_tx_prefix(const string& prefix){
    for (size_t i=0;i<blockchain.size();++i){
        for (const auto& t : blockchain[i].txs){
            if (t.id.rfind(prefix,0)==0){
                cout << "[tx] block="<<i
                     << " id="<<t.id.substr(0,16)<<"..."
                     << " from="<<t.from<<" to="<<t.to
                     << " amt="<<t.amount
                     << (t.coinbase? " (coinbase)":"")
                     << "\n";
                return;
            }
        }
    }
    cout << "[query] tx " << prefix << "... not found\n";
}

// -------- One block routine with richer logs --------
void mine_one_block() {
    if (mempool.empty()) { cout << "Mempool empty.\n"; return; }

    vector<Transaction> txs = pick_top100();
    uint64_t ts = time(nullptr);

    // coinbase first
    Transaction coinbase{"reward", MINER, BLOCK_REWARD, true, ""};
    coinbase.id = tx_id(coinbase);
    txs.insert(txs.begin(), coinbase);

    // block skeleton
    Block b;
    b.prev_hash = blockchain.empty() ? ZERO64 : blockchain.back().hash;
    b.timestamp = ts;
    b.txs = txs;

    size_t block_index = blockchain.size();
    cout << "Mining block #" << block_index << "...\n";
    cout << "  Transactions (incl. coinbase): " << b.txs.size() << "\n";
    cout << "  Difficulty: " << DIFF << "\n";
    cout << "  Prev: " << b.prev_hash.substr(0,16) << "...\n";

    auto start = chrono::steady_clock::now();
    if (mine(b)) {
        auto end = chrono::steady_clock::now();
        double secs = chrono::duration<double>(end - start).count();

        cout << "  Block mined!\n";
        cout << "  Nonce: " << b.nonce << "\n";
        cout << "  Hash: " << b.hash.substr(0,16) << "...\n";
        cout << "  Time: " << secs << " sec\n";

        apply_block(b);

        cout << "  Coinbase -> " << MINER << " +" << BLOCK_REWARD << "\n";
        cout << "  Txs in block: " << b.txs.size() << " (incl. coinbase)\n";
        cout << "  Mempool left: " << mempool.size() << "\n";
        cout << "  Added block #" << (blockchain.size()-1) << "\n\n";
    } else {
        cout << "  Mining failed (reduce difficulty or increase iterations).\n";
    }
}

// -------- main --------
int main() {
    // dataset
    generate_users(1000);
    generate_txs(10000);

    cout << "Users: " << balances.size()
         << " | Mempool: " << mempool.size() << "\n";

    // mine up to 50 blocks or until mempool empties
    for (int i=0; i<50 && !mempool.empty(); i++){
        mine_one_block();
        if ((i+1)%5==0){
            cout << "[info] created blocks: " << (i+1)
                 << " | mempool left: " << mempool.size() << "\n";
            print_block_summary(blockchain.size()-1);
        }
    }

    // final summaries
    if (!blockchain.empty()){
        print_block_summary(0);
        print_block_summary(blockchain.size()-1);
    }

    cout << "\nBlockchain length: " << blockchain.size() << "\n";
    cout << "Done.\n";
    return 0;
}
