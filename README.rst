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

$make 
$ sudo ./powerusb
init done
device opened
send_cmd:aa
send_cmd:read:0101
send_cmd:a7
send_cmd:read:0201
send_cmd:a1
send_cmd:read:0001
send_cmd:a2
send_cmd:read:0001
send_cmd:ac
send_cmd:read:0101


