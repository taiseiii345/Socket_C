# ソケット通信を用いた電話プログラム

## Overview
C言語で記述したソケット通信プログラムです．
Soxを用いた双方向の音声通信が可能となっています．


<img width="1020" alt="client" src="https://github.com/user-attachments/assets/4d3f260f-e91f-4270-8697-cafcdefd57a4">

<img width="1020" alt="client" src="https://github.com/user-attachments/assets/f219690e-082c-4921-a75b-e930a29491ba">

## Requirement
Sox

## Usage
- コンパイル
```sh
gcc -o Socket_phone Socket_phone.c
```

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

- 着信音
- 留守電
- ミュート・消音・ボイスチェンジ・ノイズ除去