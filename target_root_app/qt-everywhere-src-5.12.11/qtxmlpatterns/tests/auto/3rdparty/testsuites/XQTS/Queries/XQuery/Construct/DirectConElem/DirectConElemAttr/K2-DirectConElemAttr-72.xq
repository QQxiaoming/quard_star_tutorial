(:*******************************************************:)
(: Test: K2-DirectConElemAttr-72                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that attributes constructed with computed constructors doesn't pick up the default namespace. :)
(:*******************************************************:)
namespace-uri-from-QName(node-name(exactly-one(<e xmlns="http://example.com/">{attribute attr {()} }/</e>/@attr))) eq ""