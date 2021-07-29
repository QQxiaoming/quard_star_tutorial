(: Name: predicates-13:)
(: Description: Evaluation of a simple predicate, that uses the "xs:boolean" function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/root/boolean[xs:boolean(.) = fn:true()])