#!/usr/bin/env python3
"""
burn_audio.py — 将 WAV 音频烧录到 STM32 + W25Q16 Flash

用法:
    python burn_audio.py <串口号> <WAV文件>

示例:
    python burn_audio.py COM3 prompt.wav        # Windows
    python burn_audio.py /dev/ttyUSB0 prompt.wav # Linux

依赖:
    pip install pyserial

协议:
    1. 脚本发送 "burn <字节数>\r\n"
    2. MCU 擦除扇区后回复 "READY\r\n"
    3. 脚本分 256 字节一包发送, 每包等 MCU 回复 'K'
    4. 全部完成后 MCU 回复 "DONE..."

WAV 要求: 16kHz, 16-bit, mono (单声道)
    如果不是此格式, 脚本会自动提示
"""

import sys
import struct
import serial
import time


def wav_to_pcm(wav_path):
    """读取 WAV 文件, 返回 (raw PCM bytes, sample_rate, bits_per_sample, channels)"""
    with open(wav_path, "rb") as f:
        data = f.read()

    if data[:4] != b"RIFF" or data[8:12] != b"WAVE":
        raise ValueError("不是有效的 WAV 文件")

    # 解析 fmt 块
    fmt_offset = data.find(b"fmt ")
    if fmt_offset == -1:
        raise ValueError("找不到 fmt 块")

    fmt_size = struct.unpack_from("<I", data, fmt_offset + 4)[0]
    audio_fmt, channels, sample_rate, _, _, bits = struct.unpack_from(
        "<HHIIHH", data, fmt_offset + 8
    )

    if audio_fmt != 1:
        raise ValueError(f"不支持的音频格式 (需要 PCM=1, 当前={audio_fmt})")

    # 解析 data 块
    data_offset = data.find(b"data", fmt_offset)
    if data_offset == -1:
        raise ValueError("找不到 data 块")

    pcm_size = struct.unpack_from("<I", data, data_offset + 4)[0]
    pcm_data = data[data_offset + 8 : data_offset + 8 + pcm_size]

    return pcm_data, sample_rate, bits, channels


def burn(port, wav_path, baud=115200):
    # 1) 解析 WAV
    pcm, sr, bits, ch = wav_to_pcm(wav_path)

    print(f"WAV 信息: {sr}Hz, {bits}-bit, {ch}ch, {len(pcm)} bytes")
    duration = len(pcm) / (sr * (bits // 8) * ch)
    print(f"时长: {duration:.2f} 秒")

    if sr != 16000 or bits != 16 or ch != 1:
        print(f"警告: 推荐格式为 16kHz/16-bit/mono, 当前为 {sr}/{bits}/{ch}")
        ans = input("继续? (y/n): ").strip().lower()
        if ans != "y":
            return

    # W25Q16 容量 = 2MB
    if len(pcm) > 2 * 1024 * 1024:
        print(f"错误: 数据 {len(pcm)} 字节超过 W25Q16 容量 (2MB)")
        return

    # 2) 打开串口
    ser = serial.Serial(port, baud, timeout=10)
    time.sleep(0.1)
    ser.reset_input_buffer()

    # 3) 发送 burn 命令
    cmd = f"burn {len(pcm)}\r\n"
    print(f"发送命令: burn {len(pcm)}")
    ser.write(cmd.encode())

    # 4) 等待 READY (MCU 擦除扇区需要时间)
    print("等待 MCU 擦除扇区...")
    while True:
        line = ser.readline().decode(errors="replace").strip()
        if not line:
            continue
        print(f"  MCU: {line}")
        if "READY" in line:
            break
        if "ERROR" in line:
            print("MCU 报错, 中止")
            ser.close()
            return

    # 5) 分页发送
    page_size = 256
    total = len(pcm)
    offset = 0

    print(f"开始烧录 {total} 字节...")
    t0 = time.time()

    while offset < total:
        chunk = pcm[offset : offset + page_size]
        ser.write(chunk)

        # 等待 ACK
        ack = ser.read(1)
        if ack != b"K":
            print(f"\n错误: 偏移 {offset} 处未收到 ACK (收到: {ack!r})")
            ser.close()
            return

        offset += len(chunk)
        pct = offset * 100 // total
        print(f"\r  进度: {offset}/{total} ({pct}%)", end="", flush=True)

    elapsed = time.time() - t0
    print(f"\n烧录完成! 用时 {elapsed:.1f} 秒")

    # 6) 等待 DONE
    line = ser.readline().decode(errors="replace").strip()
    # skip empty lines
    while not line:
        line = ser.readline().decode(errors="replace").strip()
    print(f"  MCU: {line}")
    line = ser.readline().decode(errors="replace").strip()
    if line:
        print(f"  MCU: {line}")

    ser.close()
    print("串口已关闭")


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("用法: python burn_audio.py <串口号> <WAV文件>")
        print("示例: python burn_audio.py COM3 prompt.wav")
        sys.exit(1)

    burn(sys.argv[1], sys.argv[2])
