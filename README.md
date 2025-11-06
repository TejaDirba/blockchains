# Supaprastinta Blokų Grandinė (Blockchain) — v0.1

Ši programa įgyvendina **supaprastintą blokų grandinės modelį** (pagal užduotį *„Supaprastintos blokų grandinės kūrimas“*).

Sistemoje realizuojamas:
- **Genesis blokas**, kurio `Previous Block Hash` yra 64 nuliai (`0000...0000`).
- **Transakcijos** tarp vartotojų.
- **Blokų kasimas (Proof-of-Work)**, kol bloko hash pradžioje yra trys nuliai (`"000"`).
- **Coinbase transakcija**, kuri suteikia kasėjui (`miner_demo`) 50 vienetų už kiekvieną iškastą bloką.
- **Transakcijų atranka** pagal siunčiamą sumą (imamos didžiausios).
- **Konsolės log‘ai** su santrauka: bloko numeris, hash, nonce, transakcijų kiekis, likusios transakcijos mempoole.
- **Automatinis balanso atnaujinimas** siuntėjui ir gavėjui.

---

## Pagrindinės savybės

| Funkcija | Aprašymas |
|-----------|------------|
| **Genesis Block** | Pirmasis blokas su `prev_hash = 64 nuliai`. |
| **Users** | 1000 atsitiktinių vartotojų su pradiniais balansais. |
| **Transactions** | 10 000 atsitiktinių transakcijų mempoole. |
| **Blocks** | Kiekviename bloke ~100 transakcijų + 1 coinbase. |
| **Coinbase** | Nauji „pinigai“ kasėjui — atlieka „centrinio banko“ vaidmenį. |
| **Proof-of-Work** | Hash turi prasidėti `"000"`. |
| **Logging** | Aiškūs žingsniai kasant, parodytas hash, nonce, mempool likutis. |

---

## Programos struktūra

### `main.cpp` (arba `blockchain.cpp`)
- Pagrindinis failas, kuriame realizuota:
  - vartotojų generacija,
  - transakcijų kūrimas,
  - blokų formavimas ir kasimas,
  - balansų atnaujinimas.

### Pagrindinės struktūros:
- **User** — vardas, raktas (public_key), balansas.  
- **Transaction** — siuntėjas, gavėjas, suma, `transaction_id`.  
- **BlockHeader** — `prev_hash`, `timestamp`, `difficulty`, `nonce`.  
- **Block** — sąrašas transakcijų ir antraštė.  

---

## Logikos eiga

1. Sukuriama 1000 vartotojų (`generate_users`).
2. Sugeneruojama 10 000 transakcijų (`generate_transactions`).
3. Sukuriamas **genesis blokas** su `prev_hash = 64 nuliai`.
4. Kasimo metu:
   - pasirenkamos 100 didžiausių transakcijų,
   - pridedama **coinbase** transakcija (kasėjui `miner_demo`),
   - formuojamas `BlockHeader`,
   - ieškomas hash, kurio pradžia `"000"`.
5. Jei pavyksta, blokas pridedamas prie grandinės, atnaujinami balansai, pašalinamos panaudotos transakcijos.
6. Procesas kartojamas iki kol baigsis transakcijos mempoole.

---

## Kompiliavimas ir paleidimas
### Windows
g++ -O2 -std=c++20 blockchain.cpp -o blockchain.exe
blockchain.exe
<img width="1074" height="366" alt="Screenshot 2025-11-06 114805" src="https://github.com/user-attachments/assets/74267acb-16a7-4738-abd7-d4800eaee73a" />
<img width="1077" height="323" alt="Screenshot 2025-11-06 114820" src="https://github.com/user-attachments/assets/e0d6e7dd-c102-47a2-9f7e-8de51aaceb2f" />

### Linux / macOS
```bash
g++ -O2 -std=c++20 blockchain.cpp -o blockchain
./blockchain


