(: Name: predicates-8:)
(: Description: Evaluation of a simple predicate, that uses the "xs:string()" function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/root/string[xs:string(.) = "A String Function"])

