import serial
import numpy as np
import wave

# ===== 参数（必须和 STM32 一致）=====
PORT = 'COM3'
BAUD = 460800
SAMPLE_RATE = 16000
CHANNELS = 1
DURATION = 10  # 秒
BYTES_PER_SAMPLE = 2

TOTAL_SAMPLES = SAMPLE_RATE * DURATION
TOTAL_BYTES = TOTAL_SAMPLES * BYTES_PER_SAMPLE

ser = serial.Serial(PORT, BAUD, timeout=1)

buffer = bytearray()

print("Recording...")

while len(buffer) < TOTAL_BYTES:
    data = ser.read(TOTAL_BYTES - len(buffer))
    if data:
        buffer.extend(data)

ser.close()
print("Done.")

# ===== 转成 int16 =====
audio = np.frombuffer(buffer, dtype=np.int16)

# ===== 写 WAV =====
with wave.open("output.wav", "wb") as wf:
    wf.setnchannels(CHANNELS)
    wf.setsampwidth(2)   # 16-bit
    wf.setframerate(SAMPLE_RATE)
    wf.writeframes(audio.tobytes())

print("Saved output.wav")
