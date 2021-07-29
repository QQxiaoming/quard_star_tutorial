(: Name: nametest-14 :)
(: Description: Name test that selects an "child::*" and "child::day" (same nodes) of an element node used as part of an except operation.:)
(: Use fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := $input-context1/works[1]/child::employee[12]/overtime
return
 fn:count($var/child::* except $var/child::day)
