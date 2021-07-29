(: Name: namespaceDecl-14 :)
(: Description: verify that upper case "XML" is different from lower case "xml". :)

declare namespace XML = "http://www.example.com/examples";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <XML:someElement>some context</XML:someElement>
return
 $var