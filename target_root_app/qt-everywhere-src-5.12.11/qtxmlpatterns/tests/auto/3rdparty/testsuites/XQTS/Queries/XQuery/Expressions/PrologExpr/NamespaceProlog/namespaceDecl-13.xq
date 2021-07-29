(: Name: namespaceDecl-13 :)
(: Description: verify that the "local" prefix can be redefined and used. :)

declare namespace local = "http://www.example.com/examples";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <local:someElement>some context</local:someElement>
return
 $var