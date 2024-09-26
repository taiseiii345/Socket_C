# ソケット通信を用いた電話プログラム

## Overview
C言語で記述したソケット通信プログラムです．
Soxを用いた双方向の音声通信が可能となっています．

<img src="./fig/flowchart_client.pdf" alt="client" width="400">


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
./Socket_phone (port)
```
(port)には好きなポート番号を入力し、サーバーが立ちあがる．

- クライアント側
```sh
./Socket_phone (IP_addres) (port)
```
(IP_addres)および(port)にはサーバー側のIPアドレス，ポート番号を入力し、接続が開始する．

## Feature