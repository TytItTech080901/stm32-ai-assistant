import wave
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.font_manager import FontProperties
import os


class AudioAnalyzer:
    def __init__(self, wav_file_path):
        """
        初始化音频分析器
        
        参数:
            wav_file_path: WAV文件的路径
        """
        self.file_path = wav_file_path
        self.sample_rate = None
        self.audio_data = None
        self.duration = None
        self.channels = None
        self.sample_width = None
        
        # 加载音频文件
        self._load_audio()
    
    def _load_audio(self):
        """加载WAV音频文件"""
        try:
            with wave.open(self.file_path, 'rb') as wav_file:
                # 获取音频参数
                self.channels = wav_file.getnchannels()
                self.sample_width = wav_file.getsampwidth()
                self.sample_rate = wav_file.getframerate()
                n_frames = wav_file.getnframes()
                
                # 读取音频数据
                audio_bytes = wav_file.readframes(n_frames)
                
                # 根据采样宽度选择数据类型
                if self.sample_width == 1:
                    dtype = np.uint8
                elif self.sample_width == 2:
                    dtype = np.int16
                elif self.sample_width == 4:
                    dtype = np.int32
                else:
                    raise ValueError(f"不支持的采样宽度: {self.sample_width}")
                
                # 转换为numpy数组
                self.audio_data = np.frombuffer(audio_bytes, dtype=dtype)
                
                # 如果是立体声，重塑数组
                if self.channels == 2:
                    self.audio_data = self.audio_data.reshape(-1, 2)
                
                # 计算时长
                self.duration = n_frames / self.sample_rate
                
            print(f"成功加载音频文件: {os.path.basename(self.file_path)}")
            
        except Exception as e:
            print(f"加载音频文件时出错: {e}")
            raise
    
    def calculate_parameters(self):
        """
        计算音频参数
        
        返回:
            包含各种音频参数的字典
        """
        # 获取单声道数据用于分析
        if self.channels == 2:
            mono_data = np.mean(self.audio_data, axis=1)
        else:
            mono_data = self.audio_data
        
        # 归一化到 [-1, 1] 范围
        max_val = 2 ** (8 * self.sample_width - 1)
        normalized_data = mono_data.astype(np.float64) / max_val
        
        # 计算RMS (Root Mean Square) - 均方根
        rms = np.sqrt(np.mean(normalized_data ** 2))
        
        # 计算峰值 (Peak)
        peak = np.max(np.abs(normalized_data))
        
        # 计算峰值因数 (Crest Factor)
        crest_factor = peak / rms if rms > 0 else 0
        
        # 计算dB增益 (相对于满刻度)
        # dBFS (dB relative to Full Scale)
        rms_db = 20 * np.log10(rms) if rms > 0 else -np.inf
        peak_db = 20 * np.log10(peak) if peak > 0 else -np.inf
        
        # 计算响度 (LUFS的简化版本 - 使用RMS作为近似)
        # 真正的LUFS需要复杂的滤波，这里使用简化版本
        loudness_lufs = rms_db - 0.691  # K权重近似
        
        # 计算动态范围
        dynamic_range = peak_db - rms_db
        
        # 计算零交叉率 (Zero Crossing Rate)
        zero_crossings = np.sum(np.abs(np.diff(np.sign(normalized_data)))) / 2
        zcr = zero_crossings / len(normalized_data)
        
        # 计算能量
        energy = np.sum(normalized_data ** 2)
        
        # 计算平均振幅
        mean_amplitude = np.mean(np.abs(normalized_data))
        
        parameters = {
            '文件名': os.path.basename(self.file_path),
            '时长(秒)': round(self.duration, 3),
            '采样率(Hz)': self.sample_rate,
            '声道数': self.channels,
            '位深度(bit)': self.sample_width * 8,
            '采样点数': len(mono_data),
            'RMS(均方根)': round(rms, 6),
            'RMS(dB)': round(rms_db, 2),
            '峰值': round(peak, 6),
            '峰值(dB)': round(peak_db, 2),
            '峰值因数': round(crest_factor, 2),
            '响度(LUFS近似)': round(loudness_lufs, 2),
            '动态范围(dB)': round(dynamic_range, 2),
            '零交叉率': round(zcr, 6),
            '总能量': round(energy, 2),
            '平均振幅': round(mean_amplitude, 6)
        }
        
        return parameters
    
    def plot_waveform(self, save_path=None, show=True):
        """
        绘制音频波形图
        
        参数:
            save_path: 保存图像的路径(可选)
            show: 是否显示图像
        """
        # 创建时间轴
        if self.channels == 2:
            n_samples = self.audio_data.shape[0]
        else:
            n_samples = len(self.audio_data)
        
        time_axis = np.linspace(0, self.duration, n_samples)
        
        # 创建图形
        if self.channels == 2:
            fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 8))
            
            # 左声道
            ax1.plot(time_axis, self.audio_data[:, 0], linewidth=0.5, color='blue')
            ax1.set_title('Left Channel Waveform', fontsize=14, fontweight='bold')
            ax1.set_ylabel('Amplitude', fontsize=12)
            ax1.grid(True, alpha=0.3)
            ax1.set_xlim(0, self.duration)
            
            # 右声道
            ax2.plot(time_axis, self.audio_data[:, 1], linewidth=0.5, color='red')
            ax2.set_title('Right Channel Waveform', fontsize=14, fontweight='bold')
            ax2.set_xlabel('Time (seconds)', fontsize=12)
            ax2.set_ylabel('Amplitude', fontsize=12)
            ax2.grid(True, alpha=0.3)
            ax2.set_xlim(0, self.duration)
            
        else:
            fig, ax = plt.subplots(figsize=(12, 6))
            ax.plot(time_axis, self.audio_data, linewidth=0.5, color='blue')
            ax.set_title(f'Audio Waveform - {os.path.basename(self.file_path)}', 
                        fontsize=14, fontweight='bold')
            ax.set_xlabel('Time (seconds)', fontsize=12)
            ax.set_ylabel('Amplitude', fontsize=12)
            ax.grid(True, alpha=0.3)
            ax.set_xlim(0, self.duration)
        
        plt.tight_layout()
        
        # 保存图像
        if save_path:
            plt.savefig(save_path, dpi=300, bbox_inches='tight')
            print(f"波形图已保存至: {save_path}")
        
        # 显示图像
        if show:
            plt.show()
        else:
            plt.close()
    
    def plot_spectrum(self, save_path=None, show=True):
        """
        绘制频谱图
        
        参数:
            save_path: 保存图像的路径(可选)
            show: 是否显示图像
        """
        # 获取单声道数据
        if self.channels == 2:
            mono_data = np.mean(self.audio_data, axis=1)
        else:
            mono_data = self.audio_data
        
        # 归一化
        max_val = 2 ** (8 * self.sample_width - 1)
        normalized_data = mono_data.astype(np.float64) / max_val
        
        # 计算FFT
        n = len(normalized_data)
        fft = np.fft.fft(normalized_data)
        freq = np.fft.fftfreq(n, 1/self.sample_rate)
        
        # 只取正频率部分
        positive_freq_idx = freq >= 0
        freq = freq[positive_freq_idx]
        magnitude = np.abs(fft[positive_freq_idx])
        
        # 转换为dB
        magnitude_db = 20 * np.log10(magnitude + 1e-10)
        
        # 绘图
        fig, ax = plt.subplots(figsize=(12, 6))
        ax.plot(freq, magnitude_db, linewidth=0.5, color='purple')
        ax.set_title(f'Frequency Spectrum - {os.path.basename(self.file_path)}', 
                    fontsize=14, fontweight='bold')
        ax.set_xlabel('Frequency (Hz)', fontsize=12)
        ax.set_ylabel('Magnitude (dB)', fontsize=12)
        ax.grid(True, alpha=0.3)
        ax.set_xlim(0, self.sample_rate / 2)
        
        plt.tight_layout()
        
        # 保存图像
        if save_path:
            plt.savefig(save_path, dpi=300, bbox_inches='tight')
            print(f"频谱图已保存至: {save_path}")
        
        # 显示图像
        if show:
            plt.show()
        else:
            plt.close()
    
    def print_parameters(self):
        """打印音频参数"""
        params = self.calculate_parameters()
        
        print("\n" + "="*50)
        print("音频参数分析")
        print("="*50)
        
        for key, value in params.items():
            print(f"{key:20s}: {value}")
        
        print("="*50 + "\n")
    
    def analyze(self, save_waveform=None, save_spectrum=None, show_plots=True):
        """
        完整分析音频文件
        
        参数:
            save_waveform: 保存波形图的路径(可选)
            save_spectrum: 保存频谱图的路径(可选)
            show_plots: 是否显示图像
        """
        # 打印参数
        self.print_parameters()
        
        # 绘制波形图
        self.plot_waveform(save_path=save_waveform, show=show_plots)
        
        # 绘制频谱图
        self.plot_spectrum(save_path=save_spectrum, show=show_plots)
        
        return self.calculate_parameters()


def main():
    """主函数示例"""
    # 示例用法
    wav_file = "output.wav"  # 替换为你的WAV文件路径
    
    if not os.path.exists(wav_file):
        print(f"错误: 找不到文件 '{wav_file}'")
        print("\n请修改main()函数中的wav_file变量为你的WAV文件路径")
        return
    
    try:
        # 创建分析器实例
        analyzer = AudioAnalyzer(wav_file)
        
        # 执行完整分析
        params = analyzer.analyze(
            save_waveform="tools\\analyzer_output\\waveform.png",
            save_spectrum="tools\\analyzer_output\\spectrum.png",
            show_plots=True
        )
        
        # 也可以单独调用各个功能:
        # analyzer.print_parameters()
        # analyzer.plot_waveform()
        # analyzer.plot_spectrum()
        # params = analyzer.calculate_parameters()
        
    except Exception as e:
        print(f"分析过程中出错: {e}")


if __name__ == "__main__":
    main()
