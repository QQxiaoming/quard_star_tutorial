(: Name: NamespaceURIFromQNameFunc010 :)
(: Description: Test function fn:namespace-uri-from-QName. Error case - multiple input values :)

declare default element namespace "http://www.example.com/QNameXSD";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:namespace-uri-from-QName($input-context/root/elemQN)