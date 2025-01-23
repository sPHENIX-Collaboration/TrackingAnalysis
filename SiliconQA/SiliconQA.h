#ifndef __SILICON_QA__
#define __SILICON_QA__

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <tuple>
#include <optional>
#include <unordered_set>

class TH1F;
class TH2;
class TFile;
class TGraphErrors;

class SiliconQA
{
public:
    SiliconQA();
    ~SiliconQA() = default;

    void GetQAhtml();
    void doQA();

    void SetQAhtmlPath(std::string qahtmlpath = "/sphenix/data/data02/sphnxpro/QAhtml/aggregated"){_inputbasedir = qahtmlpath;};
    void SetMarkdownFilename(std::string markdownfile = "README.md"){_markdownfilename = markdownfile;};
    void SetDatFilename(std::string datfile = "goodruns_silicon.dat")
    {
        _datfilename = datfile;
        size_t pos = datfile.find_last_of('.');
        if (pos != std::string::npos) 
        {
            _datrankedfilename = datfile.substr(0, pos) + "_ranked" + datfile.substr(pos);
        } else 
        {
            _datrankedfilename = datfile + "_ranked";
        }
    };

    void WriteMarkdown();
    void WriteDatFiles();

    void GoldenRuns(){b_goldenruns = true;};

private:

    float rawHitAcceptance(TH2 *h2);
    std::tuple<float, float, float, float> fitClusPhi(TH1F *hm);
    void getLTSRegStdDev(TGraphErrors *graph, float fit, float &stddev);
    std::optional<std::tuple<bool, float, float>> inttQA(TFile* qafile);
    std::optional<std::tuple<float, float, float, float, float, float, float>> mvtxQA(TFile* qafile_hit, TFile *qafile_clust);

    std::unordered_set<int> processed_runs;
    std::map<int, std::tuple<bool, float, float>> map_inttQA;
    std::map<int, std::tuple<float, float, float, float, float, float, float>> map_mvtxQA;
    std::map<int, std::tuple<float, float>> map_goodsiliconruns;
    std::map<std::string, std::vector<int>> map_allsiliconruns_categories;

    std::map<int, std::string> map_inputfile_hit;
    std::map<int, std::string> map_inputfile_clust;

    std::string _inputbasedir = "/sphenix/data/data02/sphnxpro/QAhtml/aggregated";
    std::string _markdownfilename = "README.md";
    std::string _datfilename = "goodruns_silicon.dat";
    std::string _datrankedfilename = "goodruns_silicon_ranked.dat";

    int runnumber;
    bool b_goldenruns = false;

    // *********** INTT HIT QA THRESHOLDS *********** //
    float ACCEPTANCE_THRESHOLD = 0.875;
    float ACCEPTANCE_RMS_THRESHOLD = 0.8125;
    // ********************************************** //

    // *********** INTT BCO DIFF *********** //
    int BCO_DIFF_VALUE = 23;
    // ************************************* //

    // *********** MVTX CLUSTER QA THRESHOLDS *********** //
    float scaledchi2ndf_good = 0.0015;
    float scaledchi2ndf_bad = 0.01;
    float scaledchi2ndf_good_b49961 = 0.0015;
    float scaledchi2ndf_bad_b49961 = 0.01;
    float scaledchi2ndf_good_a49961 = 0.015;
    float scaledchi2ndf_bad_a49961 = 0.025;
    float avgclus_threshold = -1E9; // will be set later
    float bovera_high = 0.28;
    float bovera_low = 0.24;
    float bovera_high_b49961 = 0.28;
    float bovera_low_b49961 = 0.24;
    float bovera_high_a49961 = 0.1;
    float bovera_low_a49961 = 0.05;
    // ************************************************** //

    // *********** MVTX HIT QA THRESHOLDS *********** //
    float layer0hitthresh = 0.95;
    float layer1hitthresh = 0.95;
    float layer2hitthresh = 0.95;
    // ********************************************** //

};

#endif