(: Name: nametest-13 :)
(: Description: Name test that selects an "child::*" and "child::day" (same nodes) of an element node used as part of an intersect operation.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := $input-context1/works[1]/child::employee[12]/overtime
return
 $var/child::* intersect $var/child::day
