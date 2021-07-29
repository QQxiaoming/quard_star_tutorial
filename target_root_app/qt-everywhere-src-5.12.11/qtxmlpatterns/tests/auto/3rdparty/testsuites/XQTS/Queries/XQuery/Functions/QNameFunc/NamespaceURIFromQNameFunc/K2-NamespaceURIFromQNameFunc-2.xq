(:*******************************************************:)
(: Test: K2-NamespaceURIFromQNameFunc-2                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check that the default element namespace is picked up with computed element constructors. :)
(:*******************************************************:)
<e xmlns="http://example.com/">
    {namespace-uri-from-QName(node-name(element anElement{"text"}))}
</e>