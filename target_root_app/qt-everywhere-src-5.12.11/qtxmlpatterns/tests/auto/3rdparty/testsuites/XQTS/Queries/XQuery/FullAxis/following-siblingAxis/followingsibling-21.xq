(: Name: following-21 :)
(: Description: Evaluation of the following axis used together with a newly constructed element. :)
(: Uses fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <anElement>Some Content</anElement>
return
 fn:count($var/following::*)