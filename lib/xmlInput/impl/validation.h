/* -------------------------------------------------------------------------- *
 * CEINMS is a standalone toolbox for neuromusculoskeletal modelling and      *
 * simulation. CEINMS can also be used as a plugin for OpenSim either         *
 * through the OpenSim GUI or API. See https://simtk.org/home/ceinms and the  *
 * NOTICE file for more information. CEINMS development was coordinated       *
 * through Griffith University and supported by the Australian National       *
 * Health and Medical Research Council (NHMRC), the US National Institutes of *
 * Health (NIH), and the European Union Framework Programme 7 (EU FP7). Also  *
 * see the PROJECTS file for more information about the funding projects.     *
 *                                                                            *
 * Copyright (c) 2010-2015 Griffith University and the Contributors           *
 *                                                                            *
 * CEINMS Contributors: C. Pizzolato, M. Reggiani, M. Sartori,                *
 *                      E. Ceseracciu, and D.G. Lloyd                         *
 *                                                                            *
 * Author(s):                                                                 *
 *                                                                            *
 * CEINMS is licensed under the Apache License, Version 2.0 (the "License").  *
 * You may not use this file except in compliance with the License. You may   *
 * obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0.*
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */
// file      : examples/cxx/tree/embedded/driver.cxx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// copyright : not copyrighted - public domain

#include <memory>   // std::auto_ptr
#include <string>
#include <fstream>
#include <iostream>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLUniDefs.hpp> // chLatin_*
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/validators/common/Grammar.hpp> // xercesc::Grammar
#include <xercesc/framework/Wrapper4InputSource.hpp>

#if _XERCES_VERSION >= 30000
#  include <xercesc/framework/XMLGrammarPoolImpl.hpp>
#else
#  include <xercesc/internal/XMLGrammarPoolImpl.hpp>
#endif

#include <xsd/cxx/xml/string.hxx>
#include <xsd/cxx/xml/dom/auto-ptr.hxx>
#include <xsd/cxx/xml/dom/bits/error-handler-proxy.hxx>
#include <xsd/cxx/xml/sax/std-input-source.hxx>

#include <xsd/cxx/tree/error-handler.hxx>

//#include "library.hxx"
//#include "library-schema.hxx"
#include "grammar-input-stream.hxx"

//using namespace std;

template <typename XSDfile> std::auto_ptr<XSDfile> parseAndValidate(const std::string& xmlFileName, const XMLByte* schemabin, std::size_t schemasize)
{

      // We need to initialize the Xerces-C++ runtime because we
    // are doing the XML-to-DOM parsing ourselves.
  //
  xercesc::XMLPlatformUtils::Initialize ();
  try
  {
    //using namespace xercesc;
    namespace xml = xsd::cxx::xml;
    namespace tree = xsd::cxx::tree;

    // Create and load the grammar pool.
    //
    xercesc::MemoryManager* mm (xercesc::XMLPlatformUtils::fgMemoryManager);

    std::auto_ptr<xercesc::XMLGrammarPool> gp (new xercesc::XMLGrammarPoolImpl (mm));

    try
    {
      grammar_input_stream is (schemabin, schemasize);
      gp->deserializeGrammars(&is);
    }
    catch(const xercesc::XSerializationException& e)
    {
      std::cerr << "unable to load schema: " <<
        xml::transcode<char> (e.getMessage ()) << endl;
      exit(EXIT_FAILURE);
    }

    // Lock the grammar pool. This is necessary if we plan to use the
    // same grammar pool in multiple threads (this way we can reuse the
    // same grammar in multiple parsers). Locking the pool disallows any
    // modifications to the pool, such as an attempt by one of the threads
    // to cache additional schemas.
    //
    gp->lockPool ();
    // Get an implementation of the Load-Store (LS) interface.
    //
    const XMLCh ls_id [] = {xercesc::chLatin_L, xercesc::chLatin_S, xercesc::chNull};

    xercesc::DOMImplementation* impl (
      xercesc::DOMImplementationRegistry::getDOMImplementation (ls_id));

#if _XERCES_VERSION >= 30000

    // Xerces-C++ 3.0.0 and later.
    //
    xml::dom::auto_ptr<xercesc::DOMLSParser> parser (
      impl->createLSParser (
        xercesc::DOMImplementationLS::MODE_SYNCHRONOUS, 0, mm, gp.get ()));

    xercesc::DOMConfiguration* conf (parser->getDomConfig ());

    // Discard comment nodes in the document.
    //
    conf->setParameter (xercesc::XMLUni::fgDOMComments, false);

    // Enable datatype normalization.
    //
    conf->setParameter (xercesc::XMLUni::fgDOMDatatypeNormalization, true);

    // Do not create EntityReference nodes in the DOM tree. No
    // EntityReference nodes will be created, only the nodes
    // corresponding to their fully expanded substitution text
    // will be created.
    //
    conf->setParameter (xercesc::XMLUni::fgDOMEntities, false);

    // Perform namespace processing.
    //
    conf->setParameter (xercesc::XMLUni::fgDOMNamespaces, true);

    // Do not include ignorable whitespace in the DOM tree.
    //
    conf->setParameter (xercesc::XMLUni::fgDOMElementContentWhitespace, false);

    // Enable validation.
    //
    conf->setParameter (xercesc::XMLUni::fgDOMValidate, true);
    conf->setParameter (xercesc::XMLUni::fgXercesSchema, true);
    conf->setParameter (xercesc::XMLUni::fgXercesSchemaFullChecking, false);

    // Xerces-C++ 3.1.0 is the first version with working multi import
    // support.
    //
#if _XERCES_VERSION >= 30100
    conf->setParameter (xercesc::XMLUni::fgXercesHandleMultipleImports, true);
#endif

    // Use the loaded grammar during parsing.
    //
    conf->setParameter (xercesc::XMLUni::fgXercesUseCachedGrammarInParse, true);

    // Disable loading schemas via other means (e.g., schemaLocation).
    //
    conf->setParameter (xercesc::XMLUni::fgXercesLoadSchema, false);

    // We will release the DOM document ourselves.
    //
    conf->setParameter (xercesc::XMLUni::fgXercesUserAdoptsDOMDocument, true);

    // Set error handler.
    //
    tree::error_handler<char> eh;
    xml::dom::bits::error_handler_proxy<char> ehp (eh);
    conf->setParameter (xercesc::XMLUni::fgDOMErrorHandler, &ehp);

#else // _XERCES_VERSION >= 30000

    // Same as above but for Xerces-C++ 2 series.
    //
    xml::dom::auto_ptr<DOMBuilder> parser (
      impl->createDOMBuilder(
        xercesc::DOMImplementationLS::MODE_SYNCHRONOUS, 0, mm, gp.get ()));


    parser->setFeature (xercesc::XMLUni::fgDOMComments, false);
    parser->setFeature (xercesc::XMLUni::fgDOMDatatypeNormalization, true);
    parser->setFeature (xercesc::XMLUni::fgDOMEntities, false);
    parser->setFeature (xercesc::XMLUni::fgDOMNamespaces, true);
    parser->setFeature (xercesc::XMLUni::fgDOMWhitespaceInElementContent, false);
    parser->setFeature (xercesc::XMLUni::fgDOMValidation, true);
    parser->setFeature (xercesc::XMLUni::fgXercesSchema, true);
    parser->setFeature (xercesc::XMLUni::fgXercesSchemaFullChecking, false);
    parser->setFeature (xercesc::XMLUni::fgXercesUseCachedGrammarInParse, true);
    parser->setFeature (xercesc::XMLUni::fgXercesUserAdoptsDOMDocument, true);

    tree::error_handler<char> eh;
    xml::dom::bits::error_handler_proxy<char> ehp (eh);
    parser->setErrorHandler (&ehp);

#endif // _XERCES_VERSION >= 30000

    // Parse XML documents.
    //
      std::ifstream ifs;
      ifs.exceptions (std::ifstream::badbit | std::ifstream::failbit);
      ifs.open (xmlFileName.c_str());

      // Wrap the standard input stream.
      //
      xml::sax::std_input_source isrc (ifs, xmlFileName);
      xercesc::Wrapper4InputSource wrap (&isrc, false);

      // Parse XML to DOM.
      //
#if _XERCES_VERSION >= 30000
      xml_schema::dom::auto_ptr<xercesc::DOMDocument> doc (parser->parse (&wrap));
#else
      xml_schema::dom::auto_ptr<xercesc::DOMDocument> doc (parser->parse (wrap));
#endif

      eh.throw_if_failed<xml_schema::parsing> ();

      // Parse DOM to the object model.
      //
      const ::xercesc::DOMElement& e (*(doc->getDocumentElement ()));
      //std::auto_ptr<XSDfile> c(new XSDfile(e));
      std::auto_ptr<XSDfile> c(xsd::cxx::tree::traits< XSDfile, char >::create ( e, 0, 0));

      //xercesc::XMLPlatformUtils::Terminate ();
      return c;
  }
  catch (const xml_schema::exception& e)
  {
    std::cerr << e << std::endl;
    exit(EXIT_FAILURE);
  }
  catch (const std::ios_base::failure&)
  {
    std::cerr << xmlFileName << ": unable to open or read failure" << std::endl;
    exit(EXIT_FAILURE);
  }

}
