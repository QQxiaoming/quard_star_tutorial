(: Name: NamespaceURIFromQNameFunc019 :)
(: Description: Test function fn:namespace-uri-from-QName. Conditional expression as parameter where both branches result in QName value:)

declare default element namespace "http://www.example.com/QNameXSD"; 

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:namespace-uri-from-QName(
	if( true() ) then fn:QName("http://www.example.com/urn", exactly-one(xs:string(($input-context/root/elemQN[1]))))
	else fn:QName("http://www.example.com/urn", exactly-one(xs:string(($input-context/root/@attrQN))[1]))
) 
