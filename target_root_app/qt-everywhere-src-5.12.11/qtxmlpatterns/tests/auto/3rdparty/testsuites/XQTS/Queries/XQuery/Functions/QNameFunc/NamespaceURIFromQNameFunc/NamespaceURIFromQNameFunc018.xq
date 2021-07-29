(: Name: NamespaceURIFromQNameFunc018 :)
(: Description: Test function fn:namespace-uri-from-QName. Explicit call to data to extract simple typed QName value :)

declare default element namespace "http://www.example.com/QNameXSD"; 

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:namespace-uri-from-QName(fn:QName("http://www.example.com/urn", exactly-one(xs:string(data(($input-context/root/elemQN))[1])))) 
