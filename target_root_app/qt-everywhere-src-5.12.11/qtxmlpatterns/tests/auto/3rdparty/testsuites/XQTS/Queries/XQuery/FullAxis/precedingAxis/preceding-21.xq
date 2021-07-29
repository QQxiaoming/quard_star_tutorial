(: Name: preceding-21 :)
(: Description: Evaluation of the preceding axis used together with a newly constructed element. :)
(: Uses fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <anElement>Some Content</anElement>
return
 fn:count($var/preceding::*)