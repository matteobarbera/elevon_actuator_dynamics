from elevon_freq_tools import *


if __name__ == "__main__":
    deflection_peaks("full_amplitude_new_servo.CSV", context="talk")
    deflection_peaks("half_amplitude_new_servo.CSV", context="talk")
    plt.show()
