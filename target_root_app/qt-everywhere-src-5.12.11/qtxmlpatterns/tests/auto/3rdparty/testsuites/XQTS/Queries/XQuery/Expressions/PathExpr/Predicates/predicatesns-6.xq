(: Name: predicatesns-6:)
(: Description: Evaluation of a simple predicate set to a boolean expression ("and" operator). Returns "false" :)
(: Uses "fn:count" to avoid empty file.  Not Schema dependent. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(($input-context1//integer[fn:false() and fn:false()]))

