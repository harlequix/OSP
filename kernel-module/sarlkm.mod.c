#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xcbef08d, "module_layout" },
	{ 0xd7fae2d6, "register_sysctl_table" },
	{ 0xc274e480, "remove_proc_entry" },
	{ 0x3c2c5af5, "sprintf" },
	{ 0x7d11c268, "jiffies" },
	{ 0xb4390f9a, "mcount" },
	{ 0xe2cde27, "proc_dostring" },
	{ 0x48f993d3, "unregister_sysctl_table" },
	{ 0x43a6b9f6, "create_proc_entry" },
	{ 0xd6c963c, "copy_from_user" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "7EBA6CD360A2285CDCE19BD");
