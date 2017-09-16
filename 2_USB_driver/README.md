# USB driver

紀錄一些USB driver實作細節與閱讀開源碼後之筆記。

利用指令可看見連接上的USB裝置之細節。

```
$lsusb -v
```

其中重要的為`idVendor`、`idProduct`、`device name`、`endpoint`和`Transfer Type`。在以下程式碼中必須填入裝置ID。

```
static const struct usb_device_id gamepad_table[] = {
	{ USB_DEVICE(USB_VENDOR_ID, USB_PRODUCT_ID) },
	{ }					/* Terminating entry */
};
```
在usb_driver struct中必須寫入剛剛找到的`device name`，如此當裝置插入時才能對應到此驅動程式。

```
static struct usb_driver gamepad_driver = {
	.name =		"Your device name",
	.probe =	gamepad_probe,
	.disconnect =	gamepad_disconnect,
	.id_table =	gamepad_table,
};
```

### Probe function

當裝置插入時，若OS認為此驅動可處理就會呼叫probe函式。為此idVendor、idProduct與device name必須與裝置相同。若都設置好了卻無法呼叫probe函式，建議拔除裝置後重新啟動(有可能每次移除mod後都需重啟)。

此函式最需要做`得到interface`這動作，詳細如下。

```
dev->udev = usb_get_dev(interface_to_usbdev(interface));
```

必須讓驅動得到介面(interface)並轉為`struct usb_device`，如此才能做許多動作。在原碼是使用自己的data structure來完成，若是簡單的驅動可直接使用靜態變數。

接著要得到endpoint，範例中是用系統自動搜尋的方式原則上也建議使用此方法，也可以直接用指令找出endpoint並直接寫死於系統中。必須要特別注意以下的動作

```
if (!dev->inte_in_endpointAddr &&
	     usb_endpoint_is_int_in(endpoint))
if (!dev->inte_out_endpointAddr &&
		    usb_endpoint_is_int_out(endpoint))
```

在這裡我的裝置是遊戲手柄(gamepad)，因此是使用`usb_endpoint_is_int_in`這個函式，在原始碼中則是`usb_endpoint_is_bulk_in`。USB裝置有4種傳輸方式，分別是`control`、`interrupt`、`bulk`與`isochronous`，儲存裝置多為bulk，而鍵盤、滑鼠和手柄等多為interrupt，使用指令事先查詢endpoint的傳輸形式並更改相對應的指令以免產生錯誤。

在probe時也可設定urb(USB Request Blocks)，簡單來說就是在USB上傳輸的資料。

### 插入裝置後

若成功probe，此時在/dev/目錄下應該會有你的裝置，可以在前面的usb_class_driver更改名子，此時的名子就會是Your_device_name0，可嘗試使用char device的讀寫方式變動檔案，此時就會調用相對應的函式。有時會需要使用chmod 666來相對應的權限，直接用sudo也可以。

### read/write

在這裡因我使用的裝置式手柄，因此將寫入的函式刪除了。需要注意的為`usb_rcvintpipe`，跟剛剛的傳輸形式有關原始碼用的式bulk在此改為interrupt。在read中可以直接使用`usb_interrupt_msg`此函式將urb包在裡面，若資訊量不多可直接使用。

### 結語

在virtualbox虛擬機器下使用ubuntu，結果無法得到資訊不知道為啥。最後，鍵盤、滑鼠等有HID介面可較簡單完成，而且也有很多手柄驅動可參考，有時間再研究。

---

### reference

<http://elixir.free-electrons.com/linux/v4.10/source/drivers/usb/usb-skeleton.c>
<http://elixir.free-electrons.com/linux/v4.10/source/drivers/hid/usbhid/usbmouse.c#L64>





