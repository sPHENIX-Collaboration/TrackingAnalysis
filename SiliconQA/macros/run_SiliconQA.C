#include <SiliconQA.h>

R__LOAD_LIBRARY(libSiliconQA.so)

void run_SiliconQA()
{
    std::string qahtmlpath = "/sphenix/data/data02/sphnxpro/QAhtml/aggregated";
    std::string mdfile = "README.md";
    std::string datfile = "goodruns_silicon.dat";

    SiliconQA *siliconqa = new SiliconQA();
    
    // for run selection: 51730-52206, 52469-53880
    // must be set before GetQAhtml()
    siliconqa->GoldenRuns();

    siliconqa->SetQAhtmlPath(qahtmlpath);
    siliconqa->GetQAhtml();

    siliconqa->doQA();

    siliconqa->SetMarkdownFilename(mdfile);
    siliconqa->WriteMarkdown();

    siliconqa->SetDatFilename(datfile);
    siliconqa->WriteDatFiles();

}