(: Name: predicates-20:)
(: Description: Evaluation of a simple predicate, that uses the "le" operator. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works//hours[xs:integer(.) le 12])