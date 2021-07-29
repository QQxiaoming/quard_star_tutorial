(: Name: NamespaceURIFromQNameFunc021 :)
(: Description: Test function fn:namespace-uri-from-QName. Conditional expression as parameter where one branch does NOT result in a QName value :)

declare default element namespace "http://www.example.com/QNameXSD"; 

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:namespace-uri-from-QName(
	if( true() ) then ($input-context/root/elemStr)[1]
	else ($input-context/root/elemQN)[1]
) 
