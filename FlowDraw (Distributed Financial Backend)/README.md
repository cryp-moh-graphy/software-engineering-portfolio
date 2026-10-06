# FlowDraw

A fully on-chain lottery smart contract featuring a dynamic hybrid draw trigger, oracle-free randomness, stale-draw recovery, and a pull-payment prize system.

## Key Features

- **Hybrid Draw Trigger:** Draws initiate dynamically based on either a configurable time interval or a maximum ticket capacity threshold—preventing stagnation on slow weeks and eliminating wait times on hot weeks.
- **Oracle-Free Randomness:** Uses the hash of a future block (seeded with the contract address) as the entropy source, so there are no external oracle fees (e.g., Chainlink). Ticket sales are locked once a draw starts, so the entropy block is unknown to every player at purchase time.
- **Stale-Draw Recovery:** `blockhash()` returns zero for blocks older than 256, which would otherwise leave a draw permanently stuck. `restartDraw()` lets anyone re-arm the draw with a fresh target block once the original hash has expired.
- **Prize Pool Rollover:** If nobody matches the winning numbers, the pool carries into the next round instead of being locked in the contract.
- **Pull-Payment Architecture:** Prizes are distributed via a pull pattern (`claimPrize`), so a single winner's failed transfer cannot revert the entire draw.
- **O(1) Participant Lookup:** Participant status is a direct mapping read (`hasJoined`) rather than a linear scan.
- **State Safeguards:** Checks-Effects-Interactions ordering in `buyTicket` and `claimPrize`, a draw that cannot be finalized twice, and strict minimum participant/ticket requirements.

## How It Works

### 1. The Setup
- **Ticket Cost:** 0.015 ETH
- **Distribution:** 0.003 ETH (20%) per ticket goes to the Host; the remainder, plus any tip above the ticket price, goes to the Prize Pool.
- **Parameters:** The host can adjust the draw interval (1-30 days), participant minimums (2-100), and ticket caps, bounded by safety checks.

### 2. The Hybrid Trigger
The contract does not force users to wait for a rigid timer if demand is high, nor does it force a draw if participation is too low.
```solidity
// Triggers if the time interval has passed OR the ticket cap is reached
bool timeConditionMet = (lastDrawTime == 0) || (block.timestamp >= lastDrawTime + drawInterval);
bool ticketCapReached = totalTickets >= maxTicketsPerDraw;
```

### 3. Draw Lifecycle
1. `startLottery()` — validates thresholds, locks ticket sales, and picks a target block 5 blocks ahead.
2. `findWinner()` — once the target block has passed, derives the winning numbers from its hash and credits winners.
3. `restartDraw()` — only needed if nobody finalized within 256 blocks; re-arms the draw with a new target block.
4. `claimPrize()` — winners withdraw their balance.

## Testing

Tests run against a local chain so blocks can be mined on demand (Remix cannot advance blocks).

```bash
npm install
npm test
```

They cover: stale-blockhash deadlock and recovery, the exact-target-block edge case, prize pool rollover when there is no winner, and state ordering during the host fee call.

## Known Limitations

- **Blockhash randomness:** A block proposer can influence a blockhash, so this is not suitable for high-value pools. It trades manipulation resistance for zero oracle cost.
- **Finalization gas:** `findWinner()` iterates over all players and tickets, so gas grows with volume near the 10,000-ticket cap. Only the participant lookup is O(1).
- **Jackpot odds:** A winning ticket must match 5 ordered numbers from 1-99 (about 1 in 9.5 billion), so most rounds roll over.
- **Winner payout path:** Covered by design review but not by automated tests, since winning numbers are random.

<sub>[⬅ Back to Main Page](https://github.com/cryp-moh-graphy/software-engineering-portfolio)</sub>
