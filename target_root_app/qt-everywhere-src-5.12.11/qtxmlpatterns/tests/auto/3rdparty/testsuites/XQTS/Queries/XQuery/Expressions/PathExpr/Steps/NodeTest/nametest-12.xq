(: Name: nametest-12 :)
(: Description: Name test that selects an "child::*:day" of an element node used as part of an union operation.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := $input-context1/works/employee[12]/overtime
return
  $var/child::*:day[1] |  $var/child::*:day[2]