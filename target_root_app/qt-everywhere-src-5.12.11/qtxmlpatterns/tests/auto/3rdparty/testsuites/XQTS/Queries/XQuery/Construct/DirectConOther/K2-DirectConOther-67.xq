(:*******************************************************:)
(: Test: K2-DirectConOther-67                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure the default namespace is picked up by independent child constructors. :)
(:*******************************************************:)
<e xmlns="http://example.com/3">{namespace-uri-from-QName(node-name(<e/>)), namespace-uri-from-QName(node-name(element e2 {()}))}</e>