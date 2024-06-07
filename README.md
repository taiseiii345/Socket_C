# Socket_C

## Overview
C言語で記述したソケット通信プログラムです．
Soxを用いた双方向の通信が可能となっています．

## Requirement
Sox

## Usage
- コンパイル
```sh
gcc -o Socket_phone Socket_phone.c
```
でコンパイル．

- サーバー側
```sh
$ rec -t raw -b 16 -c 1 -e s -r 44100 - | ./Socket_phone (port) | play -t raw -b 16 -c 1 -e s -r 44100 -
```
(port)には好きなポート番号を入れる．サーバーが立ちあがる．

- クライアント側
```sh
rec -t raw -b 16 -c 1 -e s -r 44100 - | ./Socket_phone (IP_addres) (port) | play -t raw -b 16 -c 1 -e s -r 44100 -
```
(IP_addres)および(port)にはサーバー側のIPアドレス，ポート番号を入力する．
通信が始まる．