#include "../include/msg.h"
#include <stdio.h>

uint8_t calc_parity_bit(client_msg_t msg)
{
	uint8_t counter = 0;

	msg &= ~PARITY_BIT;

	while (msg) {
		counter += msg % 2;
		msg >>= 1;
	}
	return counter % 2;
}


uint8_t get_parity_bit(client_msg_t msg)
{
	debug_print("Get parity bit of %04x\n", msg);
	return (uint8_t)msg >> PARITY_POS;
}


bool check_parity(client_msg_t msg)
{
	return calc_parity_bit(msg) == get_parity_bit(msg);
}

client_msg_t set_parity_bit(client_msg_t msg, client_msg_t parity_bit)
{
	parity_bit <<= PARITY_POS;

	msg &= ~parity_bit;
	msg |= parity_bit;

	return msg;
}

coordinate_t get_coordinates(client_msg_t msg)
{
	coordinate_t c;

	c.col = (uint8_t)((msg >> X_COORDINATE_OFFSET) & COORDINATE_BITS);
	c.row = (uint8_t)(msg & COORDINATE_BITS);

	return c;
}

status_t get_status(server_msg_t msg)
{
	return (status_t)(msg & 0xc);
}

hit_report_t get_hit_report(server_msg_t msg)
{
	return (hit_report_t)(msg & (server_msg_t)3);
}