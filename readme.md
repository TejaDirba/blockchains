# Supaprastinta Blokų Grandinė (Blockchain) — v0.2
<img width="937" height="245" alt="image" src="https://github.com/user-attachments/assets/8f341ec9-6ce9-475f-a2ab-55b50b302f5c" />
<img width="937" height="245" alt="Screenshot 2025-11-06 144323" src="https://github.com/user-attachments/assets/e1b466d8-1ace-49a0-ace7-e475bd7a9332" />

Ši programa įgyvendina **supaprastintą blokų grandinės modelį** (pagal užduotį *„Supaprastintos blokų grandinės kūrimas“*).

Šis projektas yra supaprastinta „Blockchain“ grandinės simuliacija, sukurta edukaciniams tikslams.
Programa modeliuoja vartotojų sistemą, atsitiktines pinigines transakcijas, blokų kasimą (angl. mining) ir blokų grandinės (blockchain) formavimą.

Versija v0.2 – tai išplėsta v0.1 versija su papildomais patobulinimais:

pridėtas Merkle medžio šakninio hash’o skaičiavimas;

įtrauktas coinbase (minerio atlygis) mechanizmas;

įdiegta transakcijų validacija (tinkamas siuntėjas, balansas, suma);

realizuotas penkių kandidatų blokų kasybos modelis;

įdiegta kasimo laiko apribojimo sistema (maks. 5 s per bandymą, 30 s visam veikimui);

privaloma, kad kiekviename bloke būtų bent viena reali transakcija;

blokai kasami pagal sunkumo lygį (difficulty prefix) – čia „00“;

uždraustos tuščios transakcijos ar neįmanomi pervedimai;

pridėtas mempool’o (neapdorotų transakcijų) valymas.

 Naudojamos technologijos

Programavimo kalba: C++20

💡 Pagrindinės sąvokos
Sąvoka	Paaiškinimas
Transaction (transakcija)	Pervedimas tarp vartotojų (siuntėjas, gavėjas, suma, ID).
Block (blokas)	Transakcijų rinkinys su „header“ informacija ir savo hash’u.
Coinbase transakcija	Sistema automatiškai sukuria transakciją, kur mineris gauna atlygį (50 vienetų).
Merkle Root	Hash’ų medis, kuris apibendrina visų transakcijų hash’us į vieną.
Mining (kasimas)	Atsitiktinio „nonce“ paieška, kol bloko hash’as atitinka nustatytą sunkumo lygį ("00...").
Nonce	Skaičius, kuris keičiamas, kol gaunamas reikiamas hash’as.
Difficulty prefix	Pradiniai hash’o simboliai, pagal kuriuos sprendžiama kasimo sunkumas. Pvz. „00“ – lengvas, „00000“ – labai sunkus.
Genesis block	Pirmasis blokas grandinėje, neturintis ankstesnio hash’o.

🧩 Duomenų struktūra
User: {
  name,          // Pvz. "User_0012"
  public_key,    // Sugeneruota pseudo viešo rakto forma
  balance         // Atsitiktinis pradinis balansas
}

Transaction: {
  sender, receiver, amount, transaction_id
}

BlockHeader: {
  prev_block_hash, timestamp, version, transactions_hash, difficulty, nonce
}

Block: {
  header, transactions[]
}

Blockchain: {
  blocks[], users[], pending_transactions[]
}

🔧 Programos veikimo principas

Sukuriami vartotojai (1000 vnt., atsitiktiniai balansai).

Sugeneruojamos transakcijos (10 000 atsitiktinių pervedimų).

Kuriamas „genesis“ blokas su tušču hash’u.

Kiekvienas naujas blokas kasamas:

Paimamos iki 100 patvirtintų transakcijų.

Pridedamas coinbase atlygį turintis įrašas.

Skaičiuojamas Merkle root hash’as.

Ieškomas nonce, kad hash’as prasidėtų "00".

Jei hash’as rastas – blokas įtraukiamas į grandinę.

Procesas tęsiasi, kol baigiasi transakcijos arba laikas.

📊 Parametrai ir jų reikšmės
Parametras	Aprašymas	Numatytas
USERS_COUNT	Vartotojų kiekis	1000
TX_COUNT	Transakcijų kiekis	10 000
TXS_PER_BLOCK	Transakcijų kiekis bloke	100
BLOCK_REWARD	Minerio atlygis	50
DIFFICULTY_PREFIX	Kasimo sunkumas	"00"
MAX_BLOCKS_TO_MINE	Maks. blokų skaičius (nullopt = neribota)	neribota
MAX_RUN_SECONDS	Maks. visos programos veikimo laikas	30 s
🧠 Ką ši versija demonstruoja

Deterministinį hash’ingą: ta pati įvestis → tas pats rezultatas.

Konsensusą: tik hash’ai, atitinkantys taisykles, laikomi galiojančiais.

Ekonominę logiką: balansai mažinami ir didinami pagal transakcijas.

Atsitiktinumą ir laiko ribojimą: kasimas trunka tik ribotą laiką.

Tinklo švarinimą: klaidingos ar neįmanomos transakcijos pašalinamos.


🧩 Išvesties pavyzdys
Supaprastintas Blockchain v0.2

[GENESIS] created. hash=00000000000000000000000000000000...

[USERS] generated=1000

[TX] pending=10000

[TARGET] prefix '00'
------------------------------------------------------------

[FOUND] hash=00c4a7e1c3aaf871... nonce=37 txs=101 attempts=180 time=0.01s
[CHAIN] added block#1 mempool_left=9900

[FOUND] hash=00f21d0bc9a61b02... nonce=52 txs=101 attempts=244 time=0.01s
[CHAIN] added block#2 mempool_left=9800

[DONE] Blockchain(height=52, users=1000, pending=0)
Last block hash: 00a1b77e9b532c1d...

⚠️ Pastabos

Ši versija nėra tikras blockchain – tai mokomoji imitacija.

Kasybos greitis priklauso nuo DIFFICULTY_PREFIX – daugiau nulių = sunkiau.

Balansai, vartotojai ir transakcijos generuojami atsitiktinai, todėl kiekvienas paleidimas duos skirtingą rezultatą.

## Kompiliavimas ir paleidimas

### Linux / macOS
```bash
g++ -O2 -std=c++20 blockchain.cpp -o blockchain
./blockchain

### Windows
g++ -O2 -std=c++20 blockchain.cpp -o blockchain.exe
blockchain.exe

