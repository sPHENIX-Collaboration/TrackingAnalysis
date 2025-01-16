### Cosmics track reconstruction

This is a setup to run cosmics tracking over raw data prdfs. It runs the event combining, hit unpacking + clustering, and track seeding and dumps the results into trees. You can set it up like the following

1. Run `make_list.sh` with the data type and run number, e.g.
```
sh make_list.sh cosmics 55073
```
2. That will populate all the list files needed with prdf paths. Then just run the Fun4All_Cosmics.C macro, which is already setup to read from those file lists