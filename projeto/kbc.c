#include <minix/syslib.h>
#include <minix/drivers.h>
#include <minix/sysutil.h>
#include "kbc.h"
#include "i8042.h"

int kbc_read_status_buf(unsigned char *status)
{
	return sys_inb(CMD_PORT, (unsigned long *) status);
}

int kbc_write_in_buf(unsigned char data)
{
	unsigned char status;
	while(1)
	{
		if (kbc_read_status_buf(&status) != OK)
			return 1;

		if (!(status & IN_BUF_FULL))
		{
			return sys_outb(IN_BUF, data);
		}
		tickdelay(micros_to_ticks(DELAY_US));
	}
}

int kbc_read_out_buf(unsigned char *data)
{
	unsigned char st;
	while(1)
	{
		if (kbc_read_status_buf(&st) != OK)
			return 1;

		if (st & OUT_BUF_FULL)
		{
			if (sys_inb(OUT_BUF, (unsigned long *) data) != OK)
				return 1;
			return 0;
		}
		tickdelay(micros_to_ticks(DELAY_US));
	}
}

int kbc_send_command(unsigned char command)
{
	unsigned char status;
	while(1)
	{
		if (kbc_read_status_buf(&status) != OK)
			return 1;

		if (!(status & IN_BUF_FULL))
		{
			return sys_outb(CMD_PORT, command);
		}
		tickdelay(micros_to_ticks(DELAY_US));
	}
}

int kbc_send_command_with_arg(volatile unsigned char command, volatile unsigned char arg)
{

	if (kbc_send_command(command) != OK)
		return 1;

	return kbc_write_in_buf(arg);
}



int mouse_send_command(unsigned char command)
{
	static unsigned char st;
	do
	{
		if (kbc_send_command_with_arg(WRITE_AUX, command) != OK)
			return 1;
		do
		{
			if (kbc_read_out_buf(&st) != OK)
				return 1;

		} while (st != 0xFA && st != 0xFE && st != 0xFC);
	} while (st != 0xFA);

	return OK;
}
