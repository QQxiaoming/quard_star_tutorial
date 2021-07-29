(: Name: NamespaceURIFromQNameFunc004 :)
(: Description: Test function fn:namespace-uri-from-QName. Using attribute derived from QName as input :)

declare default element namespace "http://www.example.com/QNameXSD";  

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:namespace-uri-from-QName(fn:QName("http://www.example.com/urn", exactly-one(xs:string(($input-context/root/@attrDerivQN)[1]))))
