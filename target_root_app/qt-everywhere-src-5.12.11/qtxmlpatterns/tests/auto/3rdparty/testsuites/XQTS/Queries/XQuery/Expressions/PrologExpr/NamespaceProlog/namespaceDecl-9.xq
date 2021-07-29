(: Name: namespaceDecl-9 :)
(: Description: verify that the "xs" prefix can be redefined and used. :)

declare namespace xs = "http://www.example.com/examples";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <xs:someElement>some context</xs:someElement>
return
 $var