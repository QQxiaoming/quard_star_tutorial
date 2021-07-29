(: Name: LocalNameFromQNameFunc005 :)
(: Description: Test function fn:local-name-from-QName. Check return type of function is xs:NCName :)

declare default element namespace "http://www.example.com/QNameXSD"; 

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:local-name-from-QName(fn:QName("http://www.example.com/QNameXSD", exactly-one(xs:string($input-context/root[1]/@attrDerivQN)))) instance of xs:NCName ?
