(: Name: fn-namespace-uri-16:)
(: Description: Evaluation of the fn:namespace-uri function with no argument.  Use an element node with no namespace queried from a file.:)
(: Use the fn:count function to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := $input-context1/works/employee[1]
return
 fn:count($var/fn:namespace-uri())