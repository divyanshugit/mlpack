/**
 * @file print_docs.cpp
 * @author Ryan Curtin
 *
 * Implementation of functions to print Markdown from documentation.
 */
#include "print_docs.hpp"

#include <mlpack/core/util/cli.hpp>
#include <mlpack/core/util/program_doc.hpp>
#include "binding_info.hpp"
#include "print_doc_functions.hpp"

// Make sure that this is defined.
#ifndef DOXYGEN_PREFIX
#define DOXYGEN_PREFIX "https://mlpack.org/docs/mlpack-git/doxygen/"
#endif

using namespace std;
using namespace mlpack;
using namespace mlpack::util;
using namespace mlpack::bindings;
using namespace mlpack::bindings::markdown;

// A function to imitate how kramdown will strip non-alphanumeric characters.
std::string StripAnchor(const std::string& input)
{
  std::string s(input);
  s.erase(std::remove_if(s.begin(), s.end(),
      [](char c) -> bool { return !std::isalnum(c); }), s.end());
  return s;
}

void PrintHeaders(const std::string& bindingName,
                  const std::vector<std::string>& languages)
{
  // We just want to print the name of the function and a link, as Markdown.
  // We have to mark it as having the right language with a div.
  for (size_t i = 0; i < languages.size(); ++i)
  {
    BindingInfo::Language() = languages[i];

    cout << " - [" << GetBindingName(bindingName) << "](#" << languages[i]
        << "_" << bindingName << "){: .language-link #" << languages[i] << " }"
        << endl;
  }
}

void PrintDocs(const std::string& bindingName,
               const vector<string>& languages)
{
  ProgramDoc& programDoc = BindingInfo::GetProgramDoc(bindingName);

  CLI::RestoreSettings(bindingName);

  // First, for this section, print each of the names.
  for (size_t i = 0; i < languages.size(); ++i)
  {
    BindingInfo::Language() = languages[i];

    cout << "<div class=\"language-title\" id=\"" << languages[i]
        << "\" markdown=\"1\">" << endl;
    cout << "## " << GetBindingName(bindingName) << endl;
    cout << "{: #" << languages[i] << "_" << bindingName << " }" << endl;
    cout << "</div>" << endl;
  }
  cout << endl;

  // Next we want to print the logical name of the binding (that's known by
  // ProgramInfo).
  cout << "#### " << programDoc.programName << endl;
  cout << endl;

  for (size_t i = 0; i < languages.size(); ++i)
  {
    BindingInfo::Language() = languages[i];

    cout << "<div class=\"language-import\" id=\"" << languages[i]
        << "\" markdown=\"1\">" << endl;
    string import = PrintImport(bindingName);
    if (import.size() > 1)
      cout << "```" << languages[i] << endl << import << endl << "```" << endl;
    cout << "</div>" << endl;
  }
  cout << endl;

  cout << programDoc.shortDocumentation << " ";
  for (size_t i = 0; i < languages.size(); ++i)
  {
    cout << "[Detailed documentation](#" << languages[i] << "_"
      << bindingName << "_detailed-documentation){: .language-detail-link #"
      << languages[i] << " }";
  }
  cout << "." << endl;

  // Next, print the PROGRAM_INFO() documentation for each language.
  for (size_t i = 0; i < languages.size(); ++i)
  {
    BindingInfo::Language() = languages[i];

    // This works with the Kramdown processor.
    cout << "<div class=\"language-section\" id=\"" << languages[i]
        << "\" markdown=\"1\">" << endl;

    // We need to print the signature.

    // Now, iterate through each of the input options.
    cout << endl;
    cout << "### Input options" << endl;
    cout << endl;

    cout << "| ***name*** | ***type*** | ***description*** | ***default*** |"
        << endl;
    cout << "|------------|------------|-------------------|---------------|"
        << endl;
    map<string, ParamData>& parameters = CLI::Parameters();
    for (map<string, ParamData>::const_iterator it = parameters.begin();
         it != parameters.end(); ++it)
    {
      if (!it->second.input)
        continue;

      // There are some special options that don't exist in some languages.
      if (languages[i] != "python" && it->second.name == "copy_all_inputs")
        continue;
      if (languages[i] != "cli" &&
          (it->second.name == "help" || it->second.name == "info" ||
           it->second.name == "version"))
        continue;

      // Print name, type, description, default.
      cout << "| ";
      cout << ParamString(it->second.name) << " | ";
      cout << ParamType(it->second) << " | ";
      cout << it->second.desc; // just a string
      // Print whether or not it's a "special" language-only parameter.
      if (it->second.name == "copy_all_inputs" || it->second.name == "help" ||
          it->second.name == "info" || it->second.name == "version")
      {
        cout << "  <span class=\"special\">Only exists in "
            << PrintLanguage(languages[i]) << " binding.</span>";
      }
      cout << " | ";
      string def = PrintDefault(it->second.name);
      if (def.size() > 0)
        cout << "`" << def << "` |";
      else
        cout << " |";
      cout << endl;
    }
    cout << endl;

    // Next, iterate through the list of output options.
    cout << "### Output options" << endl;
    cout << endl;
    string outputInfo = PrintOutputOptionInfo();
    if (outputInfo.size() > 0)
      cout << outputInfo << endl;
    cout << endl;
    cout << "| ***name*** | ***type*** | ***description*** |" << endl;
    cout << "|------------|------------|-------------------|" << endl;
    for (map<string, ParamData>::const_iterator it = parameters.begin();
         it != parameters.end(); ++it)
    {
      if (it->second.input)
        continue;

      // Print name, type, description.
      cout << "| ";
      cout << ParamString(it->second.name) << " | ";
      cout << ParamType(it->second) << " | ";
      cout << it->second.desc;
      // Print whether or not it's a "special" language-only parameter.
      if (it->second.name == "copy_all_inputs" || it->second.name == "help" ||
          it->second.name == "info" || it->second.name == "version")
      {
        cout << "  <span class=\"special\">Only exists in "
            << PrintLanguage(languages[i]) << " binding.</span>";
      }
      cout << " |";
      cout << endl;
    }
    cout << endl;

    cout << "### Detailed documentation" << endl;
    cout << "{: #" << languages[i] << "_" << bindingName
        << "_detailed-documentation }" << endl;
    cout << endl;
    cout << programDoc.documentation() << endl;
    cout << endl;

    cout << "### See also" << endl;
    cout << endl;
    for (size_t i = 0; i < programDoc.seeAlso.size(); ++i)
    {
      cout << " - " << "[";
      // We need special processing if the user has specified a binding name
      // starting with @ (i.e., '@kfn' or similar).
      if (programDoc.seeAlso[i].first[0] == '@')
        cout << GetBindingName(programDoc.seeAlso[i].first.substr(1));
      else
        cout << programDoc.seeAlso[i].first;
      cout << "](";

      // We need special handling of Doxygen information.
      if (programDoc.seeAlso[i].second.substr(0, 8) == "@doxygen")
        cout << DOXYGEN_PREFIX << programDoc.seeAlso[i].second.substr(9);
      else
        cout << programDoc.seeAlso[i].second;

      cout << ")" << endl;

    }
    cout << endl;

    cout << "</div>" << endl;
    cout << endl;
  }

  CLI::ClearSettings();
}
