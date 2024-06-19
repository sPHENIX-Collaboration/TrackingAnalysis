# this script requires python3

import pandas as pd
import matplotlib.pyplot as plt
import uproot as up
import numpy as np

''' a few simple functions to help scatterplots in the seeder'''
class TupScatter:
    def __init__(self, seederfile='_CLUSTER_LOG_TUPOUT.root', tracktupfile='CSV_tuple.root',event=1):
        self.seederfile = seederfile
        self.tracktupfile = tracktupfile
        self.event=event
    
    def get_tuple(self,  tupname, cols=None, which_file='seeder', event=None, cuts=None):
        # cuts are lists of triplet of [col-name, (min,max)]
        if which_file == 'seeder':
            file = self.seederfile
        elif which_file == 'track':
            file = self.tracktupfile
        else:
            print('Error: which_file must be "seeder" or "track"')
            return None, None
        
        if event is not None:
            self.event = event
        # try:
        tup = up.open(file)[tupname]
        df = pd.DataFrame(tup.arrays(library='pd'))

        try:
            df = df[df.event==self.event]
        except:
            pass

        if not cuts is None:
            cut = np.ones(len(df), dtype=bool)
            for _cut in cuts:
                col = _cut[0]
                minval = _cut[1][0]
                maxval = _cut[1][1]
                cut = cut & (df[col]>minval) & (df[col]<maxval)
            df = df[cut]

        if cols is not None:
            df_cols = df[cols]
        else:
            df_cols = df
        
        return {col:np.array(df_cols[col]) for col in df_cols.columns}

            # return tup[tup.event==event]
        # except:
            # print('Error: could not open tuple')
            # return None

    def add_num(self, ax):
        nseeds = len(self.get_tuple('ntp_track',which_file='track',cols=['seed'])['seed'])
        ntracks = len(self.get_tuple('ntp_track',which_file='track')['pt'])
        nclus_tracks = len(self.get_tuple('ntp_clus_trk',which_file='track',cols=['x'])['x'])
        ax.text(-80, -60, f'Num seeds: {nseeds}', fontsize=8)
        ax.text(-80, -65, f'Num tracks: {ntracks}', fontsize=8)
        ax.text(-80, -70, f'Num clusters in tracks: {nclus_tracks}', fontsize=8)

def plot_tup(tupscat, xlim, ylim, tup='ntp_clus_trk', file='track', size=10, color='green', facecolors=None, label=None,marker='o', alpha=0.8, fig=None, ax=None):
    cuts=(('x',xlim),('y',ylim))
    if fig is None:
        fig, ax = plt.subplots(figsize=(10,7))
    data = tupscat.get_tuple(tup, cols=['x','y','z'],cuts=cuts,which_file=file)

    if facecolors is None:
        facecolors ='none'


    if label:
        ax.scatter(data['x'], data['y'], edgecolors=color, marker=marker,alpha=alpha,s=size, facecolors=facecolors, label=label)
    else:
        ax.scatter(data['x'], data['y'], edgecolors=color, marker=marker,alpha=alpha,facecolors=facecolors, s=size)
    return fig, ax
    

def plot_prog_with_z(tupscat, xlim, ylim, plot_cos_angle=False, fig=None, ax=None,
                       line_bilinks=False, size=20, print_zs=True, print_grow_seed=False,
                       plot_seeds=True, plot_grown_seeds=True, plot_track=False):
    cuts=(('x',xlim),('y',ylim))

    all_clus = tupscat.get_tuple('all', cols=['x','y','z','layer'], cuts=cuts)
    bilinks = tupscat.get_tuple('bilinks', cols=['x','y','z','layer','topbot01'],cuts=cuts)


    # import matplotlib.pyplot as plt
    if fig is None:
        fig, ax = plt.subplots(figsize=(10,7))

    if line_bilinks:
        cos_angle = tupscat.get_tuple('win_cos_angle', 
            cols=['x0','x1','x2','y0','y1','y2','z0','z1','z2','layer0','layer1','layer2','cos_angle'],cuts=(('x1',xlim),('y1',ylim)))
        for i in range(len(cos_angle['x0'])):
            if i==0:
                ax.plot([cos_angle['x1'][i], cos_angle['x2'][i]], [cos_angle['y1'][i], cos_angle['y2'][i]], linestyle='dotted', color='gray', label='triplet mid->bot')
            else:
                ax.plot([cos_angle['x1'][i], cos_angle['x2'][i]], [cos_angle['y1'][i], cos_angle['y2'][i]], linestyle='dotted', color='gray')

    if plot_cos_angle:
        cos_angle = tupscat.get_tuple('win_cos_angle', 
            cols=['x0','x1','x2','y0','y1','y2','z0','z1','z2','layer0','layer1','layer2','cos_angle'],cuts=(('x1',xlim),('y1',ylim)))
        for i in range(len(cos_angle['x0'])):
            if i==0:
                ax.plot([cos_angle['x0'][i], cos_angle['x1'][i]], [cos_angle['y0'][i], cos_angle['y1'][i]], linestyle='solid', color='green', label='triplet top->mid')
            else:
                ax.plot([cos_angle['x0'][i], cos_angle['x1'][i]], [cos_angle['y0'][i], cos_angle['y1'][i]], linestyle='solid', color='green')

        for i in range(len(cos_angle['x1'])):
            if i == 0:
                ax.plot([cos_angle['x1'][i], cos_angle['x2'][i]], [cos_angle['y1'][i], cos_angle['y2'][i]], linestyle='dashed', color='red', label='triplet mid->bot')
            else:
                ax.plot([cos_angle['x1'][i], cos_angle['x2'][i]], [cos_angle['y1'][i], cos_angle['y2'][i]], linestyle='dashed', color='red')
        

        # Select only the rows with cos_angle > -0.95
        _cut1 = cos_angle['cos_angle']>-0.95
        # Draw the selected points as open circles
        _sc_fail = ax.scatter(cos_angle['x1'][_cut1], cos_angle['y1'][_cut1], c=cos_angle['cos_angle'][_cut1], marker='o', cmap='Reds_r', s=130, label='cos_angle > -0.95', vmin=-0.95,vmax=0.)
        
        # Select only the rows with cos_angle < -0.95
        _cut0 = cos_angle['cos_angle']<-0.95
        # Draw the selected points as open circles
        _sc_pass = ax.scatter(cos_angle['x1'][_cut0], cos_angle['y1'][_cut0], c=cos_angle['cos_angle'][_cut0], marker='o', cmap='Greens_r', s=80, label='cos_angle < -0.95', vmin=-1, vmax=-0.95)

        plt.colorbar(_sc_fail, label='cos_angle>-0.95')
        plt.colorbar(_sc_pass, label='cos_angle<-0.95')



    # ax.scatter(all_track['x'], all_track['y'], color='red', s=size+200, marker='s',  alpha=0.1, label='all clusters')
    ax.scatter(all_clus['x'], all_clus['y'], color='black', s=size, label='all clusters')
    ax.scatter(bilinks['x'], bilinks['y'], color='tan', s=size, label='bilink link')
    if (plot_seeds):
        seeds = tupscat.get_tuple('seeds', cols=['x','y','z','layer'],cuts=cuts)
        ax.scatter(seeds['x'], seeds['y'], color='cyan', s=size, label='seeds',marker='+')
    if plot_track:
        tracks = tupscat.get_tuple('ntp_clus_trk', cols=['x','y','z'],which_file='track')
        ax.scatter(tracks['x'], tracks['y'], c='green', marker='+',alpha=.10, s=150, label='tracks')
    if (plot_grown_seeds):
        grown_seeds = tupscat.get_tuple('grown_seeds', cols=['x','y','z','layer'],cuts=cuts)
        ax.scatter(grown_seeds['x'], grown_seeds['y'], color='blue', s=(size+20), label='grown seeds',marker='+')

    if print_grow_seed:
        win_sim = tupscat.get_tuple('search_windows')
        dzdr_lim = win_sim['dzdr_ClAdd'][0]
        dphi2_lim = win_sim['dphidr2_ClAdd'][0]

        print(f'limits: dzdr:{dzdr_lim} dphi2:{dphi2_lim}')

        gs = tupscat.get_tuple('win_seedL1',cols=['xL','yL','zL','layerL','x1','y1','z1','layer1','dzdr_12','dzdr_L1','delta_dzdr_12_L1','d2phidr2_123','d2phidr2_L12','delta_d2phidr2'],cuts=(('xL',xlim),('yL',ylim)))

        first_add = True
        first_fail = True
        for i in range(len(gs['layerL'])):
            xL = gs['xL'][i] - (xlim[1]-xlim[0])/5.6
            yL = gs['yL'][i]

            dzdr = gs['delta_dzdr_12_L1'][i]
            dphidr = gs['delta_d2phidr2'][i]

            if dzdr < dzdr_lim and dphidr < dphi2_lim:
                ax.text(xL, yL, r'$|\Delta\frac{dz}{dR}|$'+f':{dzdr:.2g}'+
                r' $|\Delta\frac{d^2\phi}{dR^2}|$:'+f'{dphidr:.2g}', fontsize=11,color='green')
                if first_add:
                    ax.scatter(gs['xL'][i], gs['yL'][i], color='green', s=size*2, marker='*',label='Clusters added to seed')
                    first_add=False
                else:
                    ax.scatter(gs['xL'][i], gs['yL'][i], color='green', s=size*2, marker='*')
            else:
                ax.text(xL, yL, r'$|\Delta\frac{dz}{dR}|$'+f':{dzdr:.2g}'+
                r' $|\Delta\frac{d^2\phi}{dR^2}|$:'+f'{dphidr:.2g}', fontsize=11,color='red')
                if first_fail:
                    ax.scatter(gs['xL'][i], gs['yL'][i], color='red', s=size*2, marker='*',label='Cluster fails to add (terminate seed)')
                    first_fail=False
                else:
                    ax.scatter(gs['xL'][i], gs['yL'][i], color='red', s=size*2, marker='*')
            
            x1 = gs['x1'][i] + (xlim[1]-xlim[0])/65
            y1 = gs['y1'][i]
            dzdr - gs['dzdr_12'][i]
            dphidr_123 = gs['d2phidr2_123'][i]
            ax.text(x1, y1, r'$\frac{dz}{dR}$'+f':{dzdr:.2g}'+
                    r'$\frac{d^2\phi}{dR^2}$:'+f'{dphidr_123:.2g}', fontsize=11,color='blue')

    ax.set_xlabel('x [cm]')
    ax.set_ylabel('y [cm]')



    if print_zs:
        for i in range(len(all_clus['layer'])):
            # print(i)
            x = all_clus['x'][i]
            y = all_clus['y'][i]
            if (x>xlim[0] and x<xlim[1] and y>ylim[0] and y<ylim[1]):
                L = all_clus['layer'][i]
                z = all_clus['z'][i]
                ax.text(x, y, f'{L:.0f} z:{z:.2f}', fontsize=7)
        # if (x>xlim[0] and x<xlim[1] and y>ylim[0] and y<ylim[1]):
            # ax.text(x, y, f'{all_clus['layer'][i]:.0f} $\eta$:{all_clus['z'][i]:.2f} $\phi$:{np.arctan2(all_clus['x'][i],all_clus['y'][i]):0.2f}', fontsize=11)
# 
        plt.xlim(xlim)
        plt.ylim(ylim)

    # plt.title(r'Center of triplets with $|\Delta\eta|<1.5$ and $|\Delta\phi|<0.2$ (the defaults search is  $|\Delta\phi|<0.05$)')
    plt.legend()
    return fig, ax
