(:*******************************************************:)
(: Test: K2-DefaultNamespaceProlog-6                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure the default element namespace is properly handled with default namespace attribute declarations. :)
(:*******************************************************:)
declare default element namespace "http://example.com/";
namespace-uri-from-QName(xs:QName("localName")),
<e xmlns="">
    {
    " | ",
    namespace-uri-from-QName(xs:QName("localName"))
    }
</e>,
namespace-uri-from-QName(xs:QName("localName"))