/**
 * @file msg.h
 * @author Matthias Pichler, 01634256
 * @date 2018-04-10
 *
 * @brief Message type definitions for OSUE exercise 1B `Battleship' client and
 * server.
 */

#ifndef MSG_H
#define MSG_H

#include "../include/common.h"
#include <stdint.h>

typedef uint16_t client_msg_t;
typedef uint8_t server_msg_t;

#define PARITY_BIT 0x800  // bitmask of the parity bit
#define PARITY_POS 15	 // position of the parity bit

/**
 * @brief calculate the parity bit for the given message
 * @details counts the number of 1 bits in the message and returns 1 if it was
 * odd and 0 if it was even. Therefore the return value can be used to set the
 * parity of the message. If a parity bit is already set for the message it is
 * ignored.
 */
uint8_t calc_parity_bit(client_msg_t msg);

/**
 * @brief checks the correctness of the parity bit of the given message
 * @param msg the message to check
 * @return true if the parity bit is correct, false otherwise
 */
bool check_parity(client_msg_t msg);

/**
 * @brief return the parity bit of a given message
 * @param msg the message to read the parity bit format
 * @return the parity bit of the message. Either 1 or 0
 */
uint8_t get_parity_bit(client_msg_t msg);

/**
 * @brief set the parity bit of a message
 * @details sets the parity bit of the given message with the least significant
 * bit of the given parity_bit parameter. No inputs are validated
 * @param msg the message to add the parity bit to. Any set parity bit of the
 * message is discarded.
 * @param parity_bit the parity bit to be set. Only the least significant bit is
 * used.
 * @return a copy of the original message, with the parity bit set.
 */
client_msg_t set_parity_bit(client_msg_t msg, client_msg_t parity_bit);

/**
 * @brief parse a message and extract the coordinates
 * @param msg the message to be parsed
 * @return the coordinates given by the message
 */
coordinate_t get_coordinates(client_msg_t msg);

/**
 * @brief get the hit report from the servers response
 * @param msg the response message from the server
 * @return the hit report of the message
 */
hit_report_t get_hit_report(server_msg_t msg);

/**
 * @brief get the status code from the server response
 * @param msg the response msg from the server
 * @return the status code of the message
 */
status_t get_status(server_msg_t msg);


#endif  // MSG_H