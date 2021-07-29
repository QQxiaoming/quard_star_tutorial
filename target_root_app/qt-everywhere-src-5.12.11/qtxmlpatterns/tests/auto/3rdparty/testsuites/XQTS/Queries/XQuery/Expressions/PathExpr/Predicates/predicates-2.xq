(: Name: predicates-2:)
(: Description: Evaluation of a simple predicate with a "false" value (uses "fn:false").  Use of fn:count to avoid empty file.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(($input-context1//integer[fn:false()]))

