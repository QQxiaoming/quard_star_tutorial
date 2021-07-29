(:*******************************************************:)
(: Test: K2-NamespaceURIFromQNameFunc-3                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check that xmlns declarations overrides.     :)
(:*******************************************************:)
declare default element namespace "http://example.com/";
<e xmlns="">{namespace-uri-from-QName(xs:QName("l"))}</e>