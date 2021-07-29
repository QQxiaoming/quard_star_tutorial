(: Name: LocalNameFromQNameFunc018 :)
(: Description: Test function fn:local-name-from-QName. Explicit call to data to extract simple typed QName value :)

declare default element namespace "http://www.example.com/QNameXSD"; 

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:local-name-from-QName(fn:QName("http://www.example.com/QNameXSD", exactly-one(xs:string(data(($input-context/root/elemQN)[1]))))) 
