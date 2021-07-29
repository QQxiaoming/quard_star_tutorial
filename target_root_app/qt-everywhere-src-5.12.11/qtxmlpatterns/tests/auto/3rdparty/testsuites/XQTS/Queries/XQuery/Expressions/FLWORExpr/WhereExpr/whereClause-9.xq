(: Name: whereClause-9 :)
(: Description: use of where clause used with is comparator. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <anElement>Some content</anElement>

where $var is $var
return
 $var is $var