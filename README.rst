=======================
libusb-1.0 PowerUSB 
=======================

PowerUSB ( http://www.pwrusb.com/ ) をlibusb-1.0 経由で操作する予定のプログラムです。

動作環境
-------------------

debian/sid on x86_64 とlibusb-1.0.9~rc3-3 で動作確認しています。

環境に合わせてヘッダファイルやMakefile等書き換えてください

必要なライブラリ等については、自分で調べてください。





実行方法
-------------------

::

 $ make 
 $ sudo ./powerusb
 Model:Basic
 firmware version: 2.1
 Outlet1:off
 Outlet2:off
 Outlet3:on

