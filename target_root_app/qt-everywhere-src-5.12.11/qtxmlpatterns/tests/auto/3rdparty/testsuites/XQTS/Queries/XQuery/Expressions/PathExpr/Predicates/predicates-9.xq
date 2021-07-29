(: Name: predicates-9:)
(: Description: Evaluation of a simple predicate, that uses the "xs:integer()" function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/root/integer[xs:integer(.) = 12678967543233])