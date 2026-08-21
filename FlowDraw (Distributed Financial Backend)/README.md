# FlowDraw

A secure, scalable, and fully on-chain lottery smart contract featuring a dynamic hybrid draw trigger, oracle-free randomness, and a gas-optimized pull-payment system. 

## Key Features

- **Hybrid Draw Trigger:** Draws initiate dynamically based on either a configurable time interval or a maximum ticket capacity threshold—preventing stagnation on slow weeks and eliminating wait times on hot weeks.
- **Oracle-Free Randomness:** Utilizes future-block hashes seeded with the contract address to generate verifiable randomness, completely eliminating external oracle fees (e.g., Chainlink).
- **Pull-Payment Architecture:** Distributes prizes via a pull pattern (`claimPrize`), ensuring that a single winner's failed transfer cannot revert the entire draw.
- **Gas-Optimized:** Replaced $O(N)$ linear scans with $O(1)$ constant-time mappings for participant verification, keeping gas costs flat regardless of player volume.
- **Strict State Safeguards:** Idempotent draw finalization, re-entrancy protection (Checks-Effects-Interactions), and strict minimum participant/ticket requirements.

## How It Works

### 1. The Setup
- **Ticket Cost:** 0.015 ETH
- **Distribution:** 80% to the decentralized Prize Pool, 20% to the Host.
- **Parameters:** The host can dynamically adjust the draw interval (1-30 days), participant minimums (2-100), and ticket caps, bounded by strict safety checks.

### 2. The Hybrid Trigger
The contract does not force users to wait for a rigid timer if demand is high, nor does it force a draw if participation is too low.
```solidity
// Triggers if the time interval has passed OR the ticket cap is reached
bool timeConditionMet = (lastDrawTime == 0) || (block.timestamp >= lastDrawTime + drawInterval);
bool ticketCapReached = totalTickets >= maxTicketsPerDraw;