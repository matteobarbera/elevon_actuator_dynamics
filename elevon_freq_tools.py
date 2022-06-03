import sys

import numpy as np
from matplotlib import pyplot as plt
from scipy.signal import find_peaks, butter, filtfilt
import seaborn as sns
from functools import wraps


def prettify_plot():

    def decorator(func):

        @wraps(func)
        def wrapper(*args, **kwargs):
            if "context" in kwargs.keys():
                _context = kwargs["context"]
                del kwargs["context"]
            else:
                _context = "notebook"

            if "style" in kwargs.keys():
                _style = kwargs["style"]
                del kwargs["style"]
            else:
                _style = "whitegrid"

            if "params" in kwargs.keys():
                _params = kwargs["params"]
                del kwargs["params"]
            else:
                _params = None

            _default_params = {
              # "xtick.bottom": True,
              # "ytick.left": True,
              # "xtick.color": ".8",  # light gray
              # "ytick.color": ".15",  # dark gray
              "axes.spines.left": False,
              "axes.spines.bottom": False,
              "axes.spines.right": False,
              "axes.spines.top": False,
              }
            if _params is not None:
                merged_params = {**_params, **_default_params}
            else:
                merged_params = _default_params
            with sns.plotting_context(context=_context), sns.axes_style(style=_style, rc=merged_params):
                func(*args, **kwargs)
        return wrapper

    return decorator


def extract_data(filename: str):
    data = np.genfromtxt(filename, delimiter=",", skip_header=1)
    if sys.argv[0].rsplit("/")[-1] == "main.py":
        data = data[:3500, :]  # After this timestamp overflow
    else:
        data = data[:, :]
    dt = np.diff(data[:, 0])
    data_dict = {"dt": dt,
                 "timestamp": data[:, 0],
                 "PWM_motor": data[:, 1],
                 "PWM_elevon": data[:, 2],
                 "rates": data[:, 3:],
                 }
    data_dict["rate_x"] = data_dict["rates"][:, 0]
    data_dict["rate_y"] = data_dict["rates"][:, 1]
    data_dict["rate_z"] = data_dict["rates"][:, 2]
    return data_dict


@prettify_plot()
def plot_rates(filename):
    data = extract_data(filename)

    plt.figure()
    plt.plot(data["timestamp"] * 1e-3, data["rate_x"], label="x")
    plt.plot(data["timestamp"] * 1e-3, data["rate_y"], label="y")
    plt.plot(data["timestamp"] * 1e-3, data["rate_z"], label="z")
    plt.legend()
    plt.xlabel("Time [s]")
    plt.ylabel("Rates [deg/s]")


def plot_deflection(filename, make_plot=True):
    data = extract_data(filename)
    angle = 0
    angle_list = []
    for dr, dt in zip(data["rate_x"], data["dt"] * 1e-3):
        angle_list.append(angle)
        angle += dr * dt

    if make_plot:
        plt.figure()
        plt.plot(data["timestamp"][:-1], angle_list)
    return np.asarray(angle_list)


def plot_signal(filename):
    data = extract_data(filename)
    plt.figure()
    plt.plot(data["timestamp"][1:] * 1e-3, data["PWM_elevon"][1:])
    plt.xlabel("Time [s]")
    plt.ylabel("PWM [micro s]")


def time_to_frequency(t_array):
    if sys.argv[0].rsplit("/")[-1] == "main.py":
        min_f = 0.5
        max_f = 10
        t1 = 2000
        t_ramp = 30000
    else:
        min_f = 0.6
        max_f = 10
        t1 = 3333
        t_ramp = 60000
    f = []
    for t in t_array:
        if t < t1:
            f.append(min_f)
        elif t < t1 + t_ramp:
            f.append(min_f + ((max_f - min_f) / t_ramp) * (t - t1))
        else:
            f.append(max_f)
    return np.asarray(f)


@prettify_plot()
def deflection_peaks(filename):
    data = extract_data(filename)
    defl_angle = plot_deflection(filename, make_plot=False)
    peaks, _ = find_peaks(defl_angle, prominence=10, distance=5)
    # plt.figure()
    # plt.scatter(data["timestamp"][:-1][peaks] * 1e-3, defl_angle[peaks])
    # plt.plot(data["timestamp"][:-1], -defl_angle)
    # plt.plot(data["timestamp"][:-1], line, color="k")
    # plt.show()
    if filename == "zero_max_chirp.CSV":
        troughs, _ = find_peaks(defl_angle * -1, height=-20, prominence=10, distance=5)
    elif filename == "half_amplitude_new_servo.CSV":
        troughs, _ = find_peaks(defl_angle*-1, prominence=10, distance=5)
    elif filename == "full_amplitude_new_servo.CSV":
        troughs, _ = find_peaks(defl_angle * -1, prominence=10, distance=5)
    else:
        troughs, _ = find_peaks(defl_angle * -1, height=0, prominence=10, distance=5)
    print(len(peaks), len(troughs))

    # plt.figure()
    # plt.plot(data["timestamp"][:-1] * 1e-3, defl_angle, alpha=0.9, label="Deflection")
    # plt.scatter(data["timestamp"][:-1][peaks] * 1e-3, defl_angle[peaks], label="Max deflection achieved", color="C1")
    # plt.scatter(data["timestamp"][:-1][troughs] * 1e-3, defl_angle[troughs], label="Min deflection achieved", color="r")
    # plt.xlabel("Time [s]")
    # plt.ylabel("Angle [\u00B0]")
    # plt.legend()

    if len(peaks) != len(troughs):
        peaks = peaks[:-1]
    delta_angle = defl_angle[peaks] - defl_angle[troughs]

    frequency = time_to_frequency(data["timestamp"][:-1])
    # plt.figure()
    # plt.plot(data["timestamp"][:-1] * 1e-3, frequency)
    # plt.xlabel("Time [s]")
    # plt.ylabel("Input frequency [Hz]")
    nyq = 0.5 * (1 / data["dt"][0] * 1e-3)
    if sys.argv[0].rsplit("/")[-1] == "main.py":
        cutoff = 0.000005 / nyq
    else:
        cutoff = 0.000017 / nyq  
    b, a = butter(4, cutoff, btype="low", analog=False, output="ba")
    approx = filtfilt(b, a, delta_angle, method="gust", irlen=100)

    plt.figure()
    plt.plot(frequency[peaks], delta_angle, alpha=0.7)
    plt.plot(frequency[peaks], approx, color="r")
    plt.ylabel("Deflection achieved [\u00B0]")
    plt.xlabel("Input frequency [Hz]")
    # plt.title("Elevon response to chirp")


if __name__ == "__main__":
    pass
