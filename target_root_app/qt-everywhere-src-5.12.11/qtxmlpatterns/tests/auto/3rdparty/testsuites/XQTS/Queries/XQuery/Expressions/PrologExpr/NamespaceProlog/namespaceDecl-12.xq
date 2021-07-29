(: Name: namespaceDecl-12 :)
(: Description: verify that the "xdt" prefix can be redefined and used. :)

declare namespace xdt = "http://www.example.com/examples";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <xdt:someElement>some context</xdt:someElement>
return
 $var