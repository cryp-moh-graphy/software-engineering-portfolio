// SPDX-License-Identifier: MIT
pragma solidity ^0.8.18;

import "contract/Player.sol";

/**
 * @title ILottery
 * @notice Defines the standard interface for lottery execution, ticket management, and prize distribution.
 */
interface ILottery {
    /// @notice Emitted when a new address successfully purchases a ticket and enters the pool.
    /// @param participant The address entering the lottery.
    /// @param alert Supplemental status or notification message.
    event ParticipantJoined(address participant, string alert);

    /// @notice Emitted when the draw concludes and a winning address is selected.
    /// @param winner The address awarded the current prize pool.
    /// @param prize The total prize amount allocated to the winner.
    event LotteryWinner(address winner, uint64 prize);

    /// @notice Emitted when a winner successfully withdraws their allocated funds.
    /// @param winner The address initiating the withdrawal.
    /// @param amount The total ETH transferred to the winner.
    event PrizeClaimed(address winner, uint256 amount);

    /// @notice Configures the mandatory time delay between consecutive lottery draws.
    /// @param _interval The new interval duration in seconds. Must fall within a 1 to 30-day range.
    function setDrawInterval(uint256 _interval) external;

    /// @notice Establishes the minimum distinct participant threshold required to execute a draw.
    /// @param _minParticipants The new participant minimum. Bounded between 2 and 100.
    function setMinParticipants(uint16 _minParticipants) external;

    /// @notice Defines the baseline number of tickets that must be sold before a draw can occur.
    /// @param _minTickets The new ticket minimum. Must equal or exceed the minimum participant count.
    function setMinTicketsSold(uint16 _minTickets) external;

    /// @notice Sets the maximum ticket threshold that, when reached, forces an immediate draw bypass.
    /// @param _maxTickets The new ticket maximum. Must strictly exceed or equal the minimum tickets sold limit.
    function setMaxTicketsPerDraw(uint16 _maxTickets) external;

    /// @notice Facilitates the purchase of a single lottery ticket using a specific number combination.
    /// @param ticketNumbers A fixed array of 5 unique integers, each bounded between 1 and 99.
    function buyTicket(uint8[5] calldata ticketNumbers) external payable;

    /// @notice Initiates the randomization and draw phase of the active lottery.
    /// @dev Execution reverts unless all minimum time, participant, and ticket thresholds are strictly satisfied.
    function startLottery() external;

    /// @notice Concludes the active draw phase and determines the winning ticket.
    /// @dev Execution requires prior invocation of startLottery() and validation that the target randomness block has been mined.
    function findWinner() external;

    /// @notice Executes a pull-payment transfer for the caller's allocated winnings.
    /// @dev Implements the pull-over-push pattern to mitigate reentrancy and denial-of-service vectors during prize distribution.
    function claimPrize() external;

    /// @notice Evaluates whether a specific address has active tickets in the current lottery iteration.
    /// @param user The target address to evaluate.
    /// @return True if the address possesses at least one active ticket, otherwise false.
    function getParticipantStatus(address user) external view returns (bool);

    /// @notice Retrieves the total undistributed winnings allocated to a specific address.
    /// @param user The target address to query.
    /// @return The total claimable balance in Wei.
    function claimableWinnings(address user) external view returns (uint256);

    /// @notice Retrieves the designated host or administrative address of the lottery contract.
    /// @return The payable address of the contract host.
    function host() external view returns (address payable);

    /// @notice Retrieves the current accumulated prize pool available for the active draw.
    /// @return The total prize pool balance in Wei.
    function prizePool() external view returns (uint256);

    /// @notice Retrieves the fixed ETH cost required to purchase a single lottery ticket.
    /// @return The ticket price in Wei.
    function ticketCost() external view returns (uint256);

    /// @notice Retrieves the future block number designated for entropy generation.
    /// @return The target block number used for cryptographic randomization.
    function drawBlockNumber() external view returns (uint256);

    /// @notice Evaluates the operational state of the current lottery draw.
    /// @return True if the draw phase has been initiated but not yet concluded.
    function drawInitiated() external view returns (bool);

    /// @notice Retrieves the timestamp recording the conclusion of the most recent draw.
    /// @return The precise Unix timestamp of the last finalized draw.
    function lastDrawTime() external view returns (uint256);

    /// @notice Retrieves the currently configured time delay required between draws.
    /// @return The interval duration in seconds.
    function drawInterval() external view returns (uint256);

    /// @notice Retrieves the currently configured minimum participant threshold.
    /// @return The minimum number of distinct addresses required to initiate a draw.
    function minParticipants() external view returns (uint16);

    /// @notice Retrieves the currently configured minimum ticket sales threshold.
    /// @return The minimum total tickets required to initiate a draw.
    function minTicketsSold() external view returns (uint16);

    /// @notice Retrieves the currently configured upper limit for ticket sales per iteration.
    /// @return The maximum ticket threshold that forces an early draw execution.
    function maxTicketsPerDraw() external view returns (uint16);
}