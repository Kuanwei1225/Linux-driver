# Linux-driver

Linux kernel driver 筆記與一些實作。

### 安裝環境

`$(uname -r)`取得目前核心的版本。

```
sudo apt-get install  build-essential  linux-headers-$(uname -r)
```

### 說明

驅動程式並沒有主程式，它是由各種functional pointer構成。當User space做System call時會由指標導向指定的函示。

---

### Example

Makefile：

```
CFILES := hello.c

obj-m := hello.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

Code：

```
#include <linux/module.h> //in /lib/modules/`uname –r`/build/include
#include <linux/init.h>

MODULE_LICENSE("Dual GSD/GPL");

static  int hello_init(void)
{
	printk(KERN_ALERT "hello driver loaded\n");
	return 0;
}

static void hello_exit(void)
{
	printk(KERN_ALERT "hello driver unloaded\n");
}
module_init(hello_init);
module_exit(hello_exit);
```

---

### 安裝模組

```
$ sudo insmod hello.ko
```

### 檢查檔案形式

```
$file hello.ko
```

### 檢查`printk`結果

```
sudo dmesg | tail -5
```

---

### printk

```
#define KERN_EMERG "<0>"   /* system is unusable                */
#define KERN_ALERT "<1>"   /* action must be taken immediately    */
#define KERN_CRIT "<2>"    /* critical conditions     */
#define KERN_ERR "<3>"     /* error conditions            */
#define KERN_WARNING "<4>" /* warning conditions      */
#define KERN_NOTICE "<5>"  /* normal but significant condition    */
#define KERN_INFO "<6>"    /* informational           */
#define KERN_DEBUG "<7>"   /* debug-level messages        */

printk(KERN_ALERT "hello driver\n");
```

---

### cdev

建構一個屬於該dev的虛擬點

最後` grep mydev /proc/devices | awk '{print $1;}' `是你裝置的Major number

```
$sudo mknod --mode=666 /dev/mydev0 c `grep mydev /proc/devices | awk '{print $1;}'` 0
```

也可以在程式中自動註冊。

----

### Debug



---

ref:<http://opensourceforu.com/tag/linux-device-drivers-series/page/2/>







