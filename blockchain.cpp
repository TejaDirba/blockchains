#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <ctime>
#include <cstdint>

using std::string;
using std::vector;

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

    static uint64_t rotL(uint64_t v, int s){ return (v<<s) | (v>>(64-s)); }
    static uint64_t rotR(uint64_t v, int s){ return (v>>s) | (v<<(64-s)); }
    static uint64_t mix(uint64_t a, uint64_t b, uint64_t c){
        a ^= b; a = rotL(a,23); a *= PRIME1; a ^= c; return rotL(a,41);
    }
public:
    static string hash(const string& input){
        uint64_t h1=INIT_A, h2=INIT_B, h3=INIT_C, h4=INIT_D;
        const size_t len = input.size();
        for(size_t i=0;i<len;i++){
            uint64_t b = static_cast<uint64_t>(static_cast<unsigned char>(input[i]));
            b ^= (i+1) * PRIME1;
            h1 ^= b * PRIME1; h1 = rotL(h1,13); h1 *= PRIME2;
            h2 ^= b * PRIME2; h2 = rotR(h2,17); h2 += h1;
            h3 ^= b * PRIME3; h3 = rotL(h3,31); h3 ^= h2;
            h4 ^= b * PRIME4; h4 = rotR(h4,19); h4 += h3;
            if((i & 3)==3){ h1=mix(h1,h2,h3); h2=mix(h2,h3,h4); h3=mix(h3,h4,h1); h4=mix(h4,h1,h2); }
        }
        h1 ^= len * PRIME1; h2 ^= len * PRIME2; h3 ^= len * PRIME3; h4 ^= len * PRIME4;
        h1=mix(h1,h2,h3); h2=mix(h2,h3,h4); h3=mix(h3,h4,h1); h4=mix(h4,h1,h2);
        h1 += h2 + h3 + h4; h2 += h1; h3 += h1; h4 += h1;

        std::ostringstream ss; ss<<std::hex<<std::setfill('0');
        ss<<std::setw(16)<<h1<<std::setw(16)<<h2<<std::setw(16)<<h3<<std::setw(16)<<h4;
        return ss.str();
    }
};
static inline string H(const string& s){ return CustomHash::hash(s); }

// ============ Types ============
struct User {
    string name;
    string pk;
    uint64_t balance = 0;
};

struct Tx {
    string from;
    string to;
    uint64_t amount = 0;
    bool coinbase = false;
    string id; // txid = H(from|to|amount) or special for coinbase
};

struct BlockHeader {
    string prev_hash;    // Genesis: 64 zeros
    uint64_t ts = 0;     // unix time
    uint32_t version = 1;
    string txs_root;     // v0.1: hash of concatenated txids (naive)
    uint64_t nonce = 0;  // PoW counter
    string difficulty;   // e.g., "000"
};

struct Block {
    BlockHeader head;
    vector<Tx> txs;
};

// ============ Globals ============
static vector<Block> CHAIN;                       // blocks
static vector<Tx> MEMPOOL;                        // waiting tx
static std::unordered_map<string, User> USERS;    // pk -> user

static string DIFF = "000";        // PoW difficulty (prefix zeros)
static uint64_t BLOCK_REWARD = 50; // coinbase amount
static string MINER_PK = "pk_miner_demo";

static const string ZERO64 =
"0000000000000000000000000000000000000000000000000000000000000000";

// ============ Helpers ============
static string rnd_name(std::mt19937_64& rng){
    static const vector<string> n={"Aiste","Mantas","Ieva","Lukas","Ugne","Tomas","Goda","Kasparas","Emilija","Jonas"};
    return n[rng()%n.size()]+"_"+std::to_string(rng()%100000);
}
static string rnd_pk(std::mt19937_64& rng){
    string s="pk_";
    for(int i=0;i<24;i++){ int v=rng()%16; s.push_back("0123456789abcdef"[v]); }
    return s;
}

static string tx_compute_id(const string& s,const string& r,uint64_t a){
    return H(s + "|" + r + "|" + std::to_string(a));
}

static Tx make_coinbase(const string& miner_pk, uint64_t reward, uint64_t height, uint64_t ts){
    Tx t;
    t.coinbase = true;
    t.from     = "BLOCK_REWARD"; // special issuer
    t.to       = miner_pk;
    t.amount   = reward;
    t.id       = H(string("COINBASE|")+miner_pk+"|"+std::to_string(reward)+"|"+std::to_string(height)+"|"+std::to_string(ts));
    return t;
}

// v0.1 naive "root": hash of concatenated txids
static string naive_root(const vector<Tx>& txs){
    string cat; cat.reserve(txs.size()*64);
    for(const auto& t: txs) cat += t.id;
    return H(cat);
}

// Block hash = H(header fields + nonce)
static string block_hash(const Block& b){
    const auto& h = b.head;
    return H(h.prev_hash+"|"+std::to_string(h.ts)+"|"+std::to_string(h.version)+"|"+
             h.txs_root+"|"+h.difficulty+"|"+std::to_string(h.nonce));
}

// ============ Data generation ============
static void make_users(size_t n){
    USERS.clear();
    USERS.emplace("BLOCK_REWARD", User{"Block_Reward","BLOCK_REWARD",0}); // special
    std::mt19937_64 rng(123456789);
    std::uniform_int_distribution<uint64_t> bal(100, 1'000'000);
    for(size_t i=0;i<n;i++){
        string pk = rnd_pk(rng);
        USERS.emplace(pk, User{rnd_name(rng), pk, bal(rng)});
    }
    if(!USERS.count(MINER_PK)) USERS.emplace(MINER_PK, User{"Miner", MINER_PK, 0});
}

static void make_txs(size_t n){
    MEMPOOL.clear();
    if(USERS.size()<3) return;
    vector<string> keys; keys.reserve(USERS.size());
    for(auto& kv: USERS) if(kv.first!="BLOCK_REWARD") keys.push_back(kv.first);

    std::mt19937_64 rng(987654321);
    std::uniform_int_distribution<size_t> idx(0, keys.size()-1);
    std::uniform_int_distribution<uint64_t> amt(1, 50'000);

    MEMPOOL.reserve(n);
    for(size_t i=0;i<n;i++){
        string s=keys[idx(rng)], r=keys[idx(rng)];
        while(r==s) r=keys[idx(rng)];
        uint64_t a=amt(rng);
        Tx t;
        t.from=s; t.to=r; t.amount=a; t.coinbase=false;
        t.id = tx_compute_id(s,r,a);
        MEMPOOL.push_back(t);
    }
}

// take up to 100 largest by amount
static vector<Tx> pick_top100(){
    vector<Tx> v = MEMPOOL;
    std::sort(v.begin(), v.end(), [](const Tx& a,const Tx& b){ return a.amount > b.amount; });
    if(v.size()>100) v.resize(100);
    return v;
}

// ============ Mining & apply ============
static bool mine(Block& b, uint64_t max_iters = 50'000'000){
    for(uint64_t i=0;i<max_iters;i++){
        b.head.nonce = i;
        if(block_hash(b).rfind(DIFF, 0)==0) return true; // starts with DIFF
    }
    return false;
}

static void apply_block(const Block& b){
    for(const auto& t: b.txs){
        if(t.coinbase){
            USERS[t.to].balance += t.amount;
        }else{
            auto& s = USERS[t.from];
            auto& r = USERS[t.to];
            if(s.balance >= t.amount) s.balance -= t.amount;
            else s.balance = 0; // v0.1 simplification
            r.balance += t.amount;
        }
    }
    // remove confirmed from mempool
    for(const auto& t: b.txs){
        auto it = std::find_if(MEMPOOL.begin(), MEMPOOL.end(),
                               [&](const Tx& x){ return x.id==t.id; });
        if(it!=MEMPOOL.end()) MEMPOOL.erase(it);
    }
    CHAIN.push_back(b);
}

static void print_block_short(size_t i){
    if(i >= CHAIN.size()){ std::cout<<"[block] "<<i<<" nerastas\n"; return; }
    const Block& b = CHAIN[i];
    string bh = block_hash(b);
    std::cout << "[block "<<i<<"] ts="<< b.head.ts
              << " txs="<< b.txs.size()
              << " prev="<< b.head.prev_hash.substr(0,8)
              << " hash="<< bh.substr(0,16)
              << " nonce="<< b.head.nonce << "\n";
}

// ============ One block routine ============
static void make_and_mine_one_block(){
    // 1) pick top 100 tx
    vector<Tx> txs = pick_top100();
    if(txs.empty()){
        std::cout<<"[info] mempool tuscias\n";
        return;
    }
    // 2) coinbase first
    uint64_t ts = std::time(nullptr);
    Tx cb = make_coinbase(MINER_PK, BLOCK_REWARD, (uint64_t)CHAIN.size(), ts);
    txs.insert(txs.begin(), cb);

    // 3) build block
    Block b;
    b.txs = txs;
    b.head.prev_hash = CHAIN.empty()? ZERO64 : block_hash(CHAIN.back());
    b.head.ts = ts;
    b.head.version = 1;
    b.head.txs_root = naive_root(b.txs); // v0.1 root (not real Merkle)
    b.head.nonce = 0;
    b.head.difficulty = DIFF;

    std::cout << "[kasimas] txs="<<b.txs.size()<<" diff="<<DIFF<<"\n";
    bool ok = mine(b);
    if(!ok){
        std::cout<<"[kasimas] nepavyko. Silpnink DIFF arba didink iteracijas.\n";
        return;
    }
    string bh = block_hash(b);
    std::cout<<"[kasimas] iskasta. nonce="<<b.head.nonce<<" hash="<<bh.substr(0,16)<<"...\n";
    for(size_t i=0;i<std::min<size_t>(5, b.txs.size()); i++){
        const Tx& t = b.txs[i];
        if(i==0 && t.coinbase) std::cout<<"  tx[0] COINBASE -> "<<t.to<<" "<<t.amount<<"\n";
        else std::cout<<"  tx["<<i<<"] "<<t.id.substr(0,12)<<" "<<t.amount<<"\n";
    }
    // 4) apply
    apply_block(b);
}

// ============ main ============
int main(){
    // data
    make_users(1000);
    make_txs(10'000);

    std::cout<<"users="<<USERS.size()<<" mempool="<<MEMPOOL.size()<<"\n";

    // mine up to 50 blocks or until mempool is empty
    for(int i=0; i<50 && !MEMPOOL.empty(); i++){
        make_and_mine_one_block();
        if((i+1)%5==0){
            std::cout<<"[info] sukurta bloku: "<<(i+1)<<" mempool liko: "<<MEMPOOL.size()<<"\n";
            print_block_short(CHAIN.size()-1);
        }
    }

    std::cout<<"\n[rezultatas] grandines ilgis: "<<CHAIN.size()<<"\n";
    std::cout<<"Baigta.\n";
    return 0;
}
