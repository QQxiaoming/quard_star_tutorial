(: Name: namespaceDecl-15 :)
(: Description: verify that upper case "XMLNS" is different from lower case "xmlns". :)

declare namespace XMLNS = "http://www.example.com/examples";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <XMLNS:someElement>some context</XMLNS:someElement>
return
 $var