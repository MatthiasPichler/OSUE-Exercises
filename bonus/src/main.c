// module stuff
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/types.h>

#include "../include/debug.h"
#include "../include/secvault.h"

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Matthias Pichler <e1634256@student.tuwien.ac.at>");
MODULE_DESCRIPTION("Secure Vault");
MODULE_VERSION("1.1");

static dev_t device;

module_param(debug, bool, S_IRUGO);
MODULE_PARM_DESC(debug, "Enable debugging");

static int __init sv_init(void)
{
	debug_print("%s\n", "Init function called ...");

	int ret = 0;
	if ((ret = vault_setup()) < 0) {
		debug_print("%s:%d\n", "Vault setup failed", ret);
		vault_cleanup();
		return ret;
	}
	if ((ret = ctl_setup()) < 0) {
		debug_print("%s:%d\n", "Control setup failed", ret);
		vault_cleanup();
		ctl_cleanup();
		return ret;
	}
	debug_print("%s\n", "Done");
	return 0;
}

static void __exit sv_exit(void)
{
	debug_print("%s\n", "Exit function called...");
	vault_cleanup();
	ctl_cleanup();
	debug_print("%s\n", "Done");
}


module_init(sv_init);
module_exit(sv_exit);
