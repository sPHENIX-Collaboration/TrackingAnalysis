#include </sphenix/user/kgable/TrackingAnalysis/SiliconQA/SiliconQA.h>

//R__LOAD_LIBRARY(libSiliconQA.so)
R__LOAD_LIBRARY(/sphenix/user/kgable/TrackingAnalysis/SiliconQA/build/.libs/libSiliconQA.so)

void run_SiliconQA()
{
    std::string qahtmlpath = "/sphenix/data/data02/sphnxpro/QAhtml/aggregated";
    std::string mdfile = "README.md";
    std::string datfile = "goodruns_silicon.dat";

    SiliconQA *siliconqa = new SiliconQA();
    

    siliconqa->SetQAhtmlPath(qahtmlpath);

    siliconqa->SetProductionTag("ana561_2025p013_v001");


    siliconqa->GetQAhtml();

    siliconqa->doQA();

    siliconqa->SetMarkdownFilename(mdfile);
    siliconqa->WriteMarkdown();

    siliconqa->SetDatFilename(datfile);
    siliconqa->WriteDatFiles();

}
