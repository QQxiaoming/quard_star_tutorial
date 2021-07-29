(: Name: predicates-10:)
(: Description: Evaluation of a simple predicate, that uses the "xs:decimal()" function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/root/decimal[xs:decimal(.) = 12678967.543233])