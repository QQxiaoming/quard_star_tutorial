(: Name: predicates-22:)
(: Description: Evaluation of a simple predicate, that uses the "ge" operator. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works//hours[xs:integer(.) ge 80])