(: Name: ancestorself-21 :)
(: Description: Evaluation of the ancestor-or-self axis used together with a newly constructed element. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <anElement>Some Content</anElement>
return
 $var/ancestor-or-self::*