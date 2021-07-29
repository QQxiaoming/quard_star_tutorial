(: Name: predicatesns-12:)
(: Description: Evaluation of a simple predicate, that uses the "xs:double()" function. :)
(: Not Schema dependent. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/root/double[xs:double(.) = 1267.43233E12])