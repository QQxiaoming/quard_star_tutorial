(: Name: predicates-18:)
(: Description: Evaluation of a simple predicate, that uses the "ne" operator. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works//day[xs:string(.) ne "Monday"])