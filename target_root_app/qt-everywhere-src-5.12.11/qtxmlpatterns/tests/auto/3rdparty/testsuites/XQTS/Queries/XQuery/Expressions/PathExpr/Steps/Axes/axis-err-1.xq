(: Name: axis-err-1:)
(: Description: Evaluation of a step axis, which operates on a non node context item.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <anElement>Some content</anElement>
return
$var/20[child::text()]