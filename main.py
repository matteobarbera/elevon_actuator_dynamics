from elevon_freq_tools import *

if __name__ == "__main__":
    min_max_test = "min_max_chirp.CSV"
    zero_max_test = "zero_max_chirp.CSV"
    # plot_rates(min_max_test, context="talk")
    # plot_rates(zero_max_test, context="talk")
    # plot_deflection(min_max_test)
    # plot_deflection(zero_max_test)
    # plot_signal(min_max_test)
    deflection_peaks(min_max_test, context="poster")
    deflection_peaks(zero_max_test, context="poster")
    plt.show()
