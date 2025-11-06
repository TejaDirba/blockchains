#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <random>
#include <algorithm>
#include <chrono>
#include <numeric>

using namespace std;

// ---- Config ----
static const string VERSION_ = "v0.1";
static const int USERS_COUNT = 1000;
static const int64_t TX_COUNT = 10000;
static const int TXS_PER_BLOCK = 100;              // ~100 tx per block
static const string DIFFICULTY_PREFIX = "000";     // PoW target
static const uint64_t RNG_SEED = 42;
static const string MINER_PK = "pk_miner_demo";    // coinbase receiver
static const int64_t BLOCK_REWARD = 50;            // coinbase reward
static optional<int> MAX_BLOCKS_TO_MINE = nullopt; // set to value to limit mined blocks

// ---- Utils ----
static bool starts_with(const string& s, const string& pref) {
    return s.size() >= pref.size() && equal(pref.begin(), pref.end(), s.begin());
}

static string to_hex16(uint64_t x) {
    stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16) << std::nouppercase << x;
    return ss.str();
}

// Simple deterministic 256-bit string hash (toy, not cryptographic)
string custom_hash(const string& input) {
    uint64_t part0 = 0x1234567890abcdefULL;
    uint64_t part1 = 0xfedcba0987654321ULL;
    uint64_t part2 = 0x0f1e2d3c4b5a6978ULL;
    uint64_t part3 = 0x89abcdef01234567ULL;

    for (unsigned char b : input) {
        part0 = part0 + b;
        part1 = part1 + part0 * 3;
        part2 = part2 + part1 + (b * 7);
        part3 = part3 + part2 + (part0 * 2);
        // light mixing
        part0 ^= (part1 >> 11);
        part1 ^= (part2 << 7);
        part2 ^= (part3 >> 5);
        part3 ^= (part0 << 3);
    }

    // a few rounds of extra mixing
    for (int i = 0; i < 8; i++) {
        part0 = (part0 ^ (part1 * 3)) * 0x4e97b2d8f3c1a5e3ULL;
        part1 = (part1 ^ (part2 * 5)) * 0xa3d5f79c482eb16fULL;
        part2 = (part2 ^ (part3 * 7)) * 0x5b18e4c7d92fa06eULL;
        part3 = (part3 ^ (part0 * 9)) * 0xc74a9e21f05bd83cULL;
    }

    stringstream ss;
    ss << hex << setfill('0')
       << setw(16) << part0
       << setw(16) << part1
       << setw(16) << part2
       << setw(16) << part3;
    return ss.str();
}

// ---- Data structures ----
struct User {
    string name;
    string public_key;
    int64_t balance;
};

struct Transaction {
    string sender;
    string receiver;
    int64_t amount;
    string transaction_id;

    Transaction() = default;
    Transaction(const string& s, const string& r, int64_t a)
        : sender(s), receiver(r), amount(a) {
        string base = sender + "|" + receiver + "|" + to_string(amount);
        transaction_id = custom_hash(base);
    }
};

struct BlockHeader {
    string prev_block_hash;   // genesis: 64 zeros
    double timestamp;         // seconds since epoch
    string version;           // "v0.1"
    string transactions_hash; // hash of concatenated txids (naive root)
    string difficulty;        // e.g., "000"
    uint64_t nonce = 0;

    string to_string() const {
        ostringstream oss;
        oss << prev_block_hash << '|' << fixed << setprecision(6) << timestamp
            << '|' << version << '|' << transactions_hash
            << '|' << difficulty << '|' << nonce;
        return oss.str();
    }
};

struct Block {
    BlockHeader header;
    vector<Transaction> transactions;

    explicit Block(BlockHeader h, vector<Transaction> txs)
        : header(std::move(h)), transactions(std::move(txs)) {}

    string compute_hash() const { return custom_hash(header.to_string()); }
};

class Blockchain {
public:
    Blockchain() : rng(RNG_SEED) { create_genesis_block(); }

    void generate_users(int n) {
        users.reserve(n + 1);
        for (int i = 0; i < n; ++i) {
            string name = "User_" + fmt_index(i, 4);
            string pk = "pk_" + custom_hash(name).substr(0, 16);
            int64_t balance = rand_int(100, 1'000'000);
            users.emplace(pk, User{name, pk, balance});
            user_keys.push_back(pk);
        }
        // ensure miner exists
        if (!users.count(MINER_PK)) {
            users.emplace(MINER_PK, User{"Miner", MINER_PK, 0});
            user_keys.push_back(MINER_PK);
        }
        cout << "[USERS] generated=" << users.size() << "\n\n";
    }

    void generate_transactions(int64_t n) {
        pending.reserve(pending.size() + n);
        for (int64_t i = 0; i < n; ++i) {
            string s = random_key();
            string r = random_key();
            while (r == s) r = random_key();
            // allow full range; policy will validate later if needed
            int64_t amount = rand_int(1, 50'000);
            pending.emplace_back(s, r, amount);
        }
        cout << "[TX] pending=" << pending.size() << "\n\n";
    }

    // mine next block: coinbase + top-by-amount policy
    optional<Block> mine_next_block(int txs_per_block = TXS_PER_BLOCK) {
        if (pending.empty()) { cout << "[MINE] no pending tx\n"; return nullopt; }

        // --- policy: top by amount ---
        vector<int> idx(pending.size());
        iota(idx.begin(), idx.end(), 0);
        sort(idx.begin(), idx.end(), [&](int a, int b){
            return pending[a].amount > pending[b].amount;
        });
        int k = min<int>(txs_per_block, (int)pending.size());
        vector<Transaction> txs; txs.reserve(k + 1);
        for (int i = 0; i < k; ++i) txs.push_back(pending[idx[i]]);

        // coinbase first
        Transaction coinbase("BLOCK_REWARD", MINER_PK, BLOCK_REWARD);
        txs.insert(txs.begin(), coinbase);

        // naive "root": hash of concatenated txids
        string add_ids; add_ids.reserve(txs.size() * 65);
        for (auto& tx : txs) { add_ids += tx.transaction_id; add_ids += '|'; }
        string txs_hash = custom_hash(add_ids);

        string prev_hash = last_block_hash();
        BlockHeader header{
            prev_hash,
            current_time_seconds(),
            VERSION_,
            txs_hash,
            DIFFICULTY_PREFIX,
            0
        };
        Block block(header, txs);

        cout << "[MINE] block#" << blocks.size()
             << " txs=" << block.transactions.size()
             << " diff='" << DIFFICULTY_PREFIX << "' prev=" << prev_hash.substr(0,16) << "...\n";

        auto start = chrono::steady_clock::now();
        uint64_t attempts = 0;
        string h;

        while (true) {
            h = block.compute_hash();
            ++attempts;
            if (starts_with(h, DIFFICULTY_PREFIX)) {
                auto took = chrono::duration<double>(chrono::steady_clock::now() - start).count();
                cout << "       mined hash=" << h.substr(0,16) << "... attempts=" << attempts
                     << " time=" << fixed << setprecision(3) << took << "s\n";
                break;
            }
            ++block.header.nonce;
        }

        // chain tip check
        if (block.header.prev_block_hash != last_block_hash()) {
            cerr << "[ERROR] chain tip changed; aborting block\n";
            return nullopt;
        }

        // apply effects
        apply_transactions(block.transactions);
        erase_used_transactions(block.transactions);

        blocks.push_back(std::move(block));
        cout << "[CHAIN] added block#" << (blocks.size()-1)
             << " mempool_left=" << pending.size()
             << " coinbase->" << MINER_PK << " +" << BLOCK_REWARD << "\n\n";
        return blocks.back();
    }

    string last_block_hash() const { return blocks.back().compute_hash(); }

    string info() const {
        ostringstream oss;
        oss << "Blockchain(height=" << (blocks.size()-1)
            << ", users=" << users.size()
            << ", pending=" << pending.size() << ")";
        return oss.str();
    }

    void run_all() {
        cout << "[INFO] " << info() << "\n\n";
        cout << "[TARGET] prefix '" << DIFFICULTY_PREFIX << "'\n\n";
        int mined = 0;
        while (!pending.empty()) {
            if (MAX_BLOCKS_TO_MINE && mined >= *MAX_BLOCKS_TO_MINE) {
                cout << "[STOP] demo cap reached: " << *MAX_BLOCKS_TO_MINE << " blocks\n";
                break;
            }
            auto b = mine_next_block(TXS_PER_BLOCK);
            if (!b) break;
            ++mined;
        }
        cout << "[DONE] " << info() << "\n";
        cout << "Last block hash: " << last_block_hash() << "\n";
    }

private:
    vector<Block> blocks;
    unordered_map<string, User> users;
    vector<string> user_keys;
    vector<Transaction> pending;
    mt19937_64 rng;

    void create_genesis_block() {
        BlockHeader header{
            string(64, '0'),   // prev = 64 zeros
            current_time_seconds(),
            VERSION_,
            string(64, '0'),   // tx root placeholder for genesis
            DIFFICULTY_PREFIX,
            0
        };
        Block genesis(header, {});
        blocks.push_back(genesis);
        cout << "[GENESIS] created. hash=" << genesis.compute_hash() << "\n\n";
    }

    static string fmt_index(int x, int width) {
        ostringstream oss; oss << setw(width) << setfill('0') << x; return oss.str();
    }

    double current_time_seconds() const {
        using namespace std::chrono;
        auto now = system_clock::now().time_since_epoch();
        return duration<double>(now).count();
    }

    int64_t rand_int(int64_t a, int64_t b) {
        uniform_int_distribution<int64_t> dist(a, b);
        return dist(rng);
    }

    string random_key() {
        uniform_int_distribution<size_t> dist(0, user_keys.size() - 1);
        return user_keys[dist(rng)];
    }

    void apply_transactions(const vector<Transaction>& txs) {
        for (const auto& tx : txs) {
            if (tx.sender == "BLOCK_REWARD") {
                users[tx.receiver].balance += tx.amount; // coinbase
                continue;
            }
            auto& s = users[tx.sender];
            auto& r = users[tx.receiver];
            if (s.balance >= tx.amount) {
                s.balance -= tx.amount;
                r.balance += tx.amount;
            }
        }
    }

    void erase_used_transactions(const vector<Transaction>& used) {
        unordered_set<string> used_ids;
        used_ids.reserve(used.size()*2);
        for (auto& t : used) {
            if (t.sender != "BLOCK_REWARD") used_ids.insert(t.transaction_id);
        }
        vector<Transaction> keep; keep.reserve(pending.size());
        for (auto& t : pending) if (!used_ids.count(t.transaction_id)) keep.push_back(std::move(t));
        pending.swap(keep);
    }
};

int main() {
    cout << "Supaprastintas Blockchain " << VERSION_ << "\n\n";
    Blockchain bc;
    bc.generate_users(USERS_COUNT);
    bc.generate_transactions(TX_COUNT);
    bc.run_all();
    return 0;
}
