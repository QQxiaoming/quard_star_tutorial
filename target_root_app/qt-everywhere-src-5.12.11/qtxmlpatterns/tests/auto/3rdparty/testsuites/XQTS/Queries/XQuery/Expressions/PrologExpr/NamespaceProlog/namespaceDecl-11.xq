(: Name: namespaceDecl-11 :)
(: Description: verify that the "fn" prefix can be redefined and used. :)

declare namespace fn = "http://www.example.com/examples";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <fn:someElement>some context</fn:someElement>
return
 $var