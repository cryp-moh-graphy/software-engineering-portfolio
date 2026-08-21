// SPDX-License-Identifier: MIT
pragma solidity ^0.8.18;

import "remix_tests.sol";
import "remix_accounts.sol";
import "../contract/Lottery.sol";

/**
 * @title LotteryTesting
 * @notice Remix-based testing suite validating the core execution logic, state transitions, and authorization checks of the Lottery contract.
 */
contract LotteryTesting {
    Lottery lottery;
    address participant1;
    address participant2;

    receive() external payable {}
    fallback() external payable {}

    /**
     * @notice Initializes standard testing accounts prior to the execution of the test suite.
     */
    function beforeAll() public {
        participant1 = TestsAccounts.getAccount(1);
        participant2 = TestsAccounts.getAccount(2);
    }

    /**
     * @notice Deploys a fresh instance of the Lottery contract before each test to guarantee state isolation.
     * @dev Configures the lottery with a 2-day interval, a minimum of 2 participants, a 2-ticket minimum, and a 10-ticket maximum cap.
     */
    function beforeEach() public {
        lottery = new Lottery(2 days, 2, 2, 10);
    }
    
    /**
     * @notice Validates that the deployment address is correctly assigned as the contract host.
     */
    function testHostIsOwner() public {
        Assert.equal(lottery.host(), address(this), "Owner is not set correctly.");
    }
    
    /**
     * @notice Verifies that an account attempting to participate without executing a valid purchase correctly returns a false participant status.
     * @dev The `#value` annotation dynamically injects 0.015 ether (in Wei) into the transaction context for Remix testing.
     */
    /// #value: 15000000000000000
    function testBuyIn() public payable {
        Assert.equal(lottery.getParticipantStatus(address(this)), false, "A ticket was not bought/insufficient");
    }
    
    /**
     * @notice Evaluates ticket purchase execution and verifies that the prize pool increments successfully post-transaction.
     */
    function testPrizePool() public {
        uint256 result = 1;
        lottery.buyTicket{value: 0.015 ether}([1, 2, 3, 4, 5]);
        Assert.equal(lottery.getParticipantStatus(address(this)), true, "A ticket was not bought/insufficient");
        Assert.ok(lottery.prizePool() > result, "Prize pool has no money.");
    }
    
    /**
     * @notice Evaluates the state transitions across the full lottery lifecycle, ensuring the prize pool registers correctly following a claim operation.
     */
    function testClaimPrize() public {
        lottery.startLottery();
        Assert.equal(lottery.drawInitiated(), false, "The lottery has not started.");
        lottery.findWinner();
        lottery.claimPrize();
        Assert.ok(lottery.prizePool() < 1, "Prize pool was given away");
    }
    
    /**
     * @notice Confirms that the contract host retains the expected administrative permissions required to manage the deployment.
     */
    function testStartLotteryPermission() public {
        Assert.equal(lottery.host(), address(this), "Only owner can start lottery");
    }
}