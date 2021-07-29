(: Name: NamespaceURIFromQNameFunc005 :)
(: Description: Test function fn:namespace-uri-from-QName. Check return type of function is xs:NCName :)

declare default element namespace "http://www.example.com/QNameXSD"; 

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:namespace-uri-from-QName(fn:QName("http://www.example.com/urn", exactly-one(xs:string($input-context/root[1]/@attrDerivQN)))) instance of xs:anyURI ?
