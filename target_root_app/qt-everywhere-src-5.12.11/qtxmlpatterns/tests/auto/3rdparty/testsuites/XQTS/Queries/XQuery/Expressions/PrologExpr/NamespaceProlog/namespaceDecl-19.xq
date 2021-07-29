(: Name: namespaceDecl-19 :)
(: Description: Verify that the same namespace URI can be bound to different prefixes. :)

declare namespace px1 = "http://www.example.com/examples";
declare namespace px2 = "http://www.example.com/examples";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <px1:someElement>some context</px1:someElement>
return
 $var