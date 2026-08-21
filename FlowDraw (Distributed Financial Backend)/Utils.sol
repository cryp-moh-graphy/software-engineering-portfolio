// SPDX-License-Identifier: MIT
pragma solidity ^0.8.18;

import "contract/Player.sol";

/**
 * @notice Evaluates participant tickets against the winning sequence to determine jackpot recipients.
 * @dev Executes a two-pass algorithm: the first pass calculates the total number of winning tickets 
 * to dynamically allocate memory, and the second pass populates the winners array. 
 * Operates exclusively in memory to minimize state-read gas costs.
 * @param totalPrizePool The total accumulated prize pool in Wei available for distribution.
 * @param winningNumbers A fixed array of 5 integers representing the drawn lottery sequence.
 * @param addresses An array of participant addresses corresponding index-to-index with the players array.
 * @param players An array of Player structs containing ticket data for each participant.
 * @return winners An array of addresses that hold a winning ticket.
 * @return winAmount The proportionate share of the prize pool allocated to each winning ticket.
 */
function determinePrizes(
    uint256 totalPrizePool,
    uint8[5] memory winningNumbers,
    address[] memory addresses,
    Player[] memory players
) pure returns (address[] memory winners, uint256 winAmount) {
    require(addresses.length == players.length, "Length mismatch");

    uint256 totalJackpotWinners = 0;

    // Pass 1: Count the total number of winning tickets to allocate memory
    for (uint256 i = 0; i < players.length; i++) {
        uint256 n = players[i].numberOfTickets;

        for (uint256 j = 0; j < n; j++) {
            uint8[5] memory ticket = players[i].ticketNumbers[j];

            bool isJackpotWinner = true;
            for (uint8 x = 0; x < 5; x++) {
                if (ticket[x] != winningNumbers[x]) {
                    isJackpotWinner = false;
                    break;
                }
            }

            if (isJackpotWinner) {
                totalJackpotWinners += 1;
            }
        }
    }

    if (totalJackpotWinners == 0) {
        return (new address[](0), 0);
    }
    
    // Pass 2: Populate the dynamically sized array with winning addresses
    winners = new address[](totalJackpotWinners);
    uint256 winnerIndex = 0;
    uint256 share = totalPrizePool / totalJackpotWinners;

    for (uint256 i = 0; i < players.length; i++) {
        uint256 n = players[i].numberOfTickets;

        for (uint256 j = 0; j < n; j++) {
            bool isJackpotWinner = true;

            for (uint256 x = 0; x < 5; x++) {
                if (players[i].ticketNumbers[j][x] != winningNumbers[x]) {
                    isJackpotWinner = false;
                    break;
                }
            }

            if (isJackpotWinner) {
                winners[winnerIndex] = addresses[i];
                winnerIndex++;
            }
        }
    }

    return (winners, share);
}