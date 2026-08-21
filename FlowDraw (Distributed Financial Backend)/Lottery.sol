// SPDX-License-Identifier: MIT
pragma solidity ^0.8.18;

import "contract/Player.sol";
import "contract/ILottery.sol";
import "utils/Utils.sol";

/**
 * @title Lottery
 * @notice Implements a flexible hybrid lottery system using deterministic blockhash randomness.
 * @dev Draws execute dynamically based on strictly enforced minimums, triggered by either 
 * elapsed time intervals or a maximum ticket capacity threshold to prevent stagnation.
 */
contract Lottery is ILottery {

    // -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    // State Variables & Storage
    // -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

    mapping(address => Player) public participantsToTickets;
    uint16 private totalTickets = 0;
    address[] participantAddresses;

    address payable public host;

    uint256 public prizePool = 0;
    uint256 private hostCut = 0;
    uint256 public ticketCost = 0.015 ether;
    uint256 private hostTicketFee = (ticketCost * 20) / 100;

    uint256 public drawBlockNumber; 
    bool public drawInitiated = false; 
    bool public ended = false; 
    uint256 public lastDrawTime = 0; 

    mapping(address => uint256) public claimableWinnings; 

    uint256 public drawInterval; 
    uint16 public minParticipants; 
    uint16 public minTicketsSold; 
    uint16 public maxTicketsPerDraw; 

    // -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    // Constructor & Modifiers
    // -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

    constructor(
        uint256 _drawInterval,
        uint16 _minParticipants,
        uint16 _minTicketsSold,
        uint16 _maxTicketsPerDraw
    ) {
        host = payable(msg.sender);

        require(_drawInterval >= 1 days, "Interval must be at least 1 day");
        require(_drawInterval <= 30 days, "Interval cannot exceed 30 days");
        drawInterval = _drawInterval;

        require(_minParticipants >= 2, "Need at least 2 participants");
        require(_minParticipants <= 100, "Too high - would never trigger");
        minParticipants = _minParticipants;

        require(
            _minTicketsSold >= _minParticipants,
            "Min tickets should be >= min participants"
        );
        require(_minTicketsSold <= 1000, "Too high - would never trigger");
        minTicketsSold = _minTicketsSold;

        require(
            _maxTicketsPerDraw >= _minTicketsSold,
            "Max must be >= min tickets"
        );
        require(_maxTicketsPerDraw <= 10000, "Too high - unrealistic cap");
        maxTicketsPerDraw = _maxTicketsPerDraw;
    }

    modifier onlyHost() {
        require(msg.sender == host, "You are not the host.");
        _;
    }

    // -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    // Administrative Functions
    // -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

    function setDrawInterval(uint256 _interval) external onlyHost {
        require(_interval >= 1 days, "Interval must be at least 1 day");
        require(_interval <= 30 days, "Interval cannot exceed 30 days");
        drawInterval = _interval;
    }

    function setMinParticipants(uint16 _minParticipants) external onlyHost {
        require(_minParticipants >= 2, "Need at least 2 participants");
        require(_minParticipants <= 100, "Too high - would never trigger");
        minParticipants = _minParticipants;
    }

    function setMinTicketsSold(uint16 _minTickets) external onlyHost {
        require(
            _minTickets >= minParticipants,
            "Min tickets should be >= min participants"
        );
        require(_minTickets <= 1000, "Too high - would never trigger");
        minTicketsSold = _minTickets;
    }

    function setMaxTicketsPerDraw(uint16 _maxTickets) external onlyHost {
        require(_maxTickets >= minTicketsSold, "Max must be >= min tickets");
        require(_maxTickets <= 10000, "Too high - unrealistic cap");
        maxTicketsPerDraw = _maxTickets;
    }

    // -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    // Core Lottery Logic
    // -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

    /**
     * @notice Processes ticket purchases and distributes the administrative fee.
     * @dev Utilizes a low-level call for fee transfer to mitigate standard transfer constraints.
     */
    function buyTicket(uint8[5] calldata ticketNumbers) external payable {
        require(!drawInitiated, "Cannot buy tickets during active draw");
        require(
            msg.value >= ticketCost,
            "Wrong amount of ether to join. Please pay at least 0.015 ether. Any more will be considered a tip."
        );
        validateTicketNumbers(ticketNumbers);

        hostCut = hostTicketFee;
        prizePool += msg.value - hostCut;

        (bool sent, ) = host.call{value: hostCut}("");
        require(sent, "Owner was not payed.");

        participantsToTickets[msg.sender].ticketNumbers.push(ticketNumbers);
        participantsToTickets[msg.sender].numberOfTickets += 1;

        if (!participantsToTickets[msg.sender].hasJoined) {
            participantsToTickets[msg.sender].hasJoined = true;
            participantAddresses.push(msg.sender);
        }

        totalTickets += 1;

        emit ParticipantJoined(msg.sender, "joined");
    }

    /**
     * @notice Validates draw thresholds and initiates the entropy generation phase.
     * @dev Requests randomness by designating a future block to mitigate miner manipulation.
     */
    function startLottery() external {
        require(!drawInitiated, "Draw already initiated.");

        require(
            participantAddresses.length >= minParticipants,
            "Not enough participants to start draw."
        );
        require(
            totalTickets >= minTicketsSold,
            "Not enough tickets sold to start draw."
        );

        bool timeConditionMet = (lastDrawTime == 0) ||
            (block.timestamp >= lastDrawTime + drawInterval);
        bool ticketCapReached = totalTickets >= maxTicketsPerDraw;

        require(
            timeConditionMet || ticketCapReached,
            "Draw conditions not met. Wait for time interval or ticket cap."
        );

        drawBlockNumber = block.number + 5;
        drawInitiated = true;
        lastDrawTime = block.timestamp;
    }

    /**
     * @notice Resolves the lottery draw, evaluates winning combinations, and allocates funds.
     * @dev Derives randomness from the previously assigned target blockhash. Utilizes a lock 
     * flag (`ended`) to prevent re-execution and strictly updates claimable balances for gas efficiency.
     */
    function findWinner() external {
        require(drawInitiated, "Draw not initiated.");
        require(
            block.number >= drawBlockNumber,
            "Target block hasn't arrived yet."
        );
        require(!ended, "Lottery already ended.");

        bytes32 futureBlockHash = blockhash(drawBlockNumber);

        require(
            futureBlockHash != bytes32(0),
            "Block hash unavailable or too old."
        );

        uint256 randomSeed = uint256(
            keccak256(abi.encodePacked(futureBlockHash, address(this)))
        );

        uint8[5] memory winningNumbers;
        for (uint256 i = 0; i < 5; i++) {
            winningNumbers[i] = uint8(
                (uint256(keccak256(abi.encodePacked(randomSeed, i))) % 99) + 1
            );
        }

        uint256 numPlayers = participantAddresses.length;
        Player[] memory players = new Player[](numPlayers);
        for (uint256 i = 0; i < numPlayers; i++) {
            players[i] = participantsToTickets[participantAddresses[i]];
        }

        (address[] memory winners, uint256 amountPerWinner) = determinePrizes(
            prizePool,
            winningNumbers,
            participantAddresses,
            players
        );

        ended = true;

        if (winners.length > 0) {
            for (uint256 i = 0; i < winners.length; i++) {
                claimableWinnings[winners[i]] += amountPerWinner;
                emit LotteryWinner(winners[i], uint64(amountPerWinner));
            }
        }

        prizePool = 0;
        totalTickets = 0;
        drawInitiated = false;
        ended = false;

        for (uint256 i = 0; i < participantAddresses.length; i++) {
            delete participantsToTickets[participantAddresses[i]];
        }
        delete participantAddresses;
    }

    /**
     * @notice Executes a pull-payment transfer for the caller's allocated winnings.
     */
    function claimPrize() external {
        uint256 amount = claimableWinnings[msg.sender];
        require(amount > 0, "No winnings to claim");

        claimableWinnings[msg.sender] = 0;

        (bool success, ) = payable(msg.sender).call{value: amount}("");
        require(success, "Prize transfer failed");

        emit PrizeClaimed(msg.sender, amount);
    }

    /**
     * @notice Evaluates whether a specific address is currently participating in the active lottery iteration.
     * @dev Operates in O(1) time complexity utilizing a direct state mapping.
     */
    function getParticipantStatus(address user) public view returns (bool) {
        return participantsToTickets[user].hasJoined;
    }

    /**
     * @notice Enforces boundary checks and strict uniqueness constraints on ticket number arrays.
     */
    function validateTicketNumbers(
        uint8[5] calldata ticketNumbers
    ) internal pure {
        for (uint256 i = 0; i < 5; i++) {
            require(
                ticketNumbers[i] >= 1 && ticketNumbers[i] <= 99,
                "Out of range"
            );
            for (uint256 j = i + 1; j < 5; j++) {
                require(
                    ticketNumbers[i] != ticketNumbers[j],
                    "Duplicate in ticket"
                );
            }
        }
    }
}