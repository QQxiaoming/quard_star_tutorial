(: Name: predicates-19:)
(: Description: Evaluation of a simple predicate, that uses the "lt" operator. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works//hours[xs:integer(.) lt 13])