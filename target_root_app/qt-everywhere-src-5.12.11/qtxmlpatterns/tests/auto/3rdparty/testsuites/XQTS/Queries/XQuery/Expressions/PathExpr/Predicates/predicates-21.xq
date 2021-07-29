(: Name: predicates-21:)
(: Description: Evaluation of a simple predicate, that uses the "gt" operator. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works//hours[xs:integer(.) gt 79])