"""End-to-end direct-input tests use a small fake ROOT reader, not real PyROOT."""

import contextlib
import io
import json
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

import numpy as np
from export_tpcqa import discover

FAKE_ROOT = """
import numpy as np
from pathlib import Path
class Settings:
    def SetBatch(self,value): pass
    def AddDirectory(self,value): pass
gROOT=Settings()
TH1=Settings()
class Axis:
    def __init__(self,edges,title):self.edges=np.array(edges);self.title=title
    def GetNbins(self):return len(self.edges)-1
    def GetBinLowEdge(self,i):return self.edges[i-1]
    def GetTitle(self):return self.title
class Histogram:
    def __init__(self,name,scale,bad=False):
        phi=Axis(np.linspace(-np.pi,np.pi,7),'phi')
        eta=Axis(np.linspace(-1.2,1.2,5),'eta')
        pt=Axis([0,.2,.5,.7,1,1.5,2,3,4],'pT')
        dca=Axis(np.linspace(-1,1,11),'DCAxy [cm]')
        self.axes=[phi,eta,pt] if name.endswith('pt_phi_eta') else [phi if '_phi_' in name else eta,pt,dca]
        if bad and not name.endswith('pt_phi_eta'):self.axes[2]=Axis(np.linspace(-2,2,self.axes[2].GetNbins()+1),'changed axis')
        self.data=np.full(tuple(a.GetNbins()+2 for a in self.axes[::-1]),scale,dtype=np.float32).ravel()
    def ClassName(self):return 'TH3F'
    def GetNbinsX(self):return self.axes[0].GetNbins()
    def GetNbinsY(self):return self.axes[1].GetNbins()
    def GetNbinsZ(self):return self.axes[2].GetNbins()
    def GetNcells(self):return self.data.size
    def GetArray(self):return self.data
    def GetSumw2N(self):return 0
    def GetXaxis(self):return self.axes[0]
    def GetYaxis(self):return self.axes[1]
    def GetZaxis(self):return self.axes[2]
    def GetTitle(self):return 'TPC-only QA; unchanged producer selection'
class File:
    def __init__(self,path):self.path=Path(path)
    def IsZombie(self):return False
    def TestBit(self,bit):return False
    def Get(self,name):
        if not name.startswith('h_TpcSeedsQA_'): return None
        return Histogram(name,int(self.path.stem.rsplit('-',1)[-1])+1,self.path.read_text()=='bad')
    def Close(self):pass
class TFile:
    kRecovered=1
    @staticmethod
    def Open(path,mode):return File(path)
"""


FAKE_CDB = r"""
import csv
class System:
    def Load(self,name): return 0
class Interpreter:
    def Declare(self,text): return True
gSystem=System();gInterpreter=Interpreter()
_flags={}
class Consts:
    def set_IntFlag(self,name,value):_flags[name]=value
    def set_uint64Flag(self,name,value):_flags[name]=value
    def set_StringFlag(self,name,value):_flags[name]=value
    @staticmethod
    def instance():return Consts()
recoConsts=Consts
class CDB:
    @staticmethod
    def instance():return CDB()
    def getUrl(self,name):return name+'.root'
CDBInterface=CDB
class CDBTTree:
    def __init__(self,path):pass
    def LoadCalibrations(self):pass
    def GetUInt64EntryMap(self):return {1:{'gbco':100},2:{'gbco':200}}
    def GetSingleFloatEntryMap(self):return {'Fkeffside0':_flags['RUNNUMBER']/100000,'Fkeffside1':1.2}
    def GetFloatValue(self,channel,name):return float(channel)
class TpcConditionsReference:
    def set_cdb_tag(self,tag):self.tag=tag
    def set_reference_field(self,value):self.field=value
    def export_phgarfield(self,run,group,path):
        rows=[dict(group=group,run=run,side=side,velocity_cm_us=7.4,t0_bins=8,
                   source='synthetic PHGarfield test',z_pad_cm=102.5,cm_halfwidth_cm=.175,
                   cdb_tag=self.tag,reference_field_Vcm=self.field) for side in (0,1)]
        with open(path,'w',newline='') as stream:
            w=csv.DictWriter(stream,fieldnames=list(rows[0]));w.writeheader();w.writerows(rows)
"""


class DirectTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.base = Path(self.temp.name)
        (self.base / "ROOT.py").write_text(FAKE_ROOT)
        self.sources = []
        for system, run in [("run3pp", 81558), ("run3oo", 82688)]:
            source = self.base / system / "physics/ana585_2026p006_v001/DST_TRKR_SEED"
            self.sources.append(f"{system}={source}")
            for segment in [7, 2, 0]:
                self.fixture(source, system, run, segment)

    def fixture(self, source, system, run, segment, text=""):
        folder = source / str(run) / "hist"
        folder.mkdir(parents=True, exist_ok=True)
        path = (
            folder
            / f"HIST_DST_TRKR_SEED_{system}_ana585_2026p006_v001-{run:08d}-{segment:05d}.root"
        )
        path.write_text(text)
        return path

    def command(self, out, *extra):
        args = [sys.executable, str(Path(__file__).with_name("export_tpcqa.py"))]
        for source in self.sources:
            args += ["--source", source]
        args += ["--all-runs", "--max-files", "2", "--jobs", "2", "--out", str(out), *extra]
        result = subprocess.run(
            args, capture_output=True, text=True, env=dict(os.environ, PYTHONPATH=str(self.base))
        )
        return result

    def test_direct_two_system_sum_cap_and_resume(self):
        out = self.base / "output"
        result = self.command(out)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        manifest = json.loads((out / "manifest.json").read_text())
        self.assertEqual(manifest["input_mode"], "production_qa_histograms")
        self.assertEqual(len(manifest["runs"]), 2)
        for run in manifest["runs"]:
            self.assertEqual(run["source_segments"], 2)
            self.assertEqual([f["segment"] for f in run["source_files"]], [0, 2])
            with np.load(out / run["file"]) as arrays:
                m = next(m for m in run["maps"] if m["axis"] == "phi" and m["window"] == "pt_0p5_1")
                np.testing.assert_array_equal(arrays[m["key"] + "_counts"], np.full((2, 10), 24.0))
                np.testing.assert_array_equal(
                    arrays[m["key"] + "_variance"], np.full((2, 10), 24.0)
                )
            charge = next(c for c in run["charge_counts"] if c["window"] == "pt_0p5_1")
            self.assertEqual(charge["count"], 192.0)
        result = self.command(out, "--resume")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(result.stdout.count("reused verified direct-QA export"), 2)

    def test_one_go_cdb_and_phgarfield_snapshot_in_fresh_processes(self):
        import csv

        (self.base / "ROOT.py").write_text(FAKE_ROOT + FAKE_CDB)
        out = self.base / "complete"
        result = self.command(out, "--cdb-tag", "test_tag", "--reference-field", "375")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        manifest = json.loads((out / "manifest.json").read_text())
        self.assertEqual(manifest["conditions"]["status"], "exported")
        self.assertEqual(manifest["conditions"]["incomplete_sides"], 0)
        with (out / "conditions_snapshot.csv").open() as stream:
            rows = list(csv.DictReader(stream))
        self.assertEqual(len(rows), 4)
        for row in rows:
            self.assertEqual(row["keff_applied"], "")
            self.assertEqual(row["cdb_tag"], "test_tag")
            if row["side"] == "south":
                self.assertAlmostEqual(float(row["keff_base"]), int(row["run"]) / 100000)
        self.assertTrue((out / "phgarfield_reference.csv").is_file())
        self.assertTrue((out / "charge_balance.csv").is_file())
        result = self.command(
            out, "--resume", "--cdb-tag", "another_tag", "--reference-field", "375"
        )
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(result.stdout.count("reused verified direct-QA export"), 2)
        with (out / "conditions_snapshot.csv").open() as stream:
            rows = list(csv.DictReader(stream))
        self.assertEqual({r["cdb_tag"] for r in rows}, {"another_tag"})

    def test_oo_missing_cdb_uses_explicit_igem_reference(self):
        import csv

        fake = FAKE_CDB.replace(
            "return name+'.root'",
            "return '' if name=='Tpc_PolyClusterizer_kEff' and _flags['RUNNUMBER']==82688 else name+'.root'",
        )
        (self.base / "ROOT.py").write_text(FAKE_ROOT + fake)
        out = self.base / "oo_igem"
        result = self.command(out, "--cdb-tag", "test", "--keff-fallback", "igem")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        with (out / "conditions_snapshot.csv").open() as stream:
            rows = list(csv.DictReader(stream))
        for row in rows:
            if row["group"].startswith("run3oo"):
                self.assertEqual(row["keff_cdb"], "")
                self.assertEqual(row["keff_applied"], "")
                self.assertEqual(row["keff_kind"], "derived_igem_not_verified_applied")
                self.assertGreater(float(row["keff_reference"]), 0)
            else:
                self.assertEqual(row["keff_kind"], "cdb_base_not_verified_applied")
        manifest = json.loads((out / "manifest.json").read_text())
        self.assertEqual(manifest["conditions"]["derived_igem_sides"], 2)

    def test_schema_mismatch_never_publishes_success(self):
        source = Path(self.sources[0].split("=", 1)[1])
        self.fixture(source, "run3pp", 81558, 2, "bad")
        out = self.base / "bad_output"
        result = self.command(out)
        self.assertNotEqual(result.returncode, 0)
        self.assertFalse((out / "SUCCESS").exists())
        manifest = json.loads((out / "manifest.json").read_text())
        self.assertEqual(len(manifest["failed"]), 1)
        self.assertIn("Mixed QA", manifest["failed"][0]["error"])

    def test_historical_range_directory_and_requested_run(self):
        source = Path(self.sources[0].split("=", 1)[1])
        (source / "81558").rename(source / "run_00081500_00081600")
        with contextlib.redirect_stdout(io.StringIO()):
            records, sources = discover(self.sources[:1], [("run3pp", 81558)], False, 1)
        self.assertEqual(len(records), 1)
        self.assertEqual(records[0]["files"][0]["segment"], 0)
        with self.assertRaisesRegex(ValueError, "Missing or ambiguous"):
            with contextlib.redirect_stdout(io.StringIO()):
                discover(self.sources[:1], [("run3pp", 81559)], False, 1)


if __name__ == "__main__":
    unittest.main()
