# Visually checking the clustering of the seed:

If you go into the `trackreco/PHCASeeding.h` module and un-comment the line for `#define _PHCASEEDING_CLUSTERLOG_TUPOUT_` (and separately `#define _PHCASEEDING_CLUSTERLOG_TUPOUT_)` and compile the PHCASeeding module will write a number of THNtuples into an output file. One way to read these is to use a Jupyter notebook with some code included and below.

Doing this looks a bit like:

1. Run the code over a few events.
   - e.g.: Run `Fun4All_Seeding.C` included here
2. Copy the output files named `CSV_tuple.root` and `_CLUSTER_LOG_TUPOUT.root` to a local `input` directory where you can run a Jupyter notebook. Add a "tag" to the front of these files (note that a blank tag is totally fine -- it just helps to be able to differentiate between different sets of input files).
    e.g.: 
  `./input/TAGCSV_tuple.root`
  `./input/TAG_CLUSTER_LOG_TUPOUT.root`
3. Copy ./TupScatterer.py local directory.
4. Run a local Jupyter notebook with code similar to:
```python
from TupScatter import TupScatter
from TupScatter import plot_prog_with_z
import numpy as np
import matplotlib.pyplot as plt

file_prefix='TAG'
for event in range(5):
    tupscat = TupScatter(seederfile=f'input/{file_prefix}_CLUSTER_LOG_TUPOUT.root', tracktupfile=f"input/{file_prefix}CSV_tuple.root", event=event)

    fig, ax = plot_prog_with_z(tupscat, (-100,100), (-100,100), plot_cos_angle=False,line_bilinks=False, size=17, print_zs=False, print_grow_seed=False, plot_seeds=True,plot_grown_seeds=True,plot_track=True)

    options = tupscat.get_tuple('search_windows',which_file='seeder')
    opt_str = f'file({file_prefix})  '
    for key, val in options.items():
        opt_str += '{}: {:.5g}, '.format(key, val[0])
    print(opt_str)
    plt.title(opt_str, fontsize=8)
```
