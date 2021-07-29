(:*******************************************************:)
(: Test: K2-ComputeConElem-14                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use a technique that sometimes is used for adding namespace nodes. :)
(:*******************************************************:)
declare function local:addNamespace($argElement as element(), $argPrefix as xs:string, $namespaceURI as xs:string) as element()
{
    element { QName($namespaceURI, concat($argPrefix, ":x")) }{$argElement}/*
};

local:addNamespace(<a><b/></a>, "prefix", "http://example.com/")