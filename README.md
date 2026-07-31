<div id="top">

<div align="center">

# FRTA Program

<em>学生フォーミュラ向けのCAN制御・車両制御プログラム群である。</em>

<em>MCP2515ベースのCAN通信を用い、BSE、APPS、モーター制御の各要素を分けて開発している。</em>

<br>

<img src="https://img.shields.io/badge/Arduino-00979D.svg?style=default&logo=Arduino&logoColor=white" alt="Arduino">
<img src="https://img.shields.io/badge/C-A8B9CC.svg?style=default&logo=C&logoColor=black" alt="C">
<img src="https://img.shields.io/badge/C%2B%2B-00599C.svg?style=default&logo=C%2B%2B&logoColor=white" alt="C++">
<img src="https://img.shields.io/badge/CAN-1F6FEB.svg?style=default&logo=canva&logoColor=white" alt="CAN">
<img src="https://img.shields.io/badge/SPI-6E4C13.svg?style=default&logoColor=white" alt="SPI">

</div>
<br>

---

## Table of Contents

- [Table of Contents](#table-of-contents)
- [Overview](#overview)
- [Features](#features)
- [Project Structure](#project-structure)
  - [Project Index](#project-index)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Usage](#usage)
- [CAN ID Roles](#can-id-roles)

---

## Overview

<br>このリポジトリは、学生フォーミュラ向けのCAN制御コードをまとめたものである。</br>
<br> APPS、BSE、モーター制御を個別に検証できるように分割してあり、MCP2515ベースのCAN通信を前提としている。ECU側はまだ作製途中。</br>

---

## Features

- APPSの2系統入力を監視し、異常差分を検出する
- BSEのブレーキ入力とブレーキランプ制御を行う
- CAN ID 0x50, 0x51, 0x100, 0x101 を使った車両内通信を行う
- モーター制御側でのアクセル値反映とPID制御の試作を含む

---

## Project Structure

```text
program/
├── CAN_BSE/
│   └── CAN_BSE.ino
├── ECU/
│   ├── APPS.cpp
│   ├── APPS.h
│   ├── BSE.cpp
│   ├── BSE.h
│   ├── ECU.h
│   └── ECU.ino
├── mcp_can/
│   ├── mcp_can.cpp
│   ├── mcp_can.h
│   └── mcp_can_dfs.h
├── motor/
│   ├── PID_test.cpp
│   └── CAN_MotorControl_Demo/
│       ├── CAN_MotorControl_Demo.ino
│       ├── NI_CU.cpp
│       ├── NI_CU.h
│       └── README.md
└── readme-ai.md
```

### Project Index

<details open>
	<summary><b><code>program/</code></b></summary>
	<details>
		<summary><b>CAN_BSE</b></summary>
		<blockquote>
			<p>BSE側のCAN送信テスト用スケッチ。</p>
		</blockquote>
	</details>
	<details>
		<summary><b>ECU</b></summary>
		<blockquote>
			<p>APPSとBSEをまとめるECU処理である。まだ作製途中。</p>
		</blockquote>
	</details>
	<details>
		<summary><b>mcp_can</b></summary>
		<blockquote>
			<p>MCP2515用CANライブラリ一式。</p>
		</blockquote>
	</details>
	<details>
		<summary><b>motor</b></summary>
		<blockquote>
			<p>モーターインバータ向けCAN制御のサンプルとPID試作を含む。</p>
		</blockquote>
	</details>
</details>

---

## Getting Started

### Prerequisites

- Arduino IDE
- MCP2515 CANモジュール
- 対応するArduinoボード
- 配線済みのCANバス

### Usage

各フォルダは個別のスケッチや検証コードとして扱う。

- ECUのメイン処理は [ECU/ECU.ino](ECU/ECU.ino) 
- BSE単体確認は [CAN_BSE/CAN_BSE.ino](CAN_BSE/CAN_BSE.ino) 
- モーター制御のサンプルは [motor/CAN_MotorControl_Demo/CAN_MotorControl_Demo.ino](motor/CAN_MotorControl_Demo/CAN_MotorControl_Demo.ino) 

---

## CAN ID Roles

- 0x49: BSE側の同期・受信確認に使うID
- 0x50: BSEから送信するブレーキ値と状態情報のID
- 0x51: APPSから送信するアクセル状態と異常情報のID
- 0x100: モーター制御側から送信する制御フレームのID
- 0x101: モーター制御側が受信するモータ応答データのID

</div>
