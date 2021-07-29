(:*******************************************************:)
(: Test: K2-NamespaceURIFromQNameFunc-4                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check that xmlns declarations overrides(#2). :)
(:*******************************************************:)
declare default element namespace "http://example.com/";
declare namespace p = "http://example.com/2";
<e xmlns="" xmlns:p="http://example.com/3">{namespace-uri-from-QName(xs:QName("n1")), namespace-uri-from-QName(xs:QName("p:n2"))}</e>