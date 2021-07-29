(: Name: nametest-16 :)
(: Description: Name test that selects all the children of the context node with "child::employee".:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := $input-context1/works
return
 fn:count($var/child::employee)
