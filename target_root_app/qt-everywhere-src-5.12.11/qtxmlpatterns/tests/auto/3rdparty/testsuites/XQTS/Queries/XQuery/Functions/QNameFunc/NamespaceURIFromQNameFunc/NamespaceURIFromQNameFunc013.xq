(: Name: NamespaceURIFromQNameFunc013 :)
(: Description: Test function fn:namespace-uri-from-QName. Error case - typo in function name :)

declare default element namespace "http://www.example.com/QNameXSD"; 

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:namespace-uri-from-Qname(($input-context/root/elemQN)[1])