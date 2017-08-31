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







