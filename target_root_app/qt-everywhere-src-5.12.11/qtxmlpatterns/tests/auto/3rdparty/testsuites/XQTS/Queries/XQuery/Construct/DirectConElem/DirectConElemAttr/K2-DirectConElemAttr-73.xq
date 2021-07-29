(:*******************************************************:)
(: Test: K2-DirectConElemAttr-73                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that attributes constructed with computed constructors doesn't pick up the default namespace(#2). :)
(:*******************************************************:)
declare default element namespace "http://example.com/";
namespace-uri-from-QName(node-name(attribute e {()})) eq ""