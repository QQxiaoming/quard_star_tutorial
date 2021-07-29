(: Name: namespaceDecl-10 :)
(: Description: verify that the "xsi" prefix can be redefined and used. :)

declare namespace xsi = "http://www.example.com/examples";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <xsi:someElement>some context</xsi:someElement>
return
 $var