(: Name: predicatesns-1:)
(: Description: Evaluation of a simple predicate with a "true" value (uses "fn:true").  Use of fn:count to avoid empty file.:)
(: Not Schema dependent. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(($input-context1//integer[fn:true()]))

