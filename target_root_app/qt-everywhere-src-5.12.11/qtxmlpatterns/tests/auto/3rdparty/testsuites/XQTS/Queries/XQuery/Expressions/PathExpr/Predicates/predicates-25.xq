(: Name: predicates-25:)
(: Description: Evaluation of a simple predicate, that uses the "<" operator. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works//hours[xs:integer(.) < 13])